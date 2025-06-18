#!/bin/sh
set -ex

export CFLAGS="-g -O0"

BUILD_DIR="build_linux"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="$CFLAGS"  -DKRB_IMPL="${KRB_IMPL}" ..
make