QT_DIR="/opt/Qt/6.9.3/gcc_64"
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
        -DUSE_PYTHON=ON \
		-DBUILD_UPDATER=ON \
        -DFFMPEG_INC=/usr/local/include \
        -DFFMPEG_LIB_DIR=/usr/local/lib \PEG_INC=/usr/local/include \
        -DPython3_ROOT_DIR=/home/ubuntu/.pyenv/versions/3.13.1 \
        -DPython3_EXECUTABLE=/home/ubuntu/.pyenv/versions/3.13.1/bin/python3 \
        -DPython3_INCLUDE_DIR=/home/ubuntu/.pyenv/versions/3.13.1/include/python3.13 \
        -DPython3_LIBRARY=/home/ubuntu/.pyenv/versions/3.13.1/lib/libpython3.13.so 
}
