#!/bin/sh
set -ex

BUILD_DIR="build_macos"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

export CFLAGS="-g -O0"

cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="$CFLAGS" -DKRB_IMPL="${KRB_IMPL}"
make