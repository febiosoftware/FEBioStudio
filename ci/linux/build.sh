#!/bin/bash
set -e

. $(dirname $0)/cmake.sh

main() {
	# TODO: Figure out why it's not generating correctly on first attempt
	run_cmake
	run_cmake
	pushd cmbuild
	make -j $(nproc)
	popd
}

main
