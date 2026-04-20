#! /bin/bash
chmod +x artifacts/bin/*
scp artifacts/bin/* repo:/serverRoot/update2/FEBioStudio2/Linux/stage/bin
scp artifacts/lib/* repo:/serverRoot/update2/FEBioStudio2/Linux/stage/lib
scp artifacts/doc/* repo:/serverRoot/update2/FEBioStudio2/Linux/stage/doc
scp artifacts/sdk.zip repo:/serverRoot/update2/FEBioStudio2/Linux/stage
