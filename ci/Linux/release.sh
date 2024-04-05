#!/bin/bash

export FEBIO_REPO=$GITHUB_WORKSPACE/FEBio
CHEM_REPO=$GITHUB_WORKSPACE/FEBioChem
HEAT_REPO=$GITHUB_WORKSPACE/FEBioHeat
export FBS_REPO=$GITHUB_WORKSPACE/FEBioStudio
export RELEASE_DIR=$GITHUB_WORKSPACE/release
UPLOAD_DIR=$GITHUB_WORKSPACE/upload

# Clone and build repos
cd $FEBIO_REPO
./ci/Linux/build.sh
./ci/Linux/create-sdk.sh

ln -s $FEBIO_REPO/febio4-sdk $CHEM_REPO/
cd $CHEM_REPO
./ci/Linux/build.sh

ln -s $FEBIO_REPO/febio4-sdk $HEAT_REPO/
cd $HEAT_REPO
./ci/Linux/build.sh

ln -s $FEBIO_REPO/febio4-sdk $FBS_REPO/
cd $FBS_REPO
./ci/Linux/build.sh

cd $GITHUB_WORKSPACE

mkdir $RELEASE_DIR
mkdir $RELEASE_DIR/bin
mkdir $RELEASE_DIR/lib

mkdir $UPLOAD_DIR
mkdir $UPLOAD_DIR/bin
mkdir $UPLOAD_DIR/lib
mkdir $UPLOAD_DIR/doc
mkdir $UPLOAD_DIR/updater

bins=(
    $FEBIO_REPO/cmbuild/bin/febio4
    $FBS_REPO/cmbuild/bin/FEBioStudio
    $FBS_REPO/ci/Linux/febio.xml
)

updater=(
    $FBS_REPO/cmbuild/bin/FEBioStudioUpdater
    $FBS_REPO/cmbuild/bin/mvUtil
)

febioLibs=(
    $FEBIO_REPO/cmbuild/lib/libfebiolib.so
    $FEBIO_REPO/cmbuild/lib/libfecore.so
    $FEBIO_REPO/cmbuild/lib/libnumcore.so
    $FEBIO_REPO/cmbuild/lib/libfebioopt.so
    $FEBIO_REPO/cmbuild/lib/libfebiofluid.so
    $FEBIO_REPO/cmbuild/lib/libfeamr.so
    $FEBIO_REPO/cmbuild/lib/libfebiorve.so
    $FEBIO_REPO/cmbuild/lib/libfeimglib.so
    $FEBIO_REPO/cmbuild/lib/libfebiomix.so
    $FEBIO_REPO/cmbuild/lib/libfebiomech.so

    $CHEM_REPO/cmbuild/lib/*.so
    $HEAT_REPO/cmbuild/lib/*.so
)

libs=(

    # IOMP
    /lib/x86_64-linux-gnu/libiomp5.so

    # ZLib
    /lib/x86_64-linux-gnu/libz.so.1

    # Qt
    /lib/x86_64-linux-gnu/libQt6Network.so.6
        /lib/x86_64-linux-gnu/libzstd.so.1
        /lib/x86_64-linux-gnu/libproxy.so.1
    /lib/x86_64-linux-gnu/libQt6OpenGLWidgets.so.6
        /lib/x86_64-linux-gnu/libQt6OpenGL.so.6
    /lib/x86_64-linux-gnu/libQt6Widgets.so.6
    /lib/x86_64-linux-gnu/libQt6Gui.so.6
        /lib/x86_64-linux-gnu/libEGL.so.1
        /lib/x86_64-linux-gnu/libQt6DBus.so.6
            /lib/x86_64-linux-gnu/libdbus-1.so.3
                /lib/x86_64-linux-gnu/libsystemd.so.0
                    /lib/x86_64-linux-gnu/liblz4.so.1
                    /lib/x86_64-linux-gnu/libcap.so.2
                    /lib/x86_64-linux-gnu/libgcrypt.so.20
                        /lib/x86_64-linux-gnu/libgpg-error.so.0
        /lib/x86_64-linux-gnu/libxkbcommon.so.0
        /lib/x86_64-linux-gnu/libmd4c.so.0
    /lib/x86_64-linux-gnu/libQt6Core.so.6
        /lib/x86_64-linux-gnu/libicui18n.so.70
        /lib/x86_64-linux-gnu/libdouble-conversion.so.3
        /lib/x86_64-linux-gnu/libb2.so.1
        /lib/x86_64-linux-gnu/libpcre2-16.so.0

    # NetGen
    /usr/local/lib/libnglib.so
        /usr/local/lib/libngcore.so
        /usr/local/lib/libTKSTL.so.7.7
            /usr/local/lib/libTKXDE.so.7.7
        /usr/local/lib/libTKXDEIGES.so.7.7
        /usr/local/lib/libTKBool.so.7.7
        /usr/local/lib/libTKXDESTEP.so.7.7
            /usr/local/lib/libTKSTEPAttr.so.7.7
        /usr/local/lib/libTKSTEPBase.so.7.7
        /usr/local/lib/libTKXCAF.so.7.7
            /usr/local/lib/libTKVCAF.so.7.7
            /usr/local/lib/libTKV3d.so.7.7
                /usr/local/lib/libTKHLR.so.7.7
            /usr/local/lib/libTKService.so.7.7
            /usr/local/lib/libTKCAF.so.7.7
            /usr/local/lib/libTKCDF.so.7.7
        /usr/local/lib/libTKLCAF.so.7.7
        /usr/local/lib/libTKShHealing.so.7.7
        /usr/local/lib/libTKGeomAlgo.so.7.7
        /usr/local/lib/libTKG2d.so.7.7

    # OCCT
    /usr/local/lib/libTKernel.so.7.7
    /usr/local/lib/libTKGeomBase.so.7.7
    /usr/local/lib/libTKTopAlgo.so.7.7
    /usr/local/lib/libTKPrim.so.7.7
    /usr/local/lib/libTKMesh.so.7.7
    /usr/local/lib/libTKMath.so.7.7
    /usr/local/lib/libTKBRep.so.7.7
    /usr/local/lib/libTKFillet.so.7.7
    /usr/local/lib/libTKBO.so.7.7
    /usr/local/lib/libTKIGES.so.7.7
    /usr/local/lib/libTKSTEP.so.7.7
        /usr/local/lib/libTKSTEP209.so.7.7
    /usr/local/lib/libTKXSBase.so.7.7
    /usr/local/lib/libTKG3d.so.7.7


    # libSSH
    /lib/x86_64-linux-gnu/libssh.so.4
        /lib/x86_64-linux-gnu/libgssapi_krb5.so.2
            /lib/x86_64-linux-gnu/libkrb5.so.3
                /lib/x86_64-linux-gnu/libkeyutils.so.1
                /lib/x86_64-linux-gnu/libresolv.so.2
            /lib/x86_64-linux-gnu/libk5crypto.so.3
            /lib/x86_64-linux-gnu/libcom_err.so.2
            /lib/x86_64-linux-gnu/libkrb5support.so.0
    /lib/x86_64-linux-gnu/libcrypto.so.3

    # libZip
    /usr/local/lib/libzip.so.5

    # SQLITE
    /lib/x86_64-linux-gnu/libsqlite3.so.0

    # FFMPEG
    /usr/local/lib/libavcodec.so.60
    /usr/local/lib/libavutil.so.58
    /usr/local/lib/libswscale.so.7

    # GLEW
    /lib/x86_64-linux-gnu/libGLEW.so.2.2
)

for item in ${bins[@]}; do
    cp $item $RELEASE_DIR/bin
    cp $item $UPLOAD_DIR/bin
done

for item in ${updater[@]}; do
    cp $item $RELEASE_DIR/bin
    cp $item $UPLOAD_DIR/updater
done

for item in ${febioLibs[@]}; do
    cp $item $RELEASE_DIR/lib
    cp $item $UPLOAD_DIR/lib
done

for item in ${libs[@]}; do
    cp $item $RELEASE_DIR/lib
done

# Get Qt plugins
cp -r /lib/x86_64-linux-gnu/qt6/plugins/xcbglintegrations $RELEASE_DIR/lib/
cp -r /lib/x86_64-linux-gnu/qt6/plugins/tls $RELEASE_DIR/lib/

# Get Qt platforms
mkdir $RELEASE_DIR/lib/platforms
cp /lib/x86_64-linux-gnu/qt6/plugins/platforms/libqxcb.so $RELEASE_DIR/lib/platforms
cp /lib/x86_64-linux-gnu/libQt6XcbQpa.so.6 $RELEASE_DIR/lib

patchelf --set-rpath '$ORIGIN/..' $RELEASE_DIR/lib/platforms/libqxcb.so

# Fix up OCCT rpaths
patchelf --set-rpath '$ORIGIN/../lib' $RELEASE_DIR/lib/libTK*

# Create qt.conf
echo "[Paths]
Plugins = ../lib" > $RELEASE_DIR/bin/qt.conf

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
    libfecore.so
    libfebiomech.so
    libfebiomix.so
    libfebiofluid.so
    libfebiorve.so
    libfebioplot.a
    libxml.a
    libfebiolib.so
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
zip -r $UPLOAD_DIR/sdk.zip lib
cd $GITHUB_WORKSPACE

# Set FEBio and FBS versions in installBuilder.xml
while IFS='' read -r a; do
    echo "${a//FEBIO_VER/$FEBIO_VER}"
done < $FBS_REPO/ci/installBuilder.xml > $FBS_REPO/ci/installBuilder.xml.t
mv $FBS_REPO/ci/installBuilder.xml{.t,}

while IFS='' read -r a; do
    echo "${a//FBS_VER/$FBS_VER}"
done < $FBS_REPO/ci/installBuilder.xml > $FBS_REPO/ci/installBuilder.xml.t
mv $FBS_REPO/ci/installBuilder.xml{.t,}

# Create installer
builder build $FBS_REPO/ci/installBuilder.xml --license $GITHUB_WORKSPACE/license.xml

mkdir $UPLOAD_DIR/installer
cp /opt/installbuilder-23.11.0/output/*.run $UPLOAD_DIR/installer