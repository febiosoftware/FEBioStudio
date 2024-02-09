$bins = @(
    # FEBio
    'C:\Users\Administrator\FEBio\cmbuild\bin\Release\febio4.exe'
    'C:\Users\Administrator\FEBio\cmbuild\bin\Release\febio.xml'
    'C:\Users\Administrator\FEBio\cmbuild\bin\Release\*.dll'
    'C:\Program Files (x86)\Intel\oneAPI\compiler\latest\windows\redist\intel64_win\compiler\libiomp5md.dll'

    #FEBio Studio
    'C:\Users\Administrator\FEBioStudio\cmbuild\bin\Release\FEBioStudio2.exe'
    'C:\Users\Administrator\FEBioStudio\cmbuild\bin\Release\FEBioStudioUpdater.exe'
    'C:\Users\Administrator\FEBioStudio\cmbuild\bin\Release\mvUtil.exe'

    # Qt
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\Qt6Core.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\Qt6Widgets.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\Qt6Gui.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\Qt6OpenGLWidgets.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\Qt6OpenGL.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\Qt6Network.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\Qt6DBus.dll'

    # ZLIB
    'C:\vcpkg\buildtrees\zlib\x64-windows-rel\zlib1.dll'

    # NetGen
    'C:\usr\local\bin\nglib.dll'
    'C:\usr\local\bin\ngcore.dll'

    # OCCT
    'C:\usr\local\win64\vc14\bin\TKSTL.dll'
    'C:\usr\local\win64\vc14\bin\TKXDE.dll'
    'C:\usr\local\win64\vc14\bin\TKXDEIGES.dll'
    'C:\usr\local\win64\vc14\bin\TKBool.dll'
    'C:\usr\local\win64\vc14\bin\TKXDESTEP.dll'
    'C:\usr\local\win64\vc14\bin\TKSTEPAttr.dll'
    'C:\usr\local\win64\vc14\bin\TKSTEPBase.dll'
    'C:\usr\local\win64\vc14\bin\TKXCAF.dll'
    'C:\usr\local\win64\vc14\bin\TKVCAF.dll'
    'C:\usr\local\win64\vc14\bin\TKV3d.dll'
    'C:\usr\local\win64\vc14\bin\TKHLR.dll'
    'C:\usr\local\win64\vc14\bin\TKService.dll'
    'C:\usr\local\win64\vc14\bin\TKCAF.dll'
    'C:\usr\local\win64\vc14\bin\TKCDF.dll'
    'C:\usr\local\win64\vc14\bin\TKLCAF.dll'
    'C:\usr\local\win64\vc14\bin\TKShHealing.dll'
    'C:\usr\local\win64\vc14\bin\TKGeomAlgo.dll'
    'C:\usr\local\win64\vc14\bin\TKG2d.dll'
    'C:\usr\local\win64\vc14\bin\TKernel.dll'
    'C:\usr\local\win64\vc14\bin\TKGeomBase.dll'
    'C:\usr\local\win64\vc14\bin\TKTopAlgo.dll'
    'C:\usr\local\win64\vc14\bin\TKPrim.dll'
    'C:\usr\local\win64\vc14\bin\TKMesh.dll'
    'C:\usr\local\win64\vc14\bin\TKMath.dll'
    'C:\usr\local\win64\vc14\bin\TKBRep.dll'
    'C:\usr\local\win64\vc14\bin\TKFillet.dll'
    'C:\usr\local\win64\vc14\bin\TKBO.dll'
    'C:\usr\local\win64\vc14\bin\TKIGES.dll'
    'C:\usr\local\win64\vc14\bin\TKSTEP.dll'
    'C:\usr\local\win64\vc14\bin\TKSTEP209.dll'
    'C:\usr\local\win64\vc14\bin\TKXSBase.dll'
    'C:\usr\local\win64\vc14\bin\TKG3d.dll'

    # libSSH
    'C:\vcpkg\buildtrees\libssh\x64-windows-rel\src\ssh.dll'
    'C:\vcpkg\buildtrees\openssl\x64-windows-rel\libssl-3-x64.dll'
    'C:\vcpkg\buildtrees\openssl\x64-windows-rel\libcrypto-3-x64.dll'

    # libZip
    'C:\usr\local\bin\zip.dll'

    # FFMPEG
    'C:\Program Files\FFmpeg\bin\*.dll'

    # GLEW
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\glew32.dll'

    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\zstd.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\brotlidec.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\brotlicommon.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\icudt74.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\icuuc74.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\icuin74.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\double-conversion.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\pcre2-16.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\libpng16.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\harfbuzz.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\freetype.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\bz2.dll'


)

Foreach ($i in $bins)
{
    cp $i release/bin
}

mkdir release/bin/platforms
cp C:\usr\local\febio\vcpkg_installed\x64-windows\Qt6\plugins\platforms\qwindows.dll release\bin\platforms

mkdir release/bin/styles
cp C:\usr\local\febio\vcpkg_installed\x64-windows\Qt6\plugins\styles\qwindowsvistastyle.dll release\bin\styles
