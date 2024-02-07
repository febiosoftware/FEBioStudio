#!/bin/bash
set -e

#. "/opt/intel/oneapi/setvars.sh" --force
. $(dirname $0)/cmake.sh

main() {
	# TODO: Figure out why it's not generating correctly on first attempt
	run_cmake
	pushd cmbuild
	make -j $(sysctl -n hw.ncpu)
	popd
}

main
