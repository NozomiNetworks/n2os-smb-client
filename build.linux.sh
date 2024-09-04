#!/bin/sh
set -ex

# apt update
# apt install -y git build-essential upx-ucl cmake libssl-dev libkrb5-dev

# git submodule update --init --recursive

BUILD_KBT=""
# shellcheck disable=SC2154
if [ "${WITH_KERBEROS}" = "1" ]; then
	BUILD_KBT="-DBUILD_WITH_KRB5=true"
fi

cmake . -DCMAKE_BUILD_TYPE=Release "${BUILD_KBT}"
make
strip -s n2os_smb_client
upx --best n2os_smb_client

UNAME="$(uname -p)"
ARM_SUFFIX=""
if [ "${UNAME}" = "aarch64" ] || [ "${UNAME}" = "arm" ] || [ -n "${ENV_ARM}" ]; then
	ARM_SUFFIX="_arm64"
fi

cp n2os_smb_client "bin/n2os_smb_client.linux${ARM_SUFFIX}"
