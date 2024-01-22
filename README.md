# n2os-smb-client

This is a custom smb/cifs client with libsmb2 statically linked.

## Building

### FreeBSD version

Optional prerequisite: install kerberos if you want Kerberos support

```bash
cd /usr/ports/security/krb5
# (there should be a single Makefile in the folder)
sudo make install clean
```

```bash
git submodule update --init --recursive
cmake . -DCMAKE_BUILD_TYPE=Release
make
strip -s n2os_smb_client
```

A build script is provided in [build.bsd.sh](build.bsd.sh).

### Linux version

```bash
docker run -it -v $(pwd):/n2os-smb-client debian:bullseye-slim /bin/bash
apt update
apt install -y build-essential upx-ucl cmake libssl-dev libkrb5-dev
cd /n2os-smb-client
cmake . -DCMAKE_BUILD_TYPE=Release
make
strip -s n2os_smb_client
upx --best n2os_smb_client
```

A build script is provided in [build.linux.sh](build.linux.sh).

A [Dockerfile](Dockerfile.linux) is also provided.
Build and usage examples are provided in the [Dockerfile](Dockerfile.linux) itself.

### Kerberos support

To build with kerberos support, provided that the dependencies are met, just add
to the cmake command-line `-DBUILD_WITH_KRB5=true`. For the Linux
[Dockerfile](Dockerfile.linux) you can use
the build parameter `WITH_KERBEROS`.

## Usage

Usage is pretty simple. This client right now can just do four simple
operations: ls, del, get and put.

<!-- markdownlint-disable-next-line MD036 -->
**Note that connection password can be passed using env variable N2OS_SMB_PASSWORD**

```text
n2os-smb-client v.0.3.6 - (c) 2020-present Nozomi Networks Inc.

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
