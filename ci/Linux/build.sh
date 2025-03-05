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
    cmake -L . -B cmbuild -DFEBioDir=FEBio
    pushd cmbuild
    make -j $(nproc)
    popd
    popd
}

main
