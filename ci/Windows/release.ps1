mkdir release
mkdir release\bin

$FEBioRepo = 'C:\Users\Administrator\FEBio\'
$FBSRepo = 'C:\Users\Administrator\FEBioStudio\'

$bins = @(
    # FEBio
    $FEBioRepo + 'cmbuild\bin\Release\febio4.exe'
    $FEBioRepo + 'cmbuild\bin\Release\febio.xml'
    $FEBioRepo + 'cmbuild\bin\Release\*.dll'
    'C:\Program Files (x86)\Intel\oneAPI\compiler\latest\windows\redist\intel64_win\compiler\libiomp5md.dll'

    #FEBio Studio
    $FBSRepo + 'cmbuild\bin\Release\FEBioStudio2.exe'
    $FBSRepo + 'cmbuild\bin\Release\FEBioStudioUpdater.exe'
    $FBSRepo + 'cmbuild\bin\Release\mvUtil.exe'

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

    # SQLite
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\sqlite3.dll'

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

# Create docs
$docs = @(
    $FEBioRepo + 'Documentation\FEBio_EULA_4.pdf'
    $FEBioRepo + 'Documentation\FEBio_Theory_Manual.pdf'
    $FEBioRepo + 'Documentation\FEBio_User_Manual.pdf'
    $FEBioRepo + 'Documentation\FEBio_User_Manual.pdf'
    $FEBioRepo + 'Documentation\ReleaseNotes.txt'
    $FBSRepo + 'Documentation\FEBioStudio_User_Manual.pdf'
    $FBSRepo + 'Documentation\FEBioStudioReleaseNotes.txt'
    $FBSRepo + 'icons/febiostudio.ico'
)

mkdir release/doc

Foreach ($i in $docs)
{
    cp $i release/doc
}

# Create SDK
$sdkLibs = @(
    'FECore'
    'FEBioMech'
    'FEBioMix'
    'FEBioFluid'
    'FEBioRVE'
    'FEBioPlot'
    'FEBioXML'
    'FEBioLib'
)

mkdir release\sdk
mkdir release\sdk\include
mkdir release\sdk\lib\Release
mkdir release\sdk\lib\Debug

Foreach ($i in $sdkLibs)
{
    mkdir release\sdk\include\$i
    cp C:\Users\Administrator\FEBio\$i\*.h release\sdk\include\$i
    cp C:\Users\Administrator\FEBio\$i\*.hpp release\sdk\include\$i

    cp C:\Users\Administrator\FEBio\cmbuild\lib\Release\$i.lib release\sdk\lib\Release
    cp C:\Users\Administrator\FEBio\cmbuild\lib\Debug\$i.lib release\sdk\lib\Debug
}

mkdir release\sdk\bin
mkdir release\sdk\bin\Debug

cp C:\Users\Administrator\FEBio\cmbuild\bin\Debug\febio4.exe release\sdk\bin\Debug
cp C:\Users\Administrator\FEBio\cmbuild\bin\Debug\*.dll release\sdk\bin\Debug


Compress-Archive -Path release\sdk\* -DestinationPath release\sdk.zip