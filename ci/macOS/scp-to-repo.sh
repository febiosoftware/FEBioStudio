#! /bin/bash

REMOTE_PATH="update2/FEBioStudio2Dev/macOS/stage"
if [ $# == 1 ] && [ "$1" != "develop" ]; then
    REMOTE_PATH="update2/FEBioStudio2Dev/branches/$1/macOS/stage"
fi

chmod +x cmbuild/bin/FEBioStudio.app/Contents/MacOS/*
scp -r cmbuild/bin/FEBioStudio.app/Contents/MacOS/* repo:~/$REMOTE_PATH/FEBioStudio.app/Contents/MacOS
scp pythonModule/* repo:~/download
