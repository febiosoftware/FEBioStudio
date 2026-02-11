#! /bin/bash

find pythonModule/ | grep .so | xargs -I file scp file repo:~/download
