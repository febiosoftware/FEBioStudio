#! /bin/bash
chmod +x artifacts/MacOS/*
scp artifacts/MacOS/* repo:/serverRoot/update2/FEBioStudio2/macOS/stage/FEBioStudio.app/Contents/MacOS
scp artifacts/Frameworks/* repo:/serverRoot/update2/FEBioStudio2/macOS/stage/FEBioStudio.app/Contents/Frameworks
scp artifacts/doc/* repo:/serverRoot/update2/FEBioStudio2/macOS/stage/doc
scp artifacts/sdk.zip repo:/serverRoot/update2/FEBioStudio2/macOS/stage
