#!/bin/sh
set -e

# git submodule update --init --recursive

# FreeBSD-specific options for static linking
export LDFLAGS="-static"
export CFLAGS="-static"

# Use FreeBSD-specific CMake configuration
cmake . \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_EXE_LINKER_FLAGS="-static" \
  -DCMAKE_FIND_LIBRARY_SUFFIXES=".a"

make
strip -s n2os_smb_client

UNAME="$(uname -p)"
ARM_SUFFIX=""
if [ "${UNAME}" = "aarch64" ] || [ "${UNAME}" = "arm" ]; then
	ARM_SUFFIX="_arm64"
fi

ldd n2os_smb_client || echo "Binary is statically linked, no dynamic dependencies found."

DEST_FILE="bin/n2os_smb_client.bsd${ARM_SUFFIX}"
cp n2os_smb_client "${DEST_FILE}"