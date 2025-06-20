#!/bin/sh
set -e

cmake . -DCMAKE_BUILD_TYPE=Release -DKRB_IMPL="${KRB_IMPL}"
make
strip -s n2os_smb_client

UNAME="$(uname -p)"
ARM_SUFFIX=""
if [ "${UNAME}" = "aarch64" ] || [ "${UNAME}" = "arm" ]; then
	ARM_SUFFIX="_arm64"
fi

ldd n2os_smb_client || echo "Binary is statically linked, no dynamic dependencies found."

mkdir -p bin
DEST_FILE="bin/n2os_smb_client.bsd${ARM_SUFFIX}"
cp n2os_smb_client "${DEST_FILE}"