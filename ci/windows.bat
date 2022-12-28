call "%ONEAPI_ROOT%\setvars.bat"
set LIB_ROOT="c:\local"
set LIB_INC="c:\local\include
set LIB_DIR="c:\local\lib"
set FFMPEG_DIR="C:\Program Files\FFmpeg"
set FFMPEG_INC="C:\Program Files\FFmpeg\include"
set QT_ROOT="C:\local\qt\6.4.1"
set OCCT_PATH="C:\local\opencascade"
set /a "PROC=%NUMBER_OF_PROCESSORS% - 4"
cmake -version
cmake -L . -B cmbuild ^
  -DQt_Root=%QT_ROOT% ^
  -DFFMPEG_INC=%FFMPEG_INC% ^
  -DFFMPEG_LIB_DIR=%FFMPEG_DIR%\lib ^
  -DUSE_FFMPEG=ON ^
  -DMODEL_REPO=ON ^
  -DTETGEN_LIB_DIR=%LIB_DIR% ^
  -DUSE_TETGEN=ON ^
  -DOCCT_PATH=%OCCT_PATH% ^
  -DLIB_ROOT=%LIB_ROOT% ^
  -DUSE_MMG=ON ^
  -DSSH_LIB_DIR=%LIB_DIR% ^
  -DUSE_SSH=ON ^
  -DSSL_LIB_DIR=%LIB_DIR% ^
  -DUSE_SSL=ON ^
  -DCAD_FEATURES=OFF ^
  -DOCCT_INC=%OCCT_PATH%\inc ^
  -DOCCT_LIB_DIR=%OCCT_PATH%\win64\vc14\lib^
  -DNETGEN_INC=%LIB_ROOT%\netgen\include ^
  -DNETGEN_LIB_DIR=%LIB_ROOT%\netgen\lib ^
  -DUSE_NETGEN=ON ^
  -DUSE_ITK=ON
cd cmbuild
msbuild /P:Configuration=Release /P:WarningLevel=0 /m:%PROC% ALL_BUILD.vcxproj
cd ..
exit 0
