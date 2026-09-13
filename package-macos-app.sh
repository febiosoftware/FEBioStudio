#!/usr/bin/env bash
#
# package-macos-app.sh — tailored for FEBioStudio
#
# Makes a Homebrew-linked .app bundle distributable outside the Mac App Store:
#
#   1. Runs macdeployqt   -> Qt frameworks AND the dlopen'd Qt plugins
#   2. Walks otool -L      -> every remaining non-system dylib / framework
#   3. Copies them into    -> App.app/Contents/Frameworks
#   4. Rewrites all refs   -> @rpath, adds the LC_RPATH entries
#   5. Signs inside-out    -> Developer ID + hardened runtime
#   6. Verifies            -> fails if any absolute build-machine path survived
#
# Notarization is OFF by default; pass --notarize to enable it.
# Written for the bash 3.2 that ships with macOS (no bash 4 features).
#
# ---------------------------------------------------------------------------
# USAGE
# ---------------------------------------------------------------------------
#
#   ./package-macos-app.sh build/bin/Release/FEBioStudio.app \
#       --search-path build/lib \
#       --search-path /path/to/febio/lib \
#       --identity "Developer ID Application: Your Name (TEAMID)"
#
# Options:
#   --search-path DIR     Where to look for @rpath libs (repeatable).
#                         You need this for libfecore/libfebio*/libnglib/libngcore.
#   --identity NAME       Signing identity. Default: first Developer ID
#                         Application cert in the keychain. Use "-" for ad-hoc.
#   --entitlements FILE   Entitlements plist for the main bundle.
#   --macdeployqt PATH    Override macdeployqt autodetection.
#   --skip-qt             Don't run macdeployqt (if you already did).
#   --force               Run even on an already-deployed bundle. Almost never
#                         what you want: rebuild the .app instead.
#   --exclude REGEX       Skip deps whose path matches (e.g. 'python@3')
#   --add-file SRC[:DEST] Copy a non-library file the app needs at runtime into
#                         the bundle before signing (repeatable). DEST is
#                         relative to Contents/ and defaults to
#                         Resources/<basename>. Use this for config and data
#                         files — dependency walking only finds Mach-O
#                         binaries, so a missing febio.xml is invisible to it.
#                         e.g. --add-file ci/MacOS/febio.xml
#                              --add-file certs/cert.pem:Resources/cert.pem
#
#                         NOTE: DEST may not be under MacOS/ unless the file is
#                         a Mach-O binary. codesign treats everything in
#                         Contents/MacOS as CODE, so a data file there fails
#                         with "code object is not signed at all". Data belongs
#                         in Contents/Resources.
#   --prefix DIR          Homebrew prefix. Default: `brew --prefix`.
#   --arch ARCH           Required architecture. Default: arm64. Every embedded
#                         Mach-O must contain it; the build fails otherwise.
#   --thin                lipo -thin every embedded binary down to --arch.
#                         Useful with official Qt, whose frameworks are
#                         universal; roughly halves their size. No effect on
#                         Homebrew bottles, which are arm64-only already.
#   --notarize            Submit to Apple and staple the ticket.
#   --keychain-profile P  notarytool profile name. Default: AC_NOTARY.
#   --dmg                 Build and sign a .dmg afterwards.
#
# ---------------------------------------------------------------------------
# CMake integration
# ---------------------------------------------------------------------------
#
#   add_custom_command(TARGET FEBioStudio POST_BUILD
#     COMMAND "${CMAKE_SOURCE_DIR}/package-macos-app.sh"
#             "$<TARGET_BUNDLE_DIR:FEBioStudio>"
#             --search-path "${CMAKE_BINARY_DIR}/lib"
#             --identity "${MACOS_SIGN_IDENTITY}"
#     VERBATIM)
#
# ---------------------------------------------------------------------------

set -euo pipefail

APP=""
IDENTITY=""
ENTITLEMENTS=""
MACDEPLOYQT=""
SKIP_QT=0
EXCLUDE=""
ADD_FILES=""    # newline-delimited "src|dest-relative-to-Contents"
ARCH="arm64"
DO_THIN=0
FORCE=0
DO_NOTARIZE=0
DO_DMG=0
KEYCHAIN_PROFILE="AC_NOTARY"
HOMEBREW_PREFIX="${HOMEBREW_PREFIX:-$(brew --prefix 2>/dev/null || echo /opt/homebrew)}"
SEARCH_PATHS=""     # newline-delimited

log()  { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33m[warn]\033[0m %s\n' "$*" >&2; }
die()  { CLEAN_EXIT=1; printf '\033[1;31m[fail]\033[0m %s\n' "$*" >&2; exit 1; }

# --- arguments -------------------------------------------------------------

# $2 may be unset, and `set -u` would abort with a cryptic message.
need_val() {
  [ -n "${2:-}" ] || die "option $1 requires a value"
  echo "$2"
}

while [ $# -gt 0 ]; do
  case "$1" in
    --identity)         IDENTITY="$(need_val "$1" "${2:-}")"; shift 2 ;;
    --entitlements)     ENTITLEMENTS="$(need_val "$1" "${2:-}")"; shift 2 ;;
    --macdeployqt)      MACDEPLOYQT="$(need_val "$1" "${2:-}")"; shift 2 ;;
    --search-path)      _sp="$(need_val "$1" "${2:-}")"
                        SEARCH_PATHS="$SEARCH_PATHS
$(cd "$_sp" 2>/dev/null && pwd || echo "$_sp")"; shift 2 ;;
    --exclude)          EXCLUDE="$(need_val "$1" "${2:-}")"; shift 2 ;;
    --add-file)         _af="$(need_val "$1" "${2:-}")"
                        case "$_af" in
                          *:*) _src="${_af%%:*}"; _dst="${_af#*:}" ;;
                          *)   _src="$_af"; _dst="Resources/$(basename "$_af")" ;;
                        esac
                        ADD_FILES="$ADD_FILES
$_src|$_dst"; shift 2 ;;
    --prefix)           HOMEBREW_PREFIX="$(need_val "$1" "${2:-}")"; shift 2 ;;
    --keychain-profile) KEYCHAIN_PROFILE="$(need_val "$1" "${2:-}")"; shift 2 ;;
    --arch)             ARCH="$(need_val "$1" "${2:-}")"; shift 2 ;;
    --skip-qt)          SKIP_QT=1; shift ;;
    --thin)             DO_THIN=1; shift ;;
    --force)            FORCE=1; shift ;;
    --notarize)         DO_NOTARIZE=1; shift ;;
    --dmg)              DO_DMG=1; shift ;;
    -h|--help)          sed -n '2,60p' "$0"; exit 0 ;;
    -*)                 die "unknown option: $1" ;;
    *)                  APP="$1"; shift ;;
  esac
done

[ -n "$APP" ] || die "no .app given. Try: $0 build/bin/Release/FEBioStudio.app"
[ -d "$APP" ] || die "not a directory: $APP"

APP="$(cd "$APP" && pwd)"
APP_NAME="$(basename "$APP" .app)"
EXE_DIR="$APP/Contents/MacOS"
FRAMEWORKS="$APP/Contents/Frameworks"
PLUGINS="$APP/Contents/PlugIns"

# Catch an incomplete bundle before anything else, so a failed compile doesn't
# masquerade as a packaging error.
if [ ! -f "$APP/Contents/Info.plist" ]; then
  warn "$APP/Contents/Info.plist is missing, so this bundle was never finished."
  die  "incomplete .app — the build failed. Fix the build first."
fi

# plutil fails cleanly on a bad key. PlistBuddy, by contrast, prints chatter
# like "File Doesn't Exist, Will Create:" to STDOUT and still exits 0, which
# silently poisons the variable.
EXECUTABLE="$(plutil -extract CFBundleExecutable raw -o - \
              "$APP/Contents/Info.plist" 2>/dev/null || true)"
case "$EXECUTABLE" in
  ""|*/*) EXECUTABLE="$APP_NAME" ;;
esac
[ -f "$EXE_DIR/$EXECUTABLE" ] || EXECUTABLE="$APP_NAME"

MAIN_BINARY="$EXE_DIR/$EXECUTABLE"
if [ ! -f "$MAIN_BINARY" ]; then
  warn "no executable at $MAIN_BINARY"
  warn "Contents/MacOS holds:"
  ls -la "$EXE_DIR" 2>/dev/null | sed 's/^/    /' >&2 || warn "    (directory missing)"
  die "incomplete .app — the build failed. Fix the build first."
fi

if [ -z "$IDENTITY" ]; then
  IDENTITY="$(security find-identity -v -p codesigning 2>/dev/null \
              | sed -n 's/.*"\(Developer ID Application:.*\)"/\1/p' | head -1)"
  [ -n "$IDENTITY" ] || die "no Developer ID cert found. Pass --identity, or --identity '-' for ad-hoc."
fi

# Check the identity NOW rather than after twenty minutes of bundling.
if [ "$IDENTITY" != "-" ]; then
  if ! security find-identity -v -p codesigning 2>/dev/null | grep -Fq "$IDENTITY"; then
    warn "no such identity in your keychain:"
    warn "    $IDENTITY"
    warn "what you actually have:"
    security find-identity -v -p codesigning 2>/dev/null | sed 's/^/    /' >&2 || true
    if security find-identity -v -p codesigning 2>/dev/null | grep -q 'Apple Development:'; then
      warn ""
      warn "An 'Apple Development' certificate is for running on your own"
      warn "machines only — Gatekeeper rejects it for distribution. You need a"
      warn "'Developer ID Application' certificate: Xcode > Settings > Accounts"
      warn "> your Apple ID > Manage Certificates > + > Developer ID Application."
      warn ""
      warn "To test this pipeline before that exists, re-run with:"
      warn "    --identity \"\$(security find-identity -v -p codesigning \\"
      warn "        | sed -n 's/.*\"\\(Apple Development:.*\\)\"/\\1/p' | head -1)\""
    fi
    die "signing identity unavailable"
  fi
fi

# Scratch state (bash 3.2 has no associative arrays).
WORK="$(mktemp -d)"
# Report unexpected exits. `set -e` inside a `cmd | while read` subshell kills
# the script with no output at all, which is miserable to debug.
cleanup() {
  rc=$?
  rm -rf "$WORK"
  if [ "$rc" -ne 0 ] && [ "${CLEAN_EXIT:-0}" = "0" ]; then
    printf '\033[1;31m[fail]\033[0m aborted unexpectedly (exit %s) — no error was reported,\n' "$rc" >&2
    printf '        which usually means a command failed under `set -e`.\n' >&2
    printf '        Re-run with: bash -x %s ... 2>&1 | tail -40\n' "$0" >&2
  fi
}
trap cleanup EXIT
SEEN="$WORK/seen"       # refs already examined
CHANGES="$WORK/changes" # "origref|newref"
LEAKS="$WORK/leaks"
SEARCHDIRS="$WORK/searchdirs"
: > "$SEEN"; : > "$CHANGES"

# Where to hunt for a dependency when its recorded path doesn't exist.
# One directory per line so paths containing spaces survive.
{
  echo "$FRAMEWORKS"
  printf '%s\n' "$SEARCH_PATHS" | grep -v '^$' || true
  echo "$HOMEBREW_PREFIX/lib"
  echo "$HOMEBREW_PREFIX/opt/qtbase/lib"
  echo "$HOMEBREW_PREFIX/opt/qt/lib"
} > "$SEARCHDIRS"

# --- refuse to run twice ---------------------------------------------------
# macdeployqt is NOT idempotent, and neither is install_name_tool rewriting.
# Once the main binary's Qt references have been changed to @rpath, macdeployqt
# can no longer see any "external" Qt frameworks, so it silently deploys
# nothing while still trying to sign frameworks it expects to exist. The result
# is a half-populated bundle with a broken signature. There is no way to undo
# this in place, because the original absolute install names are gone.
if [ "$FORCE" = "0" ]; then
  ALREADY=""
  if [ -d "$FRAMEWORKS" ] && [ -n "$(ls -A "$FRAMEWORKS" 2>/dev/null || true)" ]; then
    ALREADY="Contents/Frameworks is not empty"
  elif [ -f "$APP/Contents/Resources/qt.conf" ]; then
    ALREADY="Contents/Resources/qt.conf already exists"
  fi
  if [ -n "$ALREADY" ]; then
    warn "This bundle has already been processed ($ALREADY)."
    warn "Re-running would corrupt it. Delete the .app and rebuild:"
    warn "    rm -rf \"$APP\" && cmake --build <builddir> --target $APP_NAME"
    die  "refusing to run on an already-deployed bundle (--force to override)"
  fi
fi

log "app:        $APP"
log "executable: $EXECUTABLE"
log "arch(es):   $(lipo -archs "$MAIN_BINARY" 2>/dev/null || echo unknown)"
log "identity:   $IDENTITY"

mkdir -p "$FRAMEWORKS"

# ---------------------------------------------------------------------------
# 1. Qt first, via macdeployqt
# ---------------------------------------------------------------------------
# This matters more than it looks: the Cocoa platform plugin (libqcocoa.dylib)
# is loaded with dlopen at startup, so it never appears in `otool -L` and no
# dependency walker can discover it. Without it the app dies immediately with
# "no Qt platform plugin could be initialized".

if [ "$SKIP_QT" = "0" ]; then
  if [ -z "$MACDEPLOYQT" ]; then
    for c in "$HOMEBREW_PREFIX/opt/qtbase/bin/macdeployqt" \
             "$HOMEBREW_PREFIX/opt/qt/bin/macdeployqt" \
             "$HOMEBREW_PREFIX/bin/macdeployqt" \
             "$(command -v macdeployqt 2>/dev/null || true)"; do
      if [ -n "$c" ] && [ -x "$c" ]; then MACDEPLOYQT="$c"; break; fi
    done
  fi
  if [ -n "$MACDEPLOYQT" ]; then
    log "running macdeployqt: $MACDEPLOYQT"
    # -no-strip keeps signatures verifiable; we sign everything ourselves later.
    #
    # Output is captured and filtered. macdeployqt emits a wall of "ERROR:
    # Cannot resolve rpath" that looks alarming and is entirely expected here:
    #   * It searches only qtbase's lib dir, so it cannot see Qt modules that
    #     Homebrew ships in separate formulae (qtsvg, qtdeclarative,
    #     qtvirtualkeyboard, qtpdf).
    #   * It knows nothing about the app's own @rpath libraries (libfecore,
    #     libSimpleITK_*, libnglib, ...).
    #   * It looks for Homebrew's Python under lib/, but Homebrew puts the
    #     framework under Frameworks/.
    # Every one of those is resolved by the dependency walker below, and the
    # "auditing for build-machine paths" step at the end is the real check.
    # Anything macdeployqt says that is NOT one of those patterns still shows.
    MDQ_LOG="$WORK/macdeployqt.log"
    "$MACDEPLOYQT" "$APP" -no-strip -verbose=1 >"$MDQ_LOG" 2>&1 \
      || warn "macdeployqt returned non-zero — continuing, but check Qt frameworks by hand"

    _mdq_noise="$(grep -cE 'Cannot resolve rpath|^ERROR:  using QList|no file at .*python@|otool-classic: can.t open file' "$MDQ_LOG" 2>/dev/null || true)"
    grep -vE 'Cannot resolve rpath|^ERROR:  using QList|no file at .*python@|otool-classic: can.t open file' \
         "$MDQ_LOG" 2>/dev/null | sed '/^[[:space:]]*$/d;s/^/    /' || true
    if [ "${_mdq_noise:-0}" -gt 0 ]; then
      log "macdeployqt: suppressed $_mdq_noise expected 'cannot resolve' line(s); the dependency walker handles those"
    fi
  else
    warn "macdeployqt not found. Qt frameworks and plugins will NOT be deployed."
    warn "Install it (brew install qtbase) or pass --macdeployqt PATH."
  fi

  # -------------------------------------------------------------------------
  # 1b. Drop Qt modules the app does not use.
  # -------------------------------------------------------------------------
  # Homebrew splits Qt across formulae (qtbase, qtsvg, qtdeclarative,
  # qtvirtualkeyboard, qtpdf). macdeployqt only searches qtbase's lib dir, so
  # it cannot resolve QtSvg / QtPdf / QtVirtualKeyboard* and prints
  # "Cannot resolve rpath ..." for each. It then skips rewriting their install
  # names. The dependency walker below still copies them in, so they land in
  # the bundle with their load commands still pointing at /opt/homebrew --
  # and the moment one of those plugins is dlopen'd it drags a SECOND copy of
  # QtCore/QtGui into the process. That is the "Class ... is implemented in
  # both" duplicate-Qt crash.
  #
  # FEBio Studio uses none of them (no .svg in febiostudio.qrc, no touch
  # keyboard, no PDF image decoding), so remove them before the walker runs
  # rather than shipping a landmine.
  for _qtmod in QtPdf QtSvg QtVirtualKeyboard QtVirtualKeyboardQml; do
    if [ -d "$FRAMEWORKS/$_qtmod.framework" ]; then
      log "removing unused $_qtmod.framework"
      rm -rf "$FRAMEWORKS/$_qtmod.framework"
    fi
  done
  for _qtplug in "PlugIns/platforminputcontexts" \
                 "PlugIns/imageformats/libqpdf.dylib" \
                 "PlugIns/iconengines/libqsvgicon.dylib"; do
    if [ -e "$APP/Contents/$_qtplug" ]; then
      log "removing unused plugin $_qtplug"
      rm -rf "$APP/Contents/$_qtplug"
    fi
  done
fi

# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

is_system_lib() {
  case "$1" in
    /usr/lib/*|/System/*|/Library/Apple/*) return 0 ;;
    *) return 1 ;;
  esac
}

is_macho() { file "$1" 2>/dev/null | grep -q 'Mach-O'; }

# "/opt/homebrew/.../QtCore.framework/Versions/A/QtCore" -> ".../QtCore.framework"
framework_root_of() {
  local p="$1"
  case "$p" in
    *.framework/*)
      # Strip everything after the first .framework component.
      echo "${p%%.framework/*}.framework"
      ;;
    *) echo "" ;;
  esac
}

own_install_name() { otool -D "$1" 2>/dev/null | tail -n +2 | head -1; }

direct_deps() {
  local bin="$1" self
  self="$(own_install_name "$bin")"
  otool -L "$bin" 2>/dev/null | tail -n +2 | awk '{print $1}' | while read -r ref; do
    [ -n "$ref" ] || continue
    [ "$ref" = "$self" ] || echo "$ref"
  done
}

# Hunt for a dependency whose recorded path does not exist on disk.
# Tries the full trailing path, the framework-relative path, then the bare
# basename, across every search directory.
search_for() {
  local tail="$1" base fwtail sp
  base="$(basename "$tail")"
  fwtail=""
  case "$tail" in
    # "../Frameworks/QtSvg.framework/Versions/A/QtSvg"
    #   -> "QtSvg.framework/Versions/A/QtSvg"
    *.framework/*)
      fwtail="$(basename "${tail%%.framework/*}").framework/${tail#*.framework/}" ;;
  esac
  while IFS= read -r sp; do
    [ -n "$sp" ] || continue
    if [ -n "$fwtail" ] && [ -e "$sp/$fwtail" ]; then echo "$sp/$fwtail"; return; fi
    if [ -e "$sp/$tail" ]; then echo "$sp/$tail"; return; fi
    if [ -e "$sp/$base" ]; then echo "$sp/$base"; return; fi
  done < "$SEARCHDIRS"
  echo ""
}

# Turn a reference into a real path on disk.
#
# macdeployqt rewrites a plugin's dependencies to @executable_path/../Frameworks
# even for frameworks it has not copied, so taking these references literally
# is not enough — an unresolved one has to fall back to a search.
resolve_ref() {
  local ref="$1" referrer="$2" dir tail lit
  dir="$(cd -P "$(dirname "$referrer")" 2>/dev/null && pwd -P || echo /nonexistent)"
  case "$ref" in
    @loader_path/*)     tail="${ref#@loader_path/}";     lit="$dir/$tail" ;;
    @executable_path/*) tail="${ref#@executable_path/}"; lit="$EXE_DIR/$tail" ;;
    @rpath/*)           tail="${ref#@rpath/}";           lit="$FRAMEWORKS/$tail" ;;
    *)                  echo "$ref"; return ;;
  esac
  [ -e "$lit" ]      && { echo "$lit"; return; }
  [ -e "$dir/$tail" ] && { echo "$dir/$tail"; return; }
  search_for "$tail"
}

# ---------------------------------------------------------------------------
# 2. Discover the dependency closure
# ---------------------------------------------------------------------------

record_change() { echo "$1|$2" >> "$CHANGES"; }

# Copy a dylib or a whole framework into Contents/Frameworks.
# Echoes the new @rpath reference.
embed() {
  local ref="$1" real="$2" fwroot fwname rel base dest
  fwroot="$(framework_root_of "$real")"

  if [ -n "$fwroot" ] && [ -d "$fwroot" ]; then
    # Resolve to the PHYSICAL path before copying. Homebrew exposes frameworks
    # under $PREFIX/lib as RELATIVE symlinks into ../Cellar/<formula>/..., and
    # `cp -R` reproduces a symlink argument AS a symlink. The copy's relative
    # target then resolves against Contents/Frameworks and dangles — which the
    # prune step below would then dutifully delete.
    fwroot="$(cd -P "$fwroot" 2>/dev/null && pwd -P || true)"
    if [ -z "$fwroot" ] || [ ! -d "$fwroot" ]; then echo ""; return; fi
    fwname="$(basename "$fwroot")"                  # QtCore.framework
    rel="${real#"$fwroot"/}"                        # Versions/A/QtCore
    dest="$FRAMEWORKS/$fwname"
    if [ ! -d "$dest" ]; then
      # -R preserves the Versions/Current symlink structure that codesign needs.
      cp -R "$fwroot" "$dest"
      chmod -R u+w "$dest"
      # Trim what must not ship: headers, build metadata, debug symbols.
      rm -rf "$dest/Headers" "$dest"/Versions/*/Headers \
             "$dest"/*.prl "$dest"/Versions/*/*.prl "$dest"/*.dSYM 2>/dev/null || true
      echo "    embedded framework $fwname" >&2
    fi
    install_name_tool -id "@rpath/$fwname/$rel" "$dest/$rel" 2>/dev/null || true
    echo "@rpath/$fwname/$rel"
  else
    base="$(basename "$ref")"
    dest="$FRAMEWORKS/$base"
    if [ ! -f "$dest" ]; then
      cp -Lf "$real" "$dest"      # -L resolves Homebrew's version symlinks
      chmod u+w "$dest"
      echo "    embedded $base" >&2
    fi
    install_name_tool -id "@rpath/$base" "$dest" 2>/dev/null || true
    echo "@rpath/$base"
  fi
}

collect() {
  local bin="$1" tmp ref real newref
  tmp="$(mktemp "$WORK/deps.XXXXXX")"
  direct_deps "$bin" > "$tmp"

  # Read from a file, not a pipe: `while read` on the right of a pipe runs in a
  # subshell and would discard the state we append below.
  while IFS= read -r ref; do
    [ -n "$ref" ] || continue
    is_system_lib "$ref" && continue
    if [ -n "$EXCLUDE" ] && echo "$ref" | grep -qE "$EXCLUDE"; then
      warn "excluded by --exclude: $ref"
      continue
    fi
    grep -Fxq "$ref" "$SEEN" && continue
    echo "$ref" >> "$SEEN"

    real="$(resolve_ref "$ref" "$bin")"
    if [ -z "$real" ] || [ ! -e "$real" ]; then
      warn "unresolved: '$ref' (from $(basename "$bin")) — add a --search-path"
      continue
    fi
    # -P/pwd -P is essential: bash's `cd` is LOGICAL by default and would hand
    # back /opt/homebrew/lib/QtSvg.framework/... instead of the Cellar path it
    # actually points at, leaving embed() to copy a symlink.
    real="$(cd -P "$(dirname "$real")" && pwd -P)/$(basename "$real")"
    is_system_lib "$real" && continue

    # Already inside the bundle (e.g. macdeployqt put it there)? Just recurse.
    case "$real" in
      "$APP"/*) collect "$real"; continue ;;
    esac

    newref="$(embed "$ref" "$real")"
    record_change "$ref" "$newref"
    collect "$real"
  done < "$tmp"
  rm -f "$tmp"
}

# ---------------------------------------------------------------------------
# 1b. Extra files the app needs at runtime
# ---------------------------------------------------------------------------
# Staged BEFORE the dependency walk on purpose. An added Mach-O binary — a
# helper tool like febio4 — has its own dylib dependencies, and it must be in
# Contents/MacOS by the time collect() sweeps that directory or its libraries
# are never embedded and never relinked. (Staging must also precede signing,
# or added files fall outside the bundle seal.)

if [ -n "$(printf '%s' "$ADD_FILES" | tr -d '[:space:]')" ]; then
  log "adding extra files"
  while IFS='|' read -r _s _d; do
    [ -n "${_s:-}" ] || continue
    [ -f "$_s" ] || die "--add-file: no such file: $_s"

    # codesign seals Contents/MacOS as CODE, not as resources. A non-Mach-O
    # file there becomes an unsigned nested code object and signing fails with
    # "code object is not signed at all". Catch it here rather than 80 dylibs
    # later.
    case "$_d" in
      MacOS/*)
        if ! is_macho "$_s"; then
          warn "--add-file: '$_s' is not a Mach-O binary, so it cannot live in"
          warn "Contents/MacOS — codesign treats that directory as code and will"
          warn "fail with 'code object is not signed at all'."
          warn "Put it in Resources instead:"
          warn "    --add-file $_s:Resources/$(basename "$_s")"
          die "invalid --add-file destination: Contents/$_d"
        fi ;;
    esac

    _t="$APP/Contents/$_d"
    mkdir -p "$(dirname "$_t")"
    cp -f "$_s" "$_t"
    if is_macho "$_t"; then
      chmod +x "$_t"
      echo "    added Contents/$_d (Mach-O — will join the dependency walk)"
    else
      echo "    added Contents/$_d"
    fi
  done <<EOF
$ADD_FILES
EOF
fi

log "walking dependency graph"
collect "$MAIN_BINARY"

# Helper executables, Qt plugins, and anything macdeployqt dropped in.
for extra_dir in "$EXE_DIR" "$PLUGINS" "$FRAMEWORKS"; do
  [ -d "$extra_dir" ] || continue
  while IFS= read -r b; do
    [ -n "$b" ] || continue
    collect "$b"
  done <<EOF
$(find "$extra_dir" -type f \( -perm -u+x -o -name '*.dylib' \) 2>/dev/null)
EOF
done

log "embedded $(wc -l < "$CHANGES" | tr -d ' ') reference(s)"

# ---------------------------------------------------------------------------
# 2b. Prune what must not be sealed into the signature
# ---------------------------------------------------------------------------
# codesign follows symlinks when it seals bundle resources. A symlink whose
# target is missing makes `codesign --verify` fail with a bare
# "No such file or directory" naming the .app rather than the broken link,
# which is close to undebuggable. Homebrew frameworks — Python's especially —
# are full of symlink farms, so clear the dead ones out first.

log "pruning cruft and dangling symlinks"
find "$APP/Contents" -name '__pycache__' -type d -prune -exec rm -rf {} + 2>/dev/null || true
find "$APP/Contents" \( -name '*.pyc' -o -name '*.pyo' -o -name '*.prl' \
                        -o -name '*.la' -o -name '*.a' \) -delete 2>/dev/null || true

# CPython's own test suite must not ship. Beyond being dead weight, it contains
# deliberately malformed archives (test/zipimport_data/sparse-zip64-*.part)
# that the notary service tries to unpack and cannot, which it reports as a
# critical validation error. idlelib is the Tk IDE — equally unwanted here.
rm -rf "$FRAMEWORKS"/Python.framework/Versions/*/lib/python*/test 2>/dev/null || true
rm -rf "$FRAMEWORKS"/Python.framework/Versions/*/lib/python*/idlelib 2>/dev/null || true
rm -rf "$FRAMEWORKS"/Python.framework/Versions/*/share 2>/dev/null || true
FW_LOST="$WORK/fwlost"; : > "$FW_LOST"
while IFS= read -r l; do
  [ -n "$l" ] || continue
  # -e on a symlink tests the TARGET, so this is the broken-link test.
  if [ ! -e "$l" ]; then
    case "$l" in
      # A whole framework arriving as a dangling symlink means the copy went
      # wrong upstream. Deleting it would produce a bundle that signs and
      # verifies cleanly and then fails to load that framework at runtime,
      # so refuse rather than "fix" it.
      *.framework)
        echo "${l#"$APP"/}" >> "$FW_LOST"
        continue ;;
    esac
    rm -f "$l"
    echo "    removed dangling symlink: ${l#"$APP"/}"
  fi
done <<EOF
$(find "$APP/Contents" -type l 2>/dev/null || true)
EOF

if [ -s "$FW_LOST" ]; then
  warn "these frameworks were embedded as dangling symlinks, not directories:"
  sed 's/^/    /' "$FW_LOST" >&2
  die "framework copy produced symlinks instead of real trees — this is a bug"
fi

# ---------------------------------------------------------------------------
# 3. Rewrite every reference to point inside the bundle
# ---------------------------------------------------------------------------

all_machos() {
  find "$EXE_DIR" -type f -perm -u+x 2>/dev/null || true
  find "$FRAMEWORKS" -type f \( -name '*.dylib' -o -perm -u+x \) 2>/dev/null || true
  [ -d "$PLUGINS" ] && find "$PLUGINS" -type f -name '*.dylib' 2>/dev/null || true
}

# Optional: drop every slice except --arch. Must happen before signing.
if [ "$DO_THIN" = "1" ]; then
  log "thinning embedded binaries to $ARCH"
  all_machos | while read -r bin; do
    [ -f "$bin" ] || continue
    is_macho "$bin" || continue
    # lipo errors on an already-thin file, so only touch genuinely fat ones.
    if [ "$(lipo -archs "$bin" 2>/dev/null | wc -w | tr -d ' ')" -gt 1 ]; then
      chmod u+w "$bin"
      if lipo "$bin" -thin "$ARCH" -output "$bin.thin" 2>/dev/null; then
        mv -f "$bin.thin" "$bin"
        echo "    thinned $(basename "$bin")"
      else
        rm -f "$bin.thin"
        warn "could not thin $(basename "$bin") to $ARCH"
      fi
    fi
  done
fi

# Normalize the install name (LC_ID_DYLIB) of EVERY binary in the bundle.
# embed() only does this for files it copied itself, but macdeployqt copies
# most of the dylibs here and leaves their IDs as absolute Homebrew paths.
# collect() sees those already inside the bundle and skips embed() entirely,
# so without this pass their IDs are never touched.
log "normalizing install names"
while IFS= read -r lib; do
  [ -n "$lib" ] || continue
  [ -f "$lib" ] || continue
  is_macho "$lib" || continue
  chmod u+w "$lib" 2>/dev/null || true
  case "$lib" in
    # Frameworks keep their Foo.framework/Versions/X/Foo shape.
    *.framework/*)
      fwn="$(basename "${lib%%.framework/*}").framework"
      install_name_tool -id "@rpath/$fwn/${lib#*.framework/}" "$lib" 2>/dev/null || true ;;
    *)
      install_name_tool -id "@rpath/$(basename "$lib")" "$lib" 2>/dev/null || true ;;
  esac
done <<EOF
$(find "$FRAMEWORKS" -type f \( -name '*.dylib' -o -perm -u+x \) 2>/dev/null || true)
EOF

# ---------------------------------------------------------------------------
log "rewriting install names"
# install_name_tool accepts any number of -change pairs per invocation, so the
# whole change table is applied to each binary in ONE exec. The obvious nesting
# (a -change per pair per binary) is quadratic: ~350 binaries x ~200 recorded
# changes is ~70,000 process spawns, each one rewriting a Mach-O header, which
# took many minutes on a bundle this size. Building the argument list once
# reduces that to ~350 execs.
CHANGE_ARGS=()
while IFS='|' read -r origref newref; do
  [ -n "${origref:-}" ] || continue
  CHANGE_ARGS+=( -change "$origref" "$newref" )
done < "$CHANGES"

if [ "${#CHANGE_ARGS[@]}" -eq 0 ]; then
  log "no references to rewrite"
else
  all_machos | while read -r bin; do
    [ -f "$bin" ] || continue
    file "$bin" 2>/dev/null | grep -q 'Mach-O' || continue
    chmod u+w "$bin"
    install_name_tool "${CHANGE_ARGS[@]}" "$bin" 2>/dev/null || true
  done
fi

# ---------------------------------------------------------------------------
# 4. LC_RPATH entries
# ---------------------------------------------------------------------------

add_rpath_once() {
  local bin="$1" path="$2"
  otool -l "$bin" 2>/dev/null | grep -A2 LC_RPATH | grep -Fq " $path" && return 0
  install_name_tool -add_rpath "$path" "$bin" 2>/dev/null || true
}

find "$EXE_DIR" -type f -perm -u+x 2>/dev/null | while read -r b; do
  add_rpath_once "$b" "@executable_path/../Frameworks"
done
# So embedded libs find their siblings even when dlopen'd.
find "$FRAMEWORKS" -type f -name '*.dylib' 2>/dev/null | while read -r lib; do
  add_rpath_once "$lib" "@loader_path"
done
if [ -d "$PLUGINS" ]; then
  find "$PLUGINS" -type f -name '*.dylib' 2>/dev/null | while read -r p; do
    add_rpath_once "$p" "@loader_path/../../Frameworks"
  done
fi

# Tell Qt where its plugins live, so it never probes the build machine's paths.
if [ -d "$PLUGINS" ] && [ ! -f "$APP/Contents/Resources/qt.conf" ]; then
  mkdir -p "$APP/Contents/Resources"
  printf '[Paths]\nPlugins = PlugIns\n' > "$APP/Contents/Resources/qt.conf"
  log "wrote Contents/Resources/qt.conf"
fi

# ---------------------------------------------------------------------------
# 4b. Hardening pass — make the bundle genuinely self-contained
# ---------------------------------------------------------------------------
# Sections 2-4 are driven by $CHANGES, which only holds references collect()
# actually walked. Two blind spots let absolute paths survive:
#
#   * all_machos() selects with `-name '*.dylib' -o -perm -u+x`. A framework's
#     binary is named Foo.framework/Versions/A/Foo and ships mode 0644 --
#     neither test matches, so NO framework binary is ever rewritten here.
#     Frameworks macdeployqt handled are fine; ones it skipped keep absolute
#     /opt/homebrew references and pull a second copy of Qt at runtime.
#   * Python's extension modules (lib-dynload/*.so) and helper executables are
#     reached via dlopen, so they never enter the dependency graph at all.
#
# This pass is deliberately NOT change-list driven. It re-reads every Mach-O in
# the bundle -- detected by CONTENT, not by name or mode -- and repoints any
# remaining non-system absolute dependency at @rpath, embedding the library
# first if it is not already present. Idempotent: safe to run repeatedly.

# Enumerate by content, so framework binaries, .so modules and unsuffixed
# helper executables are all caught.
# NOTE the trailing `|| true`: this script runs under `set -euo pipefail`, and
# xargs exits 123 if ANY `file` invocation fails (a stray unreadable file is
# enough). With pipefail that failure becomes the pipeline's status, the
# command substitution that calls this fails, and set -e kills the run with no
# diagnostic. Same reason every pipeline in this section ends in `|| true`.
bundle_machos() {
  find "$APP/Contents" -type f ! -name '*.py' ! -name '*.pyc' ! -name '*.h' \
       ! -name '*.txt' ! -name '*.plist' -print0 2>/dev/null \
    | xargs -0 -n 100 file --mime-type 2>/dev/null \
    | grep -E ':[[:space:]]+application/x-mach-binary$' \
    | sed -E 's/:[[:space:]]+application\/x-mach-binary$//' \
    || true
}

# Levels from this binary up to Contents/Frameworks, as an @loader_path rpath.
rpath_to_frameworks() {
  local rel dir up c
  rel="${1#"$APP"/Contents/}"
  dir="$(dirname "$rel")"
  up=""
  if [ "$dir" != "." ]; then
    local IFS=/
    for c in $dir; do up="../$up"; done
  fi
  printf '@loader_path/%sFrameworks' "$up"
}

HARDENED=0
HARD_FAIL=0

harden_pass() {
  local bin ref self need dest newref fwroot fwname rel base real
  while IFS= read -r bin; do
    [ -n "$bin" ] || continue
    [ -f "$bin" ] || continue
    chmod u+w "$bin" 2>/dev/null || true
    # own_install_name pipes through `head -1`, which can SIGPIPE otool under
    # pipefail. Tolerate it.
    self="$(own_install_name "$bin" || true)"
    need=0

    while IFS= read -r ref; do
      [ -n "$ref" ] || continue
      [ "$ref" = "$self" ] && continue
      case "$ref" in /*) ;; *) continue ;; esac   # absolute references only
      is_system_lib "$ref" && continue
      if [ -n "$EXCLUDE" ] && echo "$ref" | grep -qE "$EXCLUDE"; then continue; fi

      fwroot="$(framework_root_of "$ref")"
      if [ -n "$fwroot" ]; then
        fwname="$(basename "$fwroot")"
        rel="${ref#"$fwroot"/}"
        dest="$FRAMEWORKS/$fwname/$rel"
        newref="@rpath/$fwname/$rel"
      else
        base="$(basename "$ref")"
        dest="$FRAMEWORKS/$base"
        newref="@rpath/$base"
      fi

      if [ ! -e "$dest" ]; then
        real="$ref"
        [ -e "$real" ] || real="$(search_for "$(basename "$ref")")"
        if [ -n "$real" ] && [ -e "$real" ]; then
          embed "$ref" "$real" >/dev/null || true
        else
          warn "hardening: cannot embed '$ref'"
          warn "  needed by ${bin#"$APP"/} — bundle will NOT run without it"
          HARD_FAIL=$((HARD_FAIL+1))
          continue
        fi
      fi

      install_name_tool -change "$ref" "$newref" "$bin" 2>/dev/null || true
      echo "    ${bin#"$APP"/Contents/}: $(basename "$ref") -> @rpath"
      need=1
      HARDENED=$((HARDENED+1))
    done <<EOF
$(otool -L "$bin" 2>/dev/null | tail -n +2 | awk '{print $1}' || true)
EOF

    # Make sure @rpath actually resolves from wherever this binary sits.
    # This MUST be an if-block, not `[ ... ] && cmd`. That form returns 1 when
    # the test is false, and as the last command of the loop body it becomes
    # the loop's status, then the function's status — and a bare `harden_pass`
    # call failing under set -e aborts the whole script.
    if [ "$need" = "1" ]; then
      add_rpath_once "$bin" "$(rpath_to_frameworks "$bin")"
    fi
  done <<EOF
$(bundle_machos)
EOF
  return 0
}

# Two passes: the first may embed frameworks whose own binaries then need the
# same treatment. The second is a no-op on an already-clean bundle.
log "hardening: making the bundle self-contained"
harden_pass
harden_pass
log "hardening: rewrote $HARDENED reference(s)"
if [ "$HARD_FAIL" -gt 0 ]; then
  warn "$HARD_FAIL dependency/ies could not be embedded — this bundle is NOT portable"
fi

# ---------------------------------------------------------------------------
# 5. Sign, inside-out
# ---------------------------------------------------------------------------

log "signing"
SIGN_ARGS="--force --timestamp --options runtime"
if [ "$IDENTITY" = "-" ]; then
  SIGN_ARGS="--force"     # ad-hoc signatures cannot carry a secure timestamp
  warn "ad-hoc signing: Gatekeeper will reject a downloaded copy of this app"
fi
sign() { codesign $SIGN_ARGS --sign "$IDENTITY" "$@"; }   # shellcheck disable=SC2086

# Notarization requires EVERY Mach-O in the bundle to carry its own Developer
# ID signature, a secure timestamp and the hardened runtime. Signing a
# framework as a bundle only seals its contents as RESOURCES, which is not the
# same thing — the notary service rejected 157 binaries on the first attempt,
# all of them inside Python.framework (lib-dynload/*.so, bin/python3.*).
# Section 4b makes this worse by design: rewriting load commands with
# install_name_tool invalidates whatever signature a file arrived with.
#
# Deepest path first, so nested code is always signed before the bundle that
# contains it. The app's own main binary is skipped — it gets signed as part
# of the .app at the end.
log "signing nested Mach-O binaries"
_nsigned=0
while IFS= read -r m; do
  [ -n "$m" ] || continue
  [ -f "$m" ] || continue
  if [ "$m" = "$MAIN_BINARY" ]; then continue; fi
  if sign "$m" >/dev/null 2>&1; then
    _nsigned=$((_nsigned+1))
  else
    warn "could not sign ${m#"$APP"/}"
  fi
done <<EOF
$(bundle_machos | awk '{ n = gsub(/\//,"/"); print n "\t" $0 }' \
   | sort -rn -k1,1 | cut -f2- || true)
EOF
log "signed $_nsigned nested binary/ies"

# Plain dylibs and plugins first.
find "$FRAMEWORKS" -maxdepth 1 -type f -name '*.dylib' 2>/dev/null | while read -r f; do sign "$f"; done
[ -d "$PLUGINS" ] && find "$PLUGINS" -type f -name '*.dylib' 2>/dev/null | while read -r f; do sign "$f"; done

# Nested app bundles must be signed before the framework that contains them.
# Python.framework ships one at Versions/*/Resources/Python.app; an unsigned
# nested bundle invalidates the enclosing framework's seal.
find "$FRAMEWORKS" -type d -name '*.app' 2>/dev/null | while read -r nested; do
  echo "    nested bundle: ${nested#"$FRAMEWORKS"/}"
  sign "$nested"
done

# Frameworks are signed as bundles, not as bare binaries.
find "$FRAMEWORKS" -maxdepth 1 -type d -name '*.framework' 2>/dev/null | while read -r fw; do sign "$fw"; done

# Helper executables, then the app itself last.
find "$EXE_DIR" -type f -perm -u+x ! -name "$EXECUTABLE" 2>/dev/null | while read -r h; do sign "$h"; done

if [ -n "$ENTITLEMENTS" ]; then
  [ -f "$ENTITLEMENTS" ] || die "entitlements file not found: $ENTITLEMENTS"
  SIGN_RC=0; sign --entitlements "$ENTITLEMENTS" "$APP" || SIGN_RC=$?
else
  SIGN_RC=0; sign "$APP" || SIGN_RC=$?
fi
if [ "$SIGN_RC" -ne 0 ]; then
  warn "signing the bundle failed (see codesign output above)."
  warn "'code object is not signed at all' names a NON-CODE file living"
  warn "somewhere codesign expects code. Anything in Contents/MacOS that is"
  warn "not a Mach-O binary triggers it. Check for stray data files:"
  find "$EXE_DIR" -type f 2>/dev/null | while IFS= read -r f; do
    is_macho "$f" || printf '        not code: %s\n' "${f#"$APP"/}" >&2
  done
  die "codesign failed"
fi

# ---------------------------------------------------------------------------
# 6. Verify
# ---------------------------------------------------------------------------

log "verifying signature"
if ! codesign --verify --strict "$APP" 2>"$WORK/verifyerr"; then
  cat "$WORK/verifyerr" >&2
  warn "--- diagnostics ---"
  warn "dangling symlinks (codesign reports these as 'No such file or directory'):"
  find "$APP" -type l ! -exec test -e {} \; -print 2>/dev/null \
    | sed 's/^/    /' >&2 || warn "    (none)"
  warn "nested bundles found:"
  find "$APP/Contents/Frameworks" -type d \( -name '*.app' -o -name '*.bundle' \) 2>/dev/null \
    | sed 's/^/    /' >&2 || warn "    (none)"
  warn "deep verification tail:"
  codesign --verify --deep --strict --verbose=4 "$APP" 2>&1 | tail -15 | sed 's/^/    /' >&2 || true
  die "signature verification failed"
fi

log "auditing for build-machine paths"
: > "$LEAKS"; : > "$WORK/staleids"
BUILD_TREE="$(dirname "$APP")"
# A here-doc, not a pipe: `cmd | while read` runs the loop in a SUBSHELL, so a
# `grep` that legitimately matches nothing takes the whole script down under
# `set -e` with no diagnostic. Every command below is explicitly `|| true`.
while IFS= read -r bin; do
  [ -n "$bin" ] || continue
  [ -f "$bin" ] || continue
  refs="$(otool -L "$bin" 2>/dev/null | tail -n +2 | awk '{print $1}' || true)"
  [ -n "$refs" ] || continue

  # A dylib's OWN install name is the first otool -L entry. It is not a
  # dependency: dyld resolves loads using the path recorded in the *dependent*
  # binary, so a stale absolute self-ID is cosmetic. Separate the two, or a
  # harmless ID gets reported as a fatal leak.
  selfid="$(own_install_name "$bin" 2>/dev/null || true)"
  if [ -n "$selfid" ]; then
    deps="$(printf '%s\n' "$refs" | grep -Fxv "$selfid" || true)"
  else
    deps="$refs"
  fi

  bad="$(printf '%s\n' "$deps" \
         | grep -E "^($HOMEBREW_PREFIX|/usr/local/(lib|opt|Cellar))" || true)"
  intree="$(printf '%s\n' "$deps" | grep -F "$BUILD_TREE" || true)"
  if [ -n "$bad" ] || [ -n "$intree" ]; then
    {
      echo "${bin#"$APP"/}: unresolvable dependencies"
      printf '%s\n%s\n' "$bad" "$intree" | grep -v '^$' | sed 's/^/    /' || true
    } >> "$LEAKS"
  fi

  # Report a stale self-ID separately, and non-fatally.
  case "$selfid" in
    "$HOMEBREW_PREFIX"/*|/usr/local/lib/*|/usr/local/opt/*|/usr/local/Cellar/*)
      echo "${bin#"$APP"/}: stale install name ($selfid)" >> "$WORK/staleids" ;;
  esac
done <<EOF
$(all_machos)
EOF

if [ -s "$WORK/staleids" ]; then
  warn "stale install names (cosmetic — dyld uses the dependent's path, not these):"
  sed 's/^/    /' "$WORK/staleids" >&2
fi

if [ -s "$LEAKS" ]; then
  cat "$LEAKS" >&2
  die "absolute library paths survived — this app will crash on machines without Homebrew"
fi
log "clean: no unresolvable dependencies remain"

# --- architecture audit ----------------------------------------------------
# Catches an x86_64-only dependency that would force the whole app under
# Rosetta, or fail to load entirely.

log "auditing architectures (require: $ARCH)"
: > "$WORK/archbad"
while IFS= read -r bin; do
  [ -n "$bin" ] || continue
  [ -f "$bin" ] || continue
  is_macho "$bin" || continue
  a="$(lipo -archs "$bin" 2>/dev/null || echo '?')"
  case " $a " in
    *" $ARCH "*) ;;
    *) echo "    ${bin#"$APP"/}: $a" >> "$WORK/archbad" ;;
  esac
done <<EOF
$(all_machos)
EOF
if [ -s "$WORK/archbad" ]; then
  warn "these binaries do not contain $ARCH:"
  cat "$WORK/archbad" >&2
  die "architecture audit failed"
fi
log "all embedded binaries contain $ARCH"

# --- minimum-OS audit ------------------------------------------------------
# Your deployment target is a floor you PROMISE, not one you get. If a bundled
# Homebrew or Qt binary was built for a newer macOS than the app claims, the
# app will install on old systems and then crash on a missing symbol. The real
# floor is the highest minos across everything you ship.

minos_of() {
  otool -l "$1" 2>/dev/null | awk '
    /LC_BUILD_VERSION/     { b=1; next }
    b && /minos/           { print $2; b=0; next }
    /LC_VERSION_MIN_MACOSX/{ v=1; next }
    v && /version/         { print $2; v=0; next }
  ' | head -1
}

APP_MINOS="$(minos_of "$MAIN_BINARY" || true)"
: > "$WORK/minos"
while IFS= read -r bin; do
  [ -n "$bin" ] || continue
  [ -f "$bin" ] || continue
  is_macho "$bin" || continue
  m="$(minos_of "$bin" || true)"
  [ -n "$m" ] && echo "$m|$(basename "$bin")" >> "$WORK/minos" || true
done <<EOF
$(all_machos)
EOF
if [ -s "$WORK/minos" ]; then
  WORST="$(sort -t'|' -k1,1 -V "$WORK/minos" | tail -1)"
  WORST_VER="${WORST%%|*}"; WORST_LIB="${WORST##*|}"
  log "app minos: ${APP_MINOS:-unknown}   highest dependency minos: $WORST_VER ($WORST_LIB)"

  # The single highest value is often one of YOUR OWN libraries, built with no
  # deployment target and so defaulting to the host SDK. That masks the real
  # constraint, which is the highest minos among third-party binaries you
  # cannot rebuild. Show the top of the list so the two can be told apart.
  log "highest 10 minimum-OS requirements in the bundle:"
  # sort -Vr, not `| tac`: macOS has no tac.
  sort -t'|' -k1,1 -Vr "$WORK/minos" 2>/dev/null | head -10 \
    | awk -F'|' '{printf "        %-8s %s\n", $1, $2}' || true
  if [ -n "$APP_MINOS" ] \
     && [ "$(printf '%s\n%s\n' "$APP_MINOS" "$WORST_VER" | sort -V | tail -1)" != "$APP_MINOS" ]; then
    warn "$WORST_LIB requires macOS $WORST_VER but the app claims $APP_MINOS."
    warn "Set CMAKE_OSX_DEPLOYMENT_TARGET to $WORST_VER or higher, or the app"
    warn "will launch on older systems and then crash on a missing symbol."
  fi

  # LSMinimumSystemVersion is what makes macOS REFUSE to launch the app with a
  # clear "requires macOS X" dialog. Without it, an older system happily starts
  # the app and it dies on a missing symbol instead. The deployment target
  # alone does not do this — the two are independent.
  LSMIN="$(plutil -extract LSMinimumSystemVersion raw -o - \
           "$APP/Contents/Info.plist" 2>/dev/null || true)"
  if [ -z "$LSMIN" ]; then
    warn "Info.plist sets no LSMinimumSystemVersion."
    warn "Older systems will launch this app and crash rather than refusing"
    warn "it cleanly. Add <key>LSMinimumSystemVersion</key><string>$WORST_VER</string>"
  elif [ "$(printf '%s\n%s\n' "$LSMIN" "$WORST_VER" | sort -V | tail -1)" != "$LSMIN" ]; then
    warn "LSMinimumSystemVersion is $LSMIN but the bundle needs $WORST_VER."
  else
    log "LSMinimumSystemVersion: $LSMIN (covers the $WORST_VER requirement)"
  fi
fi

# The Cocoa plugin is invisible to otool, so check for it explicitly.
if [ ! -f "$PLUGINS/platforms/libqcocoa.dylib" ]; then
  warn "Contents/PlugIns/platforms/libqcocoa.dylib is MISSING."
  warn "The app will fail at launch with 'no Qt platform plugin could be initialized'."
fi

if spctl -a -vvv -t exec "$APP" 2>&1 | grep -q accepted; then
  log "Gatekeeper: accepted"
else
  warn "Gatekeeper: rejected (expected when not notarized)."
  warn "Downloaded copies need System Settings > Privacy & Security > Open Anyway."
fi

# ---------------------------------------------------------------------------
# 7. Optional notarization
# ---------------------------------------------------------------------------

if [ "$DO_NOTARIZE" = "1" ]; then
  log "notarizing (profile: $KEYCHAIN_PROFILE)"
  # One-time: xcrun notarytool store-credentials AC_NOTARY \
  #             --apple-id you@example.com --team-id TEAMID --password <app-pw>
  ZIP="$WORK/$APP_NAME.zip"
  ditto -c -k --keepParent "$APP" "$ZIP"
  xcrun notarytool submit "$ZIP" --keychain-profile "$KEYCHAIN_PROFILE" --wait \
    || die "notarization failed. Inspect: xcrun notarytool log <id> --keychain-profile $KEYCHAIN_PROFILE"
  xcrun stapler staple "$APP" || die "stapling failed"
  log "notarized and stapled"
fi

# ---------------------------------------------------------------------------
# 8. Optional disk image
# ---------------------------------------------------------------------------

if [ "$DO_DMG" = "1" ]; then
  DMG="$(dirname "$APP")/$APP_NAME.dmg"
  log "building $DMG"
  STAGE="$WORK/dmg"; mkdir -p "$STAGE"
  cp -R "$APP" "$STAGE/"
  ln -s /Applications "$STAGE/Applications"
  rm -f "$DMG"
  hdiutil create -volname "$APP_NAME" -srcfolder "$STAGE" -ov -format UDZO "$DMG" >/dev/null
  codesign --force --timestamp --sign "$IDENTITY" "$DMG"
  # A stapled ticket must exist for THIS artifact. Notarizing the .app earlier
  # produces a ticket for the app only — the .dmg is a separate submission, and
  # stapling it without submitting it fails with "Record not found ... Error 65".
  # The app inside is already notarized, so this pass is quick.
  if [ "$DO_NOTARIZE" = "1" ]; then
    log "notarizing the disk image"
    xcrun notarytool submit "$DMG" --keychain-profile "$KEYCHAIN_PROFILE" --wait \
      || warn "dmg notarization failed — the .app inside is still notarized"
    xcrun stapler staple "$DMG" \
      || warn "could not staple the dmg"
  fi
  log "created $DMG"
fi

CLEAN_EXIT=1
log "done: $APP"
log "Test on a Mac without Homebrew, or: sudo mv $HOMEBREW_PREFIX ${HOMEBREW_PREFIX}.bak"
