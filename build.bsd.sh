#!/bin/sh

# git submodule update --init --recursive

cmake . -DCMAKE_BUILD_TYPE=Release
make
strip -s n2os_smb_client

cp n2os_smb_client bin/n2os_smb_client.bsd
