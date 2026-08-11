#!/bin/bash
# set -e
RUN_POST_BUILD=${RUN_POST_BUILD:=true}

security unlock-keychain -p "$MACOS_KEYCHAIN_PASSWORD" "$MACOS_KEYCHAIN"

# echo "List of keychains:"
# security list-keychains -d user

# echo -e "\n\nDefault keychain:"
# security default-keychain -d user

# echo -e "\n\nDev ID Cert Auth:"
# security find-certificate -a -c "Developer ID Certification Authority" -Z

# echo -e "\n\nApple root:"
# security find-certificate -a \
#   -c "Apple Root CA" \
#   -Z

# echo -e "\n\nSystem root store:"
# security find-certificate \
#   -c "Apple Root CA" \
#   -Z \
#   /System/Library/Keychains/SystemRootCertificates.keychain || true

security find-certificate \
  -c "Developer ID Application" \
  -p "$MACOS_KEYCHAIN" > /tmp/developer-id.cer

security verify-cert \
  -c /tmp/developer-id.cer \
  -p codesigning


# cp /bin/echo ./sign-test

# codesign --force --verbose=4 --sign "$MACOS_SIGN" ./sign-test

# security import certificate.p12 \
#   -k "$MACOS_KEYCHAIN" \
#   -P "$P12_PASSWORD" \
#   -T /usr/bin/codesign

# security set-key-partition-list \
#   -S apple-tool:,apple:,codesign: \
#   -s \
#   -k "$MACOS_KEYCHAIN_PASSWORD" \
#   "$MACOS_KEYCHAIN"

# . $(dirname $0)/cmake.sh

# main() {
# 	run_cmake
# 	pushd cmbuild
# 	make -j $(sysctl -n hw.ncpu)

# 	# ctest --output-on-failure
# 	./bin/fbs-test-suite
# 	popd

# 	if [ "$RUN_POST_BUILD" = true ]; then
# 		echo "Running postbuild.sh"
# 		 ./$(dirname $0)/postBuild.sh
# 	else
# 		echo "Skipping postbuild.sh"
# 	fi
# }

# main

