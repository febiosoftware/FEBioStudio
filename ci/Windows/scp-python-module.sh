#! /bin/bash

find pythonModule/ | grep .pyd | xargs -I file scp file repo:~/download