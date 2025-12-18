$FEBIO_REPO = $env:GITHUB_WORKSPACE + '\FEBio\'
$FBS_REPO = $env:GITHUB_WORKSPACE + '\FEBioStudio\'

# Clone and build repos
cd $FEBIO_REPO
.\ci\Windows\build.bat
.\ci\Windows\build.bat -d # build the debug version too
sh --login -i -c ci/Windows/create-sdk.sh

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

    #FEBio Studio
    $FBS_REPO + 'cmbuild\bin\Release\FEBioStudio.exe'
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
    'C:\usr\local\Qt\6.9.3\msvc2022_64\bin\Qt6Core.dll'
    'C:\usr\local\Qt\6.9.3\msvc2022_64\bin\Qt6Widgets.dll'
    'C:\usr\local\Qt\6.9.3\msvc2022_64\bin\Qt6Gui.dll'
    'C:\usr\local\Qt\6.9.3\msvc2022_64\bin\Qt6OpenGLWidgets.dll'
    'C:\usr\local\Qt\6.9.3\msvc2022_64\bin\Qt6OpenGL.dll'
    'C:\usr\local\Qt\6.9.3\msvc2022_64\bin\Qt6Network.dll'

    # ZLIB
    'C:\vcpkg\packages\zlib_x64-windows\bin\zlib1.dll'

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
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\ssh.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\libssl-3-x64.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\libcrypto-3-x64.dll'

    # libZip
    'C:\usr\local\bin\zip.dll'
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\zstd.dll'

    # FFMPEG
    'C:\Program Files\FFmpeg\bin\*.dll'

    # SQLite
    'C:\usr\local\febio\vcpkg_installed\x64-windows\bin\sqlite3.dll'

    # Python
    'C:\Program Files\Python313\python3.dll'
    'C:\Program Files\Python313\python313.dll'
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

# Copy Qt plugins
mkdir release\bin\platforms
cp C:\usr\local\Qt\6.9.3\msvc2022_64\plugins\platforms\qwindows.dll release\bin\platforms

mkdir release\bin\styles
cp C:\usr\local\Qt\6.9.3\msvc2022_64\plugins\styles\qmodernwindowsstyle.dll release\bin\styles

mkdir release\bin\tls
cp C:\usr\local\Qt\6.9.3\msvc2022_64\plugins\tls\*.dll release\bin\tls

mkdir release\bin\imageformats
cp C:\usr\local\Qt\6.9.3\msvc2022_64\plugins\imageformats\*.dll release\bin\imageformats

# Copy python libs
cp -R "C:\Program Files\Python313\DLLs" release\bin\DLLs
cp -R "C:\Program Files\Python313\Lib" release\bin\Lib

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

# Copy sdk to release dir for installer
Copy-Item -Path "$FEBIO_REPO\febio4-sdk" -Destination "release\sdk" -Recurse

# zip sdk
Compress-Archive -Path release\sdk\* -DestinationPath upload\sdk.zip

# Set FEBio and FBS versions in installBuilder.xml
(Get-Content $FBS_REPO\ci\installBuilder.xml).Replace('FEBIO_VER', $env:FEBIO_VER) | Set-Content $FBS_REPO\ci\installBuilder.xml
(Get-Content $FBS_REPO\ci\installBuilder.xml).Replace('FBS_VER', $env:FBS_VER) | Set-Content $FBS_REPO\ci\installBuilder.xml

# Create installer
cd $env:GITHUB_WORKSPACE
$env:FEBIO_REPO = $FEBIO_REPO
$env:FBS_REPO = $FBS_REPO
$env:RELEASE_DIR = $env:GITHUB_WORKSPACE + '\release'
builder-cli.exe build $FBS_REPO\ci\installBuilder.xml windows --license $env:GITHUB_WORKSPACE\license.xml

mkdir upload\installer
cp C:\Users\Administrator\Documents\InstallBuilder\output\* upload\installer

# make sdk visible to plugins
"FEBIO_SDK=$env:GITHUB_WORKSPACE/FEBio/febio4-sdk" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
