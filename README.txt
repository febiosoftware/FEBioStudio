# These are the commands needed to compile FEBioStudio on arm64 for distribution
rm -rf build/bin/Release/FEBioStudio.app
cmake --build build --target FEBioStudio --config Release

./package-macos-app.sh build/bin/Release/FEBioStudio.app \
    --search-path ~/GitHub/FEBio-safe/build/lib/Release \
    --macdeployqt /opt/homebrew/bin/macdeployqt \
    --add-file ci/macOS/febio.xml \
    --add-file ~/GitHub/FEBio-safe/build/bin/Release/febio4:MacOS/febio4 \
    --entitlements ci/macOS/entitlements.plist \
    --dmg \
    --identity "Apple Development: Gerard Ateshian (F29GW7HU7K)" \
    2>&1 | grep -v 'replacing existing signature' | tail -40
