#!/bin/bash
set -e

. $(dirname $0)/cmake.sh

export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/shims:$PYENV_ROOT/bin:$PATH"

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
    cmake -L . -B cmbuild -DFEBioDir=FEBio -DUSE_TETGEN=OFF
    pushd cmbuild
    make -j $(nproc)
    popd
    popd
}

main
