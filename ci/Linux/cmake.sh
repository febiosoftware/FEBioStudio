QT_DIR="/opt/Qt/6.7.3/gcc_64"
run_cmake() {
	cmake -L . -B cmbuild \
		-DQt_Root=$QT_DIR \
		-DFEBio_SDK=febio4-sdk \
		-DUSE_FFMPEG=ON \
		-DUSE_TETGEN=ON \
		-DUSE_MMG=ON \
		-DUSE_SSH=ON \
		-DUSE_SSL=ON \
		-DCAD_FEATURES=ON \
		-DUSE_NETGEN=ON \
		-DUSE_ITK=ON \
        -DUSE_LEVMAR=ON \
		-DBUILD_UPDATER=ON
}
