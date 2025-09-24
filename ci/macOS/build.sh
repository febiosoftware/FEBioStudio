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

    # Standalone Python module - CAUSINY ISSUES ON MACOS NOT CURRENTLY BUILDING
    # pushd PyLib
    # git clone --depth 1 https://github.com/febiosoftware/FEBio.git
    # cmake -L . -B cmbuild \
    #     -DFEBioDir=FEBio \
    #     -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 
    # pushd cmbuild
    # make -j $(sysctl -n hw.ncpu)
    # popd
    # popd
}

main
