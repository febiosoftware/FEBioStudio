#!/bin/bash
set -e

. $(dirname $0)/cmake.sh

CURRENT_DIR=$(pwd)

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CURRENT_DIR/febio4-sdk/lib:/usr/local/lib

main() {
    # git clone https://github.com/google/googletest.git ~/googletest
	# pushd ~/googletest
	# cmake -S . -B build
	# pushd build
	# make install -j$(nproc)
	# popd
	# popd	
	
	# git clone https://github.com/pybind/pybind11.git /opt/pybind11
	
	run_cmake
	pushd cmbuild
	make -j $(nproc)

	./bin/fbs-test-suite
	popd
}

main
