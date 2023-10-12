HOMEBREW_LIBS="/usr/local/homebrew/lib"
X86_ROOT="/usr/local/x86_64"
X86_LIBS="${X86_ROOT}/lib"
HOMEBREW_LIBS="/usr/local/homebrew/lib"
QT_ROOT="/usr/local/homebrew"
run_cmake() {
	cmake -LA . -B cmbuild \
		-DQt_Root=$QT_ROOT \
		-DGLEW_LIBRARY_RELEASE="${X86_LIBS}/libGLEW.dylib" \
		-DGLEW_INCLUDE_DIRS="${X86_ROOT}/include/GL" \
		-DFEBio_SDK=febio4-sdk \
		-DUSE_FFMPEG=ON \
		-DFFMPEG_LIB_DIR="$HOMEBREW_LIBS" \
		-DUSE_TETGEN=ON \
		-DTETGEN_LIB_DIR="$X86_LIBS" \
		-DUSE_MMG=ON \
		-DMMG_LIB_DIR="$X86_LIBS" \
		-DUSE_SSH=ON \
		-DSSH_LIB_DIR="$HOMEBREW_LIBS" \
		-DSSL_LIB_DIR="$HOMEBREW_LIBS" \
		-DCAD_FEATURES=ON \
		-DUSE_ITK=ON \
		-DUSE_NETGEN=OFF \
		-DSimpleITK_LIBRARIES="$X86_LIBS" \
		-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
		-DCMAKE_OSX_ARCHITECTURES=x86_64
}


