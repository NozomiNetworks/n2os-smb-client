#!/bin/sh
set -ex

cmake . -DCMAKE_BUILD_TYPE=Release -DKRB_IMPL="${KRB_IMPL}"
make
strip -s n2os_smb_client
upx --best n2os_smb_client

UNAME="$(uname -p)"
ARM_SUFFIX=""
if [ "${UNAME}" = "aarch64" ] || [ "${UNAME}" = "arm" ] || [ -n "${ENV_ARM}" ]; then
	ARM_SUFFIX="_arm64"
fi

ldd n2os_smb_client || echo "Binary is statically linked, no dynamic dependencies found."

mkdir -p bin
DEST_FILE="bin/n2os_smb_client.linux${ARM_SUFFIX}"
cp n2os_smb_client "${DEST_FILE}"