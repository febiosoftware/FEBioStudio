#!/usr/bin/env bash
#
# release-macos.sh — build, bundle, sign, notarize and package FEBio Studio
#                    into a distributable .dmg.
#
# Usage:
#     ./release-macos.sh              build + bundle + sign + notarize + dmg
#     ./release-macos.sh --local      build + bundle + sign only (no Apple round trip)
#
# One-time setup, before the first notarized run:
#
#   1. A Developer ID Application certificate must be installed. Check with:
#          security find-identity -v -p codesigning
#      An "Apple Development" certificate is NOT sufficient — Gatekeeper
#      rejects it on every Mac except the one that built the app.
#
#   2. Store notarytool credentials in the keychain once:
#          xcrun notarytool store-credentials AC_NOTARY \
#              --apple-id <your-developer-account-email> \
#              --team-id GHS68DTHQR \
#              --password <app-specific-password>
#      The app-specific password is generated at account.apple.com under
#      Sign-In and Security. Resetting your Apple ID password revokes it.
#
# Dependency search paths are declared below. They are machine-specific: if you
# move a checkout or upgrade Netgen, update SEARCH_PATHS or the packaging step
# will report "unresolved" and produce a bundle that will not launch elsewhere.

set -euo pipefail

cd "$(dirname "$0")"

# --- configuration ---------------------------------------------------------

IDENTITY="Developer ID Application: Gerard Ateshian (GHS68DTHQR)"
KEYCHAIN_PROFILE="AC_NOTARY"
BUILD_DIR="build"
APP="$BUILD_DIR/bin/Release/FEBioStudio.app"
DMG="$BUILD_DIR/bin/Release/FEBioStudio.dmg"

FEBIO_BUILD="$HOME/GitHub/FEBio-safe/build"
SIMPLEITK_BUILD="$HOME/GitHub/SimpleITK/SuperBuild/build"

SEARCH_PATHS=(
    --search-path "$FEBIO_BUILD/lib/Release"
    --search-path "/Applications/Netgen.app/Contents/MacOS"
    --search-path "$SIMPLEITK_BUILD/SimpleITK-build/lib"
    --search-path "$SIMPLEITK_BUILD/ITK-prefix/lib"
)

# --- arguments -------------------------------------------------------------

NOTARIZE_ARGS=( --dmg --notarize )
for arg in "$@"; do
    case "$arg" in
        --local) NOTARIZE_ARGS=() ;;
        *) echo "unknown option: $arg" >&2; exit 1 ;;
    esac
done

# --- preflight -------------------------------------------------------------

if ! security find-identity -v -p codesigning | grep -Fq "$IDENTITY"; then
    echo "error: signing identity not found in the keychain:" >&2
    echo "    $IDENTITY" >&2
    echo "run 'security find-identity -v -p codesigning' to see what is available" >&2
    exit 1
fi

if [ "${#NOTARIZE_ARGS[@]}" -gt 0 ]; then
    # Fails fast on a missing or misnamed keychain profile, rather than after a
    # full build and a 150 MB upload.
    if ! xcrun notarytool history --keychain-profile "$KEYCHAIN_PROFILE" >/dev/null 2>&1; then
        echo "error: no usable notarytool profile named '$KEYCHAIN_PROFILE'" >&2
        echo "see the store-credentials command in the header of this script" >&2
        exit 1
    fi
fi

# --- build -----------------------------------------------------------------

# The .app must be deleted, not updated. package-macos-app.sh refuses to run on
# a bundle that already has a populated Contents/Frameworks, because macdeployqt
# is not idempotent and a second pass silently corrupts the result.
echo "==> removing previous bundle"
rm -rf "$APP" "$DMG"

echo "==> building"
cmake --build "$BUILD_DIR" --target FEBioStudio --config Release

# --- package ---------------------------------------------------------------

echo "==> packaging"
./package-macos-app.sh "$APP" \
    "${SEARCH_PATHS[@]}" \
    --macdeployqt /opt/homebrew/bin/macdeployqt \
    --add-file ci/macOS/febio.xml \
    --add-file "$FEBIO_BUILD/bin/Release/febio4:MacOS/febio4" \
    --entitlements ci/macOS/entitlements.plist \
    --identity "$IDENTITY" \
    --keychain-profile "$KEYCHAIN_PROFILE" \
    ${NOTARIZE_ARGS[@]+"${NOTARIZE_ARGS[@]}"}

# --- report ----------------------------------------------------------------

echo
if [ "${#NOTARIZE_ARGS[@]}" -gt 0 ]; then
    echo "==> verifying the notarization ticket"
    xcrun stapler validate "$APP"
    xcrun stapler validate "$DMG"
    echo
    echo "distributable: $DMG"
    echo
    echo "Before shipping, confirm the bundle is self-contained by hiding"
    echo "Homebrew and launching the installed copy:"
    echo "    sudo mv /opt/homebrew /opt/homebrew.bak"
    echo "    open $APP"
    echo "    sudo mv /opt/homebrew.bak /opt/homebrew"
else
    echo "signed (not notarized): $APP"
    echo "re-run without --local to produce a distributable dmg"
fi
