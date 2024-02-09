#!/bin/bash

mkdir release
cd release
mkdir bin
mkdir lib
mkdir sdk

bins=(
    /home/ubuntu/FEBio/cmbuild/bin/febio4
    /home/ubuntu/FEBioStudio/cmbuild/bin/FEBioStudio
    /home/ubuntu/FEBioStudio/cmbuild/bin/FEBioStudioUpdater
    /home/ubuntu/FEBioStudio/cmbuild/bin/mvUtil
)

libs=(
    # FEBio
    /home/ubuntu/FEBio/cmbuild/lib/libfebiolib.so
    /home/ubuntu/FEBio/cmbuild/lib/libfecore.so
    /home/ubuntu/FEBio/cmbuild/lib/libnumcore.so
    /home/ubuntu/FEBio/cmbuild/lib/libfebioopt.so
    /home/ubuntu/FEBio/cmbuild/lib/libfebiofluid.so
    /home/ubuntu/FEBio/cmbuild/lib/libfeamr.so
    /home/ubuntu/FEBio/cmbuild/lib/libfebiorve.so
    /home/ubuntu/FEBio/cmbuild/lib/libfeimglib.so
    /home/ubuntu/FEBio/cmbuild/lib/libfebiomix.so
    /home/ubuntu/FEBio/cmbuild/lib/libfebiomech.so

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
    /lib/x86_64-linux-gnu/libavcodec.so.58
        /lib/x86_64-linux-gnu/libswresample.so.3
            /lib/x86_64-linux-gnu/libsoxr.so.0
        /lib/x86_64-linux-gnu/libvpx.so.7
        /lib/x86_64-linux-gnu/libwebpmux.so.3
        /lib/x86_64-linux-gnu/libwebp.so.7
        /lib/x86_64-linux-gnu/liblzma.so.5
        /lib/x86_64-linux-gnu/libdav1d.so.5
        /lib/x86_64-linux-gnu/librsvg-2.so.2
            /lib/x86_64-linux-gnu/libcairo-gobject.so.2
            /lib/x86_64-linux-gnu/libgdk_pixbuf-2.0.so.0
                /lib/x86_64-linux-gnu/libgmodule-2.0.so.0
                /lib/x86_64-linux-gnu/libpng16.so.16
                /lib/x86_64-linux-gnu/libjpeg.so.8
            /lib/x86_64-linux-gnu/libgio-2.0.so.0
                /lib/x86_64-linux-gnu/libmount.so.1
                    /lib/x86_64-linux-gnu/libblkid.so.1
                /lib/x86_64-linux-gnu/libselinux.so.1
                    /lib/x86_64-linux-gnu/libpcre2-8.so.0
            /lib/x86_64-linux-gnu/libxml2.so.2
                /lib/x86_64-linux-gnu/libicuuc.so.70
                    /lib/x86_64-linux-gnu/libicudata.so.70
            /lib/x86_64-linux-gnu/libpangocairo-1.0.so.0
                /lib/x86_64-linux-gnu/libpangoft2-1.0.so.0
                    /lib/x86_64-linux-gnu/libfreetype.so.6
                        /lib/x86_64-linux-gnu/libbrotlidec.so.1
                            /lib/x86_64-linux-gnu/libbrotlicommon.so.1
                /lib/x86_64-linux-gnu/libharfbuzz.so.0
                    /lib/x86_64-linux-gnu/libgraphite2.so.3
                /lib/x86_64-linux-gnu/libfontconfig.so.1
                    /lib/x86_64-linux-gnu/libexpat.so.1
                    /lib/x86_64-linux-gnu/libuuid.so.1
            /lib/x86_64-linux-gnu/libpango-1.0.so.0
                /lib/x86_64-linux-gnu/libfribidi.so.0
                /lib/x86_64-linux-gnu/libthai.so.0
                    /lib/x86_64-linux-gnu/libdatrie.so.1
        /lib/x86_64-linux-gnu/libgobject-2.0.so.0
            /lib/x86_64-linux-gnu/libffi.so.8
        /lib/x86_64-linux-gnu/libglib-2.0.so.0
            /lib/x86_64-linux-gnu/libpcre.so.3
        /lib/x86_64-linux-gnu/libcairo.so.2
            /lib/x86_64-linux-gnu/libpixman-1.so.0
            /lib/x86_64-linux-gnu/libxcb-shm.so.0
            /lib/x86_64-linux-gnu/libxcb.so.1
                /lib/x86_64-linux-gnu/libXau.so.6
                /lib/x86_64-linux-gnu/libXdmcp.so.6
                    /lib/x86_64-linux-gnu/libbsd.so.0
                        /lib/x86_64-linux-gnu/libmd.so.0
            /lib/x86_64-linux-gnu/libxcb-render.so.0
            /lib/x86_64-linux-gnu/libXrender.so.1
            /lib/x86_64-linux-gnu/libX11.so.6
            /lib/x86_64-linux-gnu/libXext.so.6
        /lib/x86_64-linux-gnu/libzvbi.so.0
        /lib/x86_64-linux-gnu/libsnappy.so.1
        /lib/x86_64-linux-gnu/libaom.so.3
        /lib/x86_64-linux-gnu/libcodec2.so.1.0
        /lib/x86_64-linux-gnu/libgsm.so.1
        /lib/x86_64-linux-gnu/libmp3lame.so.0
        /lib/x86_64-linux-gnu/libopenjp2.so.7
        /lib/x86_64-linux-gnu/libopus.so.0
        /lib/x86_64-linux-gnu/libshine.so.3
        /lib/x86_64-linux-gnu/libspeex.so.1
        /lib/x86_64-linux-gnu/libtheoraenc.so.1
            /lib/x86_64-linux-gnu/libogg.so.0
        /lib/x86_64-linux-gnu/libtheoradec.so.1
        /lib/x86_64-linux-gnu/libtwolame.so.0
        /lib/x86_64-linux-gnu/libvorbis.so.0
        /lib/x86_64-linux-gnu/libvorbisenc.so.2
        /lib/x86_64-linux-gnu/libx264.so.163
        /lib/x86_64-linux-gnu/libx265.so.199
            /lib/x86_64-linux-gnu/libnuma.so.1
        /lib/x86_64-linux-gnu/libxvidcore.so.4
            /lib/x86_64-linux-gnu/libpthread.so.0
        /lib/x86_64-linux-gnu/libva.so.2
        /lib/x86_64-linux-gnu/libmfx.so.1
    /lib/x86_64-linux-gnu/libavutil.so.56
        /lib/x86_64-linux-gnu/libva-drm.so.2
        /lib/x86_64-linux-gnu/libva-x11.so.2
            /lib/x86_64-linux-gnu/libXfixes.so.3
        /lib/x86_64-linux-gnu/libvdpau.so.1
        /lib/x86_64-linux-gnu/libdrm.so.2
        /lib/x86_64-linux-gnu/libOpenCL.so.1
    /lib/x86_64-linux-gnu/libswscale.so.5

    # GLEW
    /lib/x86_64-linux-gnu/libGLEW.so.2.2
)

for item in ${bins[@]}; do
    cp $item bin
done


for item in ${libs[@]}; do
    cp $item lib
done

# Get xcbglintegrations
cp -r /lib/x86_64-linux-gnu/qt6/plugins/xcbglintegrations ./lib/

# Get Qt platforms
mkdir lib/platforms
cp /lib/x86_64-linux-gnu/qt6/plugins/platforms/libqxcb.so lib/platforms
cp /lib/x86_64-linux-gnu/libQt6XcbQpa.so.6 lib
patchelf --set-rpath '$ORIGIN/..' lib/platforms/libqxcb.so

# Fix up OCCT rpaths
patchelf --set-rpath '$ORIGIN/../lib' lib/libTK*

# Create qt.conf
echo "[Paths]
Plugins = ../lib" > bin/qt.conf
