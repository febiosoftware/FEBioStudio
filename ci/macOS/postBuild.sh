#!/bin/bash

FBS_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/../../

# Copy in Updater Stuff
cp $FBS_DIR/cmbuild/bin/FEBioStudioUpdater $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/MacOS
cp $FBS_DIR/cmbuild/bin/mvUtil $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/MacOS

cp $FBS_DIR/ci/macOS/Info.plist $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents

mkdir $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/Frameworks

# Copy in FEBio - only used during release workflow
if [ -n "${FEBIO_REPO}" ]; then
    echo "FEBIO_REPO set"
    cp $FEBIO_REPO/cmbuild/bin/febio4 $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/MacOS
    cp $FEBIO_REPO/cmbuild/lib/*.dylib $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/Frameworks

    cp $FBS_DIR/ci/macOS/febio.xml $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/MacOS
fi

# Copy in Plugins  - only used during release workflow
if [ -n "${CHEM_REPO}" ]; then
    echo "CHEM_REPO set"
    cp $CHEM_REPO/cmbuild/lib/libFEBioChem.dylib $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/Frameworks
fi

if [ -n "${HEAT_REPO}" ]; then
    echo "HEAT_REPO set"
    cp $HEAT_REPO/cmbuild/lib/libFEBioHeat.dylib $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/Frameworks
fi

macdeployqt $FBS_DIR/cmbuild/bin/FEBioStudio.app -executable=$FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/MacOS/FEBioStudioUpdater

cp "$HOME/local/x86_64/lib/libTKOffset.7.7.dylib" $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/Frameworks
cp "$HOME/local/x86_64/lib//libTKXDEIGES.7.7.dylib" $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/Frameworks
cp "$HOME/local/x86_64/lib//libTKXDESTEP.7.7.dylib" $FBS_DIR/cmbuild/bin/FEBioStudio.app/Contents/Frameworks
