#!/bin/bash
set -e
RUN_POST_BUILD=${RUN_POST_BUILD:=true}

. $(dirname $0)/cmake.sh

main() {
	run_cmake
	pushd cmbuild
	make -j $(sysctl -n hw.ncpu)

	# ctest --output-on-failure
	./bin/fbs-test-suite
	popd

	if [ "$RUN_POST_BUILD" = true ]; then
		echo "Running postbuild.sh"
		 ./$(dirname $0)/postBuild.sh
	else
		echo "Skipping postbuild.sh"
	fi
}

main

