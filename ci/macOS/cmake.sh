QT_ROOT="$HOME/QtNew/6.9.3/macos"
run_cmake() {
	cmake -L . -B cmbuild \
		-DQt_Root=$QT_ROOT \
		-DCMAKE_PREFIX_PATH=febio4-sdk
		-DUSE_FFMPEG=ON \
		-DUSE_TETGEN=ON \
		-DUSE_MMG=ON \
		-DUSE_SSH=ON \
		-DCAD_FEATURES=ON \
		-DUSE_ITK=ON \
        -DUSE_PYTHON=ON \
		-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
		-DCMAKE_OSX_ARCHITECTURES=x86_64 \
		-DBUILD_UPDATER=ON \
		-DHOMEBREW=ON \
        -DUSE_MKL_OMP=ON \
        -DMKL_OMP=/opt/intel/oneapi/compiler/latest/mac/compiler/lib/libiomp5.dylib \
        -DOMP_INC=/Users/gitRunner/local/x86_64/homebrew/opt/libomp/include \
        -DFFMPEG_INC=/Users/gitRunner/local/x86_64/include \
        -DFFMPEG_LIB_DIR=/Users/gitRunner/local/x86_64/lib
}


