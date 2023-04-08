#! /bin/bash
set -o errexit
set -o verbose

BUCKET=${BUCKET:-febio-packages}
PACKAGE_NAME=${PACKAGE_NAME-febio}
SDK_S3_URI="s3://$BUCKET/$PACKAGE_NAME/$FEBIO_SDK_BRANCH/$OS/$PACKAGE"
aws s3 cp $SDK_S3_URI .
tar xzf $PACKAGE
