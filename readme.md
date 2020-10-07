# n2os-smb-client
**THIS IS IN SUPER ALPHA STAGE**  

This is a custom make smb/cifs client with libsmb2 statically linked.

We need this to avoid adding all the samba stuff to our n2os firmware.

As default libsmb2 doesn't build under FreeBSD we have forked it https://github.com/NozomiNetworks/libsmb2 and this project uses it as submodules 


## Building
```
git submodule init
cmake .
```

## Syntax

```
n2os-smb-client v.0.1a - (c) 2020 Nozomi Networks Inc.
Usage:
n2os-smb-client ls <smb2-url>
n2os-smb-client get <smb2-url> <local-filename>
n2os-smb-client put <local-filename> <smb2-url>
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
```