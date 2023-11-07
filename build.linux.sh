#!/bin/sh
set -ex

# apt update
# apt install -y git build-essential upx-ucl cmake libssl-dev libkrb5-dev

# git submodule update --init --recursive
cmake . -DCMAKE_BUILD_TYPE=Release
make
strip -s n2os_smb_client
upx --best n2os_smb_client

cp n2os_smb_client bin/n2os_smb_client.linux
