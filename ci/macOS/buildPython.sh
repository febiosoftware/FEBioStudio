#!/bin/bash
set -e

main() {
    pushd PyLib
    git clone --depth 1 https://github.com/febiosoftware/FEBio.git
    cmake -L . -B cmbuild \
        -DFEBioDir=FEBio \
        -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
        -DOMP_INC=/Users/gitRunner/local/x86_64/homebrew/opt/libomp/include \
        -DTETGEN_INC=/Users/gitRunner/local/universal/include \
        -DTETGEN_LIB_DIR=/Users/gitRunner/local/universal/lib \
        -DZLIB_LIBRARY_RELEASE=/Users/gitRunner/local/universal/lib/libz.a

    pushd cmbuild
    make -j $(sysctl -n hw.ncpu)
    popd
    popd
}

main
