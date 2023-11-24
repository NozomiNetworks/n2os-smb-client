#!/bin/sh
set -e

# git submodule update --init --recursive

cmake . -DCMAKE_BUILD_TYPE=Release
make
strip -s n2os_smb_client

UNAME="$(uname -p)"
ARM_SUFFIX=""
if [ "${UNAME}" = "aarch64" ] || [ "${UNAME}" = "arm" ]
then
  ARM_SUFFIX="_arm64"
fi

cp n2os_smb_client "bin/n2os_smb_client.bsd${ARM_SUFFIX}"
