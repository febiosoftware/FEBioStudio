#!/bin/bash
set -e

. $(dirname $0)/cmake.sh

main() {
    git clone https://github.com/pybind/pybind11.git /opt/pybind11
	# TODO: Figure out why it's not generating correctly on first attempt
	run_cmake
	run_cmake
	pushd cmbuild
	make -j $(nproc)
	popd

    # Standalone Python module
    pushd PyLib
    git clone --depth 1 https://github.com/febiosoftware/FEBio.git
    
    cmake -L . -B cmbuild \
        -DFEBioDir=FEBio \
        -DUSE_TETGEN=OFF \
        -DPython3_ROOT_DIR=/home/ubuntu/.pyenv/versions/3.13.1 \
        -DPython3_EXECUTABLE=/home/ubuntu/.pyenv/versions/3.13.1/bin/python3 \
        -DPython3_INCLUDE_DIR=/home/ubuntu/.pyenv/versions/3.13.1/include/python3.13 \
        -DPython3_LIBRARY=/home/ubuntu/.pyenv/versions/3.13.1/lib/libpython3.13.so 
    
    pushd cmbuild
    make -j $(nproc)
    popd
    popd
}

main
