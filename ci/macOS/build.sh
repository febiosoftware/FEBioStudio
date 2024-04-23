#!/bin/bash
set -e
RUN_POST_BUILD=${RUN_POST_BUILD:=true}

. $(dirname $0)/cmake.sh

main() {
	# TODO: Figure out why it's not generating correctly on first attempt
	run_cmake
	pushd cmbuild
	make -j $(sysctl -n hw.ncpu)
	popd

	if [ "$RUN_POST_BUILD" = true ]; then
		echo "Running postbuild.sh"
		 ./$(dirname $0)/postBuild.sh
	else
		echo "Skipping postbuild.sh"
	fi
}

main
