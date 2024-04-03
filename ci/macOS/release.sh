#!/bin/bash

export FEBIO_REPO=$GITHUB_WORKSPACE/FEBio
export CHEM_REPO=$GITHUB_WORKSPACE/FEBioChem
export HEAT_REPO=$GITHUB_WORKSPACE/FEBioHeat
export FBS_REPO=$GITHUB_WORKSPACE/FEBioStudio
export RELEASE_DIR=$GITHUB_WORKSPACE/release
UPLOAD_DIR=$GITHUB_WORKSPACE/upload
APP_BUDLE=$FBS_REPO/cmbuild/bin/FEBioStudio.app

# Build repos

cd $FEBIO_REPO
./ci/macOS/build.sh
./ci/macOS/create-sdk.sh

ln -s $FEBIO_REPO/febio4-sdk $CHEM_REPO/
cd $CHEM_REPO
./ci/macOS/build.sh

ln -s $FEBIO_REPO/febio4-sdk $HEAT_REPO/
cd $HEAT_REPO
./ci/macOS/build.sh

ln -s $FEBIO_REPO/febio4-sdk $FBS_REPO/
cd $FBS_REPO
./ci/macOS/build.sh

cd $GITHUB_WORKSPACE

mkdir $RELEASE_DIR

cp -r $APP_BUDLE $RELEASE_DIR

mkdir $UPLOAD_DIR
mkdir $UPLOAD_DIR/MacOS
mkdir $UPLOAD_DIR/Frameworks
mkdir $UPLOAD_DIR/doc
mkdir $UPLOAD_DIR/updater

bins=(
    $APP_BUDLE/Contents/MacOS/febio4
    $APP_BUDLE/Contents/MacOS/FEBioStudio
    $APP_BUDLE/Contents/MacOS/febio.xml
)

updater=(
    $APP_BUDLE/Contents/MacOS/FEBioStudioUpdater
    $APP_BUDLE/Contents/MacOS/mvUtil
)

febioLibs=(
    $APP_BUDLE/Contents/Frameworks/libfebiolib.dylib
    $APP_BUDLE/Contents/Frameworks/libfecore.dylib
    $APP_BUDLE/Contents/Frameworks/libnumcore.dylib
    $APP_BUDLE/Contents/Frameworks/libfebioopt.dylib
    $APP_BUDLE/Contents/Frameworks/libfebiofluid.dylib
    $APP_BUDLE/Contents/Frameworks/libfeamr.dylib
    $APP_BUDLE/Contents/Frameworks/libfebiorve.dylib
    $APP_BUDLE/Contents/Frameworks/libfeimglib.dylib
    $APP_BUDLE/Contents/Frameworks/libfebiomix.dylib
    $APP_BUDLE/Contents/Frameworks/libfebiomech.dylib

    $APP_BUDLE/Contents/Frameworks/libFEBioChem.dylib
    $APP_BUDLE/Contents/Frameworks/libFEBioHeat.dylib
)

for item in ${bins[@]}; do
    cp $item $UPLOAD_DIR/MacOS
done

for item in ${updater[@]}; do
    cp $item $UPLOAD_DIR/updater
done

for item in ${febioLibs[@]}; do
    cp $item $UPLOAD_DIR/Frameworks
done

# Create docs
docs=(
    $FEBIO_REPO/Documentation/FEBio_EULA_4.pdf
    $FEBIO_REPO/Documentation/FEBio_Theory_Manual.pdf
    $FEBIO_REPO/Documentation/FEBio_User_Manual.pdf
    $FEBIO_REPO/Documentation/FEBio_User_Manual.pdf
    $FEBIO_REPO/Documentation/ReleaseNotes.txt
    $FBS_REPO/Documentation/FEBioStudio_User_Manual.pdf
    $FBS_REPO/Documentation/FEBioStudioReleaseNotes.txt
    $FBS_REPO/icons/febiostudio.ico
)

mkdir $RELEASE_DIR/doc

for item in ${docs[@]}; do
    cp $item $RELEASE_DIR/doc
    cp $item $UPLOAD_DIR/doc
done

# Create SDK
sdkDirs=(
    FECore
    FEBioMech
    FEBioMix
    FEBioFluid
    FEBioRVE
    FEBioPlot
    FEBioXML
    FEBioLib
)

sdkLibs=(
    libfecore.dylib
    libfebiomech.dylib
    libfebiomix.dylib
    libfebiofluid.dylib
    libfebiorve.dylib
    libfebioplot.a
    libxml.a
    libfebiolib.dylib
)

mkdir $RELEASE_DIR/sdk
mkdir $RELEASE_DIR/sdk/include
mkdir $RELEASE_DIR/sdk/lib

for item in ${sdkDirs[@]}; do
    mkdir $RELEASE_DIR/sdk/include/$item
    cp $FEBIO_REPO/$item/*.h $RELEASE_DIR/sdk/include/$item
    cp $FEBIO_REPO/$item/*.hpp $RELEASE_DIR/sdk/include/$item
done

for item in ${sdkLibs[@]}; do
    cp $FEBIO_REPO/cmbuild/lib/$item $RELEASE_DIR/sdk/lib
done

cd $RELEASE_DIR/sdk
zip -r $UPLOAD_DIR/sdk.zip include
zip -r $UPLOAD_DIR/sdk.zip libintel
cd $GITHUB_WORKSPACE

# Create installer
rm -r /Applications/InstallBuilder\ Enterprise\ 23.11.0/output/*
arch -x86_64 /Applications/InstallBuilder\ Enterprise\ 23.11.0/bin/Builder.app/Contents/MacOS/osx-x86_64 build $FBS_REPO/ci/installBuilder.xml --license $GITHUB_WORKSPACE/license.xml

INSTALLER_NAME=$(ls /Applications/InstallBuilder\ Enterprise\ 23.11.0/output/)

mkdir $UPLOAD_DIR/installer
cp -r /Applications/InstallBuilder\ Enterprise\ 23.11.0/output/$INSTALLER_NAME $UPLOAD_DIR/installer
rm -r /Applications/InstallBuilder\ Enterprise\ 23.11.0/output/*

# Notarize installer
cd $UPLOAD_DIR/installer
zip -r $INSTALLER_NAME.zip $INSTALLER_NAME
xcrun notarytool submit --apple-id $MACOS_SIGN_ID --team-id $MACOS_SIGN_TEAM --password $MACOS_SIGN_PWD $INSTALLER_NAME.zip --wait
xcrun stapler staple $INSTALLER_NAME
rm $INSTALLER_NAME.zip
cd $GITHUB_WORKSPACE