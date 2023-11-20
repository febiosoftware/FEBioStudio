#! /bin/bash
scp cmbuild/bin/* repo:~/update2/FEBioStudio2Dev/Linux/stage/bin
ssh repo "chmod +x update2/FEBioStudio2Dev/Linux/stage/bin/FEBioStudio"
