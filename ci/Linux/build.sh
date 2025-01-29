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
}

main
