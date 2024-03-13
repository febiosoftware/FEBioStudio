$FEBIO_REPO = $env:GITHUB_WORKSPACE + '\FEBio\'
$CHEM_REPO = $env:GITHUB_WORKSPACE + '\FEBioChem\'
$HEAT_REPO = $env:GITHUB_WORKSPACE + '\FEBioHeat\'
$FBS_REPO = $env:GITHUB_WORKSPACE + '\FEBioStudio\'

# Clone and build repos
cd $FEBIO_REPO
.\ci\Windows\build.bat
.\ci\Windows\build.bat -d # build the debug version too
sh --login -i -c ci/Windows/create-sdk-wrapped.sh

# FEBioChem
New-Item -Path $CHEM_REPO\febio4-sdk -ItemType SymbolicLink -Value $FEBIO_REPO\febio4-sdk
cd $CHEM_REPO
.\ci\Windows\build.bat

# FEBioHeat
New-Item -Path $HEAT_REPO\febio4-sdk -ItemType SymbolicLink -Value $FEBIO_REPO\febio4-sdk
cd $HEAT_REPO
.\ci\Windows\build.bat

# FEBioStudio
New-Item -Path $FBS_REPO\febio4-sdk -ItemType SymbolicLink -Value $FEBIO_REPO\febio4-sdk
cd $FBS_REPO
.\ci\Windows\build.bat


cd $env:GITHUB_WORKSPACE
mkdir release
mkdir release\bin

mkdir upload
mkdir upload\bin
mkdir upload\doc
mkdir upload\updater

$febioBins = @(
    # FEBio
    $FEBIO_REPO + 'cmbuild\bin\Release\febio4.exe'
    $FEBIO_REPO + 'cmbuild\bin\Release\*.dll'

    $FBS_REPO + 'ci\Windows\febio.xml'

    # Plugins
    $CHEM_REPO + 'cmbuild\Release\FEBioChem.dll'
    $HEAT_REPO + 'cmbuild\Release\FEBioHeat.dll'

    #FEBio Studio
    $FBS_REPO + 'cmbuild\bin\Release\FEBioStudio2.exe'
)

$updater = @(
    #Updater
    $FBS_REPO + 'cmbuild\bin\Release\FEBioStudioUpdater.exe'
    $FBS_REPO + 'cmbuild\bin\Release\mvUtil.exe'
)

$bins = @(
    # OMP
    'C:\Program Files (x86)\Intel\oneAPI\compiler\latest\windows\redist\intel64_win\compiler\libiomp5md.dll'

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

Foreach ($i in $febioBins)
{
    cp $i release/bin
    cp $i upload/bin
}

Foreach ($i in $updater)
{
    cp $i release/bin
    cp $i upload/updater
}

Foreach ($i in $bins)
{
    cp $i release/bin
}

mkdir release\bin\platforms
cp C:\usr\local\febio\vcpkg_installed\x64-windows\Qt6\plugins\platforms\qwindows.dll release\bin\platforms

mkdir release\bin\styles
cp C:\usr\local\febio\vcpkg_installed\x64-windows\Qt6\plugins\styles\qwindowsvistastyle.dll release\bin\styles

mkdir release\bin\tls
cp C:\usr\local\febio\vcpkg_installed\x64-windows\Qt6\plugins\tls\*.dll release\bin\tls

mkdir release\bin\imageformats
cp C:\usr\local\febio\vcpkg_installed\x64-windows\Qt6\plugins\imageformats\*.dll release\bin\imageformats

# Create docs
$docs = @(
    $FEBIO_REPO + 'Documentation\FEBio_EULA_4.pdf'
    $FEBIO_REPO + 'Documentation\FEBio_Theory_Manual.pdf'
    $FEBIO_REPO + 'Documentation\FEBio_User_Manual.pdf'
    $FEBIO_REPO + 'Documentation\FEBio_User_Manual.pdf'
    $FEBIO_REPO + 'Documentation\ReleaseNotes.txt'
    $FBS_REPO + 'Documentation\FEBioStudio_User_Manual.pdf'
    $FBS_REPO + 'Documentation\FEBioStudioReleaseNotes.txt'
    $FBS_REPO + 'icons/febiostudio.ico'
)

mkdir release/doc

Foreach ($i in $docs)
{
    cp $i release/doc
    cp $i upload/doc
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
    cp $FEBIO_REPO\$i\*.h release\sdk\include\$i
    cp $FEBIO_REPO\$i\*.hpp release\sdk\include\$i

    cp $FEBIO_REPO\cmbuild\lib\Release\$i.lib release\sdk\lib\Release
    cp $FEBIO_REPO\cmbuild\lib\Debug\$i.lib release\sdk\lib\Debug
}

mkdir release\sdk\bin
mkdir release\sdk\bin\Debug

cp $FEBIO_REPO\cmbuild\bin\Debug\febio4.exe release\sdk\bin\Debug
cp $FEBIO_REPO\cmbuild\bin\Debug\*.dll release\sdk\bin\Debug

# zip sdk
Compress-Archive -Path release\sdk\* -DestinationPath upload\sdk.zip

# Create installer
cd $env:GITHUB_WORKSPACE
$env:FEBIO_REPO = $FEBIO_REPO
$env:FBS_REPO = $FBS_REPO
$env:RELEASE_DIR = $env:GITHUB_WORKSPACE + '\release'
builder-cli.exe build $FBS_REPO\ci\installBuilder.xml windows --license $env:GITHUB_WORKSPACE\license.xml

mkdir upload\installer
cp C:\Users\Administrator\Documents\InstallBuilder\output\* upload\installer