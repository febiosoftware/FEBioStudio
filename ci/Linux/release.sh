#!/bin/bash

# Get necessary packages
sudo apt install -y patchelf zip

FEBioRepo=$HOME/FEBio
ChemRepo=$HOME/FEBioChem
HeatRepo=$HOME/FEBioHeat
FBSRepo=$HOME/FEBioStudio

# Clone and build repos
git clone https://github.com/febiosoftware/FEBio.git $FEBioRepo
git -C $FEBioRepo checkout ci/develop
cd $FEBioRepo
./ci/Linux/build.sh
./ci/Linux/create-sdk.sh


git clone https://github.com/febiosoftware/FEBioChem.git $ChemRepo
ln -s $FEBioRepo/febio4-sdk $ChemRepo/
git -C $ChemRepo checkout ci/develop
cd $ChemRepo
./ci/Linux/build.sh

git clone https://github.com/febiosoftware/FEBioHeat.git $HeatRepo
ln -s $FEBioRepo/febio4-sdk $HeatRepo/
git -C $HeatRepo checkout ci/develop
cd $HeatRepo
./ci/Linux/build.sh

git clone https://github.com/febiosoftware/FEBioStudio.git $FBSRepo
ln -s $FEBioRepo/febio4-sdk $FBSRepo/
git -C $FBSRepo checkout ci/develop
cd $FBSRepo
./ci/Linux/build.sh

cd $HOME

mkdir release
mkdir release/bin
mkdir release/lib

bins=(
    $FEBioRepo/cmbuild/bin/febio4
    $FBSRepo/cmbuild/bin/FEBioStudio
    $FBSRepo/cmbuild/bin/FEBioStudioUpdater
    $FBSRepo/cmbuild/bin/mvUtil
)

libs=(
    # FEBio
    $FEBioRepo/cmbuild/lib/libfebiolib.so
    $FEBioRepo/cmbuild/lib/libfecore.so
    $FEBioRepo/cmbuild/lib/libnumcore.so
    $FEBioRepo/cmbuild/lib/libfebioopt.so
    $FEBioRepo/cmbuild/lib/libfebiofluid.so
    $FEBioRepo/cmbuild/lib/libfeamr.so
    $FEBioRepo/cmbuild/lib/libfebiorve.so
    $FEBioRepo/cmbuild/lib/libfeimglib.so
    $FEBioRepo/cmbuild/lib/libfebiomix.so
    $FEBioRepo/cmbuild/lib/libfebiomech.so

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
    cp $item release/bin
done


for item in ${libs[@]}; do
    cp $item release/lib
done

# Get Qt plugins
cp -r /lib/x86_64-linux-gnu/qt6/plugins/xcbglintegrations release/lib/
cp -r /lib/x86_64-linux-gnu/qt6/plugins/tls release/lib/

# Get Qt platforms
mkdir release/lib/platforms
cp /lib/x86_64-linux-gnu/qt6/plugins/platforms/libqxcb.so release/lib/platforms
cp /lib/x86_64-linux-gnu/libQt6XcbQpa.so.6 release/lib

patchelf --set-rpath '$ORIGIN/..' release/lib/platforms/libqxcb.so

# Fix up OCCT rpaths
patchelf --set-rpath '$ORIGIN/../lib' release/lib/libTK*

# Create qt.conf
echo "[Paths]
Plugins = ../lib" > release/bin/qt.conf

# Create docs
docs=(
    $FEBioRepo/Documentation/FEBio_EULA_4.pdf
    $FEBioRepo/Documentation/FEBio_Theory_Manual.pdf
    $FEBioRepo/Documentation/FEBio_User_Manual.pdf
    $FEBioRepo/Documentation/FEBio_User_Manual.pdf
    $FEBioRepo/Documentation/ReleaseNotes.txt
    $FBSRepo/Documentation/FEBioStudio_User_Manual.pdf
    $FBSRepo/Documentation/FEBioStudioReleaseNotes.txt
    $FBSRepo/icons/febiostudio.ico
)

mkdir release/doc

for item in ${docs[@]}; do
    cp $item release/doc
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

mkdir release/sdk
mkdir release/sdk/include
mkdir release/sdk/lib

for item in ${sdkDirs[@]}; do
    mkdir release/sdk/include/$item
    cp $FEBioRepo/$item/*.h release/sdk/include/$item
    cp $FEBioRepo/$item/*.hpp release/sdk/include/$item
done

for item in ${sdkLibs[@]}; do
    cp $FEBioRepo/cmbuild/lib/$item release/sdk/lib
done

zip -r release/sdk.zip release/sdk


# Create installer
builder build $FBSRepo/ci/installBuilder.xml --license license.xml