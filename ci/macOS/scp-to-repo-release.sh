#! /bin/bash
chmod +x artifacts/MacOS/*
scp artifacts/MacOS/* repo:~/update2/FEBioStudio2/macOS/stage/FEBioStudio.app/Contents/MacOS
scp artifacts/Frameworks/* repo:~/update2/FEBioStudio2/macOS/stage/FEBioStudio.app/Contents/Frameworks
scp artifacts/doc/* repo:~/update2/FEBioStudio2/macOS/stage/doc
scp artifacts/sdk.zip repo:~/update2/FEBioStudio2/macOS/stage
