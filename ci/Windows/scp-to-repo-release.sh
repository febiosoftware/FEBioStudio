#! /bin/bash
chmod +x artifacts/bin/*
scp artifacts/bin/* repo:~/update2/FEBioStudio2/Windows/stage/bin
scp artifacts/doc/* repo:~/update2/FEBioStudio2/Windows/stage/doc
scp artifacts/sdk.zip repo:~/update2/FEBioStudio2/Windows/stage
