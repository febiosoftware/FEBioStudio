call "%VS2022INSTALLDIR%\VC\Auxiliary\Build\vcvars64.bat"

:: Standalone Python module
cd PyLib
git clone --depth 1 https://github.com/febiosoftware/FEBio.git
cmake -L . -B cmbuild ^
  -DFEBioDir=FEBio ^
  -DPython3_INCLUDE_DIR="C:\Program Files\Python313\include" ^
  -DPython3_LIBRARY="C:\Program Files\Python313\libs\python313.lib" ^
  -DPython3_EXECUTABLE="C:\Program Files\Python313\python.exe" ^
  -DUSE_TETGEN=ON

cd cmbuild
msbuild /v:m /P:Configuration=Release  /clp:ErrorsOnly /m:%NUMBER_OF_PROCESSORS% ALL_BUILD.vcxproj
cd ..\..

exit /b %errorlevel%
