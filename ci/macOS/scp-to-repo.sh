#! /bin/bash
chmod +x cmbuild/bin/FEBioStudio.app/Contents/MacOS/*
scp -r cmbuild/bin/FEBioStudio.app/Contents/MacOS/FEBioStudio repo:~/update2/FEBioStudio2Dev/macOS/stage/FEBioStudio.app/Contents/MacOS
