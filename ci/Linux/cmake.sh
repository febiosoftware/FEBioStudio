LIB_DIR="/usr/lib/x86_64-linux-gnu"
run_cmake() {
	cmake -L . -B cmbuild \
		-DQt_Root=$LIB_DIR \
		-DFEBio_SDK=febio4-sdk \
		-DUSE_FFMPEG=ON \
		-DUSE_TEEM=OFF \
		-DUSE_DCMTK=OFF \
		-DUSE_TETGEN=ON \
		-DUSE_MMG=ON \
		-DUSE_SSH=ON \
		-DUSE_SSL=ON \
		-DCAD_FEATURES=ON \
		-DUSE_NETGEN=ON \
		-DUSE_ITK=ON \
		-DBUILD_UPDATER=ON
}
