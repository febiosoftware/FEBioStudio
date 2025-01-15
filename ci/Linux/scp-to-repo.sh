#! /bin/bash

REMOTE_PATH="update2/FEBioStudio2Dev/Linux/stage"
if [ $# == 1 ] && [ "$1" != "develop" ]; then
    REMOTE_PATH="update2/FEBioStudio2Dev/branches/$1/Linux/stage"
fi

chmod +x cmbuild/bin/*
scp cmbuild/bin/* repo:~/$REMOTE_PATH/bin
