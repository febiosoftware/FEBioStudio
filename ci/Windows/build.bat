call "%VS2022INSTALLDIR%\VC\Auxiliary\Build\vcvars64.bat"

SET "FBS_DIR=%CD%"

git clone https://github.com/google/googletest.git %USERPROFILE%\googletest
cd %USERPROFILE%\googletest
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=C:\usr\local
cd build
msbuild /v:m /P:Configuration=Release  /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS% INSTALL.vcxproj
if errorlevel 1 exit /b %errorlevel%

cd %FBS_DIR%

set Qt_Root="c:/usr/local/Qt/6.9.3/msvc2022_64"
cmake -L . -B cmbuild ^
  -DQt_Root=%Qt_Root% ^
  -DCMAKE_PREFIX_PATH=febio4-sdk ^
  -DWINDEPLOYQT_EXECUTABLE="%Qt_Root%\bin\windeployqt.exe" ^
  -DUSE_FFMPEG=ON ^
  -DUSE_TETGEN=ON ^
  -DUSE_MMG=ON ^
  -DUSE_SSH=ON ^
  -DUSE_SSL=ON ^
  -DCAD_FEATURES=ON ^
  -DUSE_NETGEN=ON ^
  -DMODEL_REPO=ON ^
  -DUSE_ITK=ON ^
  -DBUILD_UPDATER=ON ^
  -DBUILD_TESTS=ON ^
  -DUSE_PYTHON=ON ^
  -DMMG_LIB_DIR="C:\usr\local\lib" ^
  -DNETGEN_LIB_DIR="C:\usr\local\lib" ^
  -DFFMPEG_LIB_DIR="C:\Program Files\FFmpeg\lib" ^
  -DSSH_LIB_DIR="C:\vcpkg\packages\libssh_x64-windows\lib" ^
  -DPython3_INCLUDE_DIR="C:\Program Files\Python313\include" ^
  -DPython3_LIBRARY="C:\Program Files\Python313\libs\python313.lib" ^
  -DPython3_EXECUTABLE="C:\Program Files\Python313\python.exe" 

cd cmbuild
msbuild /v:m /P:Configuration=Release  /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS% ALL_BUILD.vcxproj
if errorlevel 1 exit /b %errorlevel%

SET PATH=%PATH%;C:\usr\local\win64\vc14\bin;C:\usr\local\bin;C:\usr\local\febio\vcpkg_installed\x64-windows\bin;%FBS_DIR%\febio4-sdk\bin\Release

.\bin\Release\fbs-test-suite.exe

exit /b %errorlevel%
