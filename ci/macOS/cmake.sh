QT_ROOT="$HOME/local/x86_64/QtNew/6.7.3/macos"
run_cmake() {
	cmake -L . -B cmbuild \
		-DQt_Root=$QT_ROOT \
		-DFEBio_SDK=febio4-sdk \
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
        -DOMP_INC=/Users/gitRunner/local/x86_64/homebrew/opt/libomp/include
}


