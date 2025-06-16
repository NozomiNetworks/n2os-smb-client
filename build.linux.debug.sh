#!/bin/sh
set -ex

export CFLAGS="-g -O0"

BUILD_DIR="build"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake -DKRB_IMPL="MIT" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="$CFLAGS" ..
make