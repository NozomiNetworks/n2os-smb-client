# n2os-smb-client

This is a custom smb/cifs client with libsmb2 statically linked.

## Building

### FreeBSD version

Optional prerequisite: install kerberos if you want Kerberos support
```
cd /usr/ports/security/krb5
# (there should be a single Makefile in the folder)
sudo make install clean
```

```
git submodule update --init --recursive
cmake . -DCMAKE_BUILD_TYPE=Release
make
strip -s n2os_smb_client
```

### Linux version

```
docker run -it -v $(pwd):/n2os-smb-client debian:buster-slim /bin/bash
apt update
apt install -y build-essential upx cmake libssl-dev libkrb5-dev
cd /n2os-smb-client
cmake . -DCMAKE_BUILD_TYPE=Release
make
strip -s n2os_smb_client
upx --best n2os_smb_client
```

A [Dockerfile](Dockerfile.linux) is also provided to simplify the build.
You can use it as:

```
docker build -t "smblinux:latest" -f Dockerfile.linux .
docker run --rm -v "$PWD/bin":/usr/src/n2os-smb-client/bin:Z smblinux:latest
```

### Kerberos support

To build with kerberos support, provided that the dependencies are met, just add
to the cmake commandline `-DBUILD_WITH_KRB5=true`.

## Usage

Usage is pretty simple. This client right now can just do three simple
operations: ls, get and put.

**Note that connection password can be passed using env variable N2OS_SMB_PASSWORD**

```
n2os-smb-client v.0.2 - (c) 2020 Nozomi Networks Inc.

Usage:
n2os-smb-client ls <smb2-url>
n2os-smb-client del <smb2-url>
n2os-smb-client get <smb2-url> [<local-filename>]
n2os-smb-client put <local-filename> <smb2-url>

Password can be passed using the N2OS_SMB_PASSWORD environment variable.
URL format: smb://[<domain;][<username>@]<host>[:<port>]/<share>/<path>

Exit codes:
4 - Cmd line error
5 - SMB init error
6 - SMB parse error
7 - SMB connect error
8 - SMB opendir error
9 - Invalid path error
10 - SMB open error
11 - SMB pread error
12 - Local filesystem error
13 - Local filesystem write error
14 - SMB write error
15 - SMB unlink error
```
