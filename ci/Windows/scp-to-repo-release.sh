#! /bin/bash
chmod +x artifacts/bin/*
scp artifacts/bin/* repo:/serverRoot/update2/FEBioStudio2/Windows/stage/bin
scp artifacts/doc/* repo:/serverRoot/update2/FEBioStudio2/Windows/stage/doc
scp artifacts/sdk.zip repo:/serverRoot/update2/FEBioStudio2/Windows/stage
