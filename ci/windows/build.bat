call "%ONEAPI_ROOT%\setvars.bat"
set DEPENDENCIES="third-party-deps"
set LIB_INC="%DEPENDENCIES%\include
set LIB_DIR="%DEPENDENCIES%\lib"
set FFMPEG_DIR="%DEPENDENCIES%\FFmpeg"
set FFMPEG_INC="%DEPENDENCIES%\FFmpeg\include"
set QT_ROOT="%DEPENDENCIES%\qt\6.4.1"
set OCCT_PATH="%DEPENDENCIES%\opencascade"
set GLEW_DIR="%DEPENDENCIES%\lib"
set /a "PROC=%NUMBER_OF_PROCESSORS% - 4"
cmake -version

REM TODO: Cmake requires 3 runs to generate correctly
for /l %%a in (1, 1, 3) do (
cmake -L . -B cmbuild ^
  -DFEBio_SDK=febio-sdk ^
  -DQt_Root=%QT_ROOT% ^
  -DDEPENDENCIES=%DEPENDENCIES% ^
  -DSSL_LIB_DIR=%LIB_DIR% ^
  -DSSH_INC=%LIB_DIR%\include ^
  -DTETGEN_LIB_DIR=%LIB_DIR% ^
  -DSSH_LIB_DIR=%LIB_DIR% ^
  -DOCCT_INC=%OCCT_PATH%\inc ^
  -DOCCT_LIB_DIR=%OCCT_PATH%\win64\vc14\lib^
  -DOCCT_PATH=%OCCT_PATH% ^
  -DNETGEN_INC=%LIB_ROOT%\netgen\include ^
  -DNETGEN_LIB_DIR=%LIB_ROOT%\netgen\lib ^
  -DFFMPEG_INC=%FFMPEG_INC% ^
  -DFFMPEG_LIB_DIR=%FFMPEG_DIR%\lib ^
  -DUSE_FFMPEG=ON ^
  -DMODEL_REPO=ON ^
  -DUSE_TETGEN=ON ^
  -DUSE_MMG=ON ^
  -DUSE_SSH=ON ^
  -DUSE_SSL=ON ^
  -DCAD_FEATURES=OFF ^
  -DUSE_NETGEN=ON ^
  -DUSE_ITK=ON
)
cd cmbuild
msbuild /P:Configuration=Release /P:WarningLevel=0 /m:%PROC% ALL_BUILD.vcxproj
cd ..
exit 0
