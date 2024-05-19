#! /bin/bash
chmod +x artifacts/bin/*
scp artifacts/bin/* repo:~/update2/FEBioStudio2/Linux/stage/bin
scp artifacts/lib/* repo:~/update2/FEBioStudio2/Linux/stage/lib
scp artifacts/doc/* repo:~/update2/FEBioStudio2/Linux/stage/doc
scp artifacts/sdk.zip repo:~/update2/FEBioStudio2/Linux/stage
