#!/bin/bash
set -e

. $(dirname $0)/cmake.sh

CURRENT_DIR=$(pwd)

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CURRENT_DIR/febio4-sdk/lib:/usr/local/lib

main() {
	run_cmake
	pushd cmbuild
	make -j $(nproc)

	./bin/fbs-test-suite
	popd
}

main
