// n2os-smb-client Samba v2/3 custom client
// Copyright (c) 2013-2020, Nozomi Networks Inc. All rights reserved.

#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "smb2.h"
#include "libsmb2.h"

#define MAXCMDSIZE 8
#define MAXPATHSIZE 1024
#define MAXBUF (16 * 1024 * 1024)
uint8_t buf[MAXBUF];
uint32_t pos;

#define VERSION "0.1a"
#define ECMDLINE 4
#define ESMBINIT 5
#define ESMBPARSE 6
#define ESMBCONNECT 7
#define ESMBOPENDIR 8
#define EINVALIDPATH 9
#define ESMBOPEN 10
#define ESMBPREAD 11
#define ELOCALFSERROR 12

int usage(void)
{
    fprintf(stderr, "n2os-smb-client v.%s - (c) 2020 Nozomi Networks Inc.\n"
                    "Usage:\n"
                    "n2os-smb-client ls <smb2-url>\n"
                    "n2os-smb-client get <smb2-url> <local-filename>\n"
                    "n2os-smb-client put <local-filename> <smb2-url>\n"
                    "URL format: smb://[<domain;][<username>@]<host>[:<port>]/<share>/<path>\n\n"
                    "Exit codes:\n"
                    "4 - Cmd line error\n"
                    "5 - SMB init error\n"
                    "6 - SMB parse error\n"
                    "7 - SMB connect error\n"
                    "8 - SMB opendir error\n"
                    "9 - Invalid path error\n"
                    "10 - SMB open error\n"
                    "11 - SMB pread error\n"
                    "12 - Local filesystem error\n", VERSION);

    // I apologize to Dijkstra for this exit
    exit(ECMDLINE);
}

int ls(struct smb2_context *smb2, const char *path)
{
    struct smb2dir *dir;
    struct smb2dirent *ent;
    char *link;
    int result_code = 0;

    dir = smb2_opendir(smb2, path);
    if (dir == NULL) {
        printf("smb2_opendir failed. %s\n", smb2_get_error(smb2));
        result_code = ESMBOPENDIR;
    } else {
        while ((ent = smb2_readdir(smb2, dir))) {
            char *type;
            time_t t;

            t = (time_t)ent->st.smb2_mtime;
            switch (ent->st.smb2_type) {
                case SMB2_TYPE_LINK:
                    type = "LINK";
                    break;
                case SMB2_TYPE_FILE:
                    type = "FILE";
                    break;
                case SMB2_TYPE_DIRECTORY:
                    type = "DIRECTORY";
                    break;
                default:
                    type = "unknown";
                    break;
            }
            printf("%-20s %-9s %15"PRIu64" %s", ent->name, type, ent->st.smb2_size, asctime(localtime(&t)));
            if (ent->st.smb2_type == SMB2_TYPE_LINK) {
                char buffer[256];

                if (path && path[0]) {
                    asprintf(&link, "%s/%s", path, ent->name);
                } else {
                    asprintf(&link, "%s", ent->name);
                }
                smb2_readlink(smb2, link, buffer, BUFSIZ);
                printf("    -> [%s]\n", buffer);
                free(link);
            }
        }

        smb2_closedir(smb2, dir);
    }

    return result_code;
}

int get(struct smb2_context *smb2, const char *path, const char *destfile) {
    struct smb2fh *fh;
    int count, return_code = 0;

    if (path == NULL) {
        printf("Invalid source path\n");
        return_code = EINVALIDPATH;
    } else {
        if (destfile == NULL) destfile = strdup(path);

        fh = smb2_open(smb2, path, O_RDONLY);
        if (fh == NULL) {
            printf("smb2_open failed. %s\n", smb2_get_error(smb2));
            return_code = ESMBOPEN;
        } else {
            while ((count = smb2_pread(smb2, fh, buf, MAXBUF, pos)) != 0) {
                if (count == -EAGAIN) {
                    continue;
                }
                if (count < 0) {
                    fprintf(stderr, "Failed to read file. %s\n",
                            smb2_get_error(smb2));
                    return_code = ESMBPREAD;
                    break;
                }
                write(0, buf, count);
                pos += count;
            };
            smb2_close(smb2, fh);
        }
    }

    return return_code;
}

int put(const char *source_file, struct smb2_context *smb2, const char *destination_file) {
    struct smb2fh *fh;
    int count;
    int fd, result_code = 0;

    if (source_file == NULL) {
        printf("Invalid source path\n");
        result_code = EINVALIDPATH;
    } else {
        if (destination_file == NULL) destination_file = basename(strdup(source_file));
        fd = open(source_file, O_RDONLY);
        if (fd == -1) {
            printf("Failed to open local file %s (%s)\n", source_file,
                   strerror(errno));
            result_code = ELOCALFSERROR;
        } else {
            fh = smb2_open(smb2, destination_file, O_WRONLY | O_CREAT);
            if (fh == NULL) {
                printf("smb2_open failed. %s\n", smb2_get_error(smb2));
                result_code = ESMBOPEN;
            } else {
                while ((count = read(fd, buf, 1024)) > 0) {
                    smb2_write(smb2, fh, buf, count);
                };
                smb2_close(smb2, fh);
            }
            close(fd);
        }
    }
    return result_code;
}

int main(int argc, char *argv[])
{
    struct smb2_context *smb2;
    struct smb2_url *url;
    char command[MAXCMDSIZE];
    char local_filename[MAXPATHSIZE];
    char smb_share[MAXPATHSIZE];
    int result_code = 0;

    if (argc < 2) usage();

    strncpy(command, argv[1], MAXCMDSIZE);
    if (strcmp(command, "ls") == 0) {
        if (argc < 3) usage();
        strncpy(smb_share, argv[2], MAXPATHSIZE);
    } else if (strcmp(command, "put") == 0) {
        if (argc < 4) usage();
        strncpy(smb_share, argv[3], MAXPATHSIZE);
    } else if (strcmp(command, "get") == 0) {
        if (argc < 3) usage();
        strncpy(smb_share, argv[2], MAXPATHSIZE);
    } else {
        fprintf(stderr, "Error: unknown command\n\n");
        usage();
    }

    smb2 = smb2_init_context();
    if (smb2 == NULL) {
        fprintf(stderr, "Failed to init context\n");
        result_code = ESMBINIT;
    } else {
        url = smb2_parse_url(smb2, smb_share);
        if (url == NULL) {
            fprintf(stderr, "Failed to parse url: %s\n", smb2_get_error(smb2));
            result_code = ESMBPARSE;
        } else {
            smb2_set_security_mode(smb2, SMB2_NEGOTIATE_SIGNING_ENABLED);

            if (smb2_connect_share(smb2, url->server, url->share, url->user) < 0) {
                printf("smb2_connect_share failed. %s\n", smb2_get_error(smb2));
                result_code = ESMBCONNECT;
            } else {
                if (strcmp(command, "ls") == 0) {
                    ls(smb2, url->path);
                } else if (strcmp(command, "put") == 0) {
                    strncpy(local_filename, argv[2], 256);
                    if (put(local_filename, smb2, url->path)) {
                        printf("OK: copy completed\n");
                    } else {
                        printf("ERROR: unable to copy\n");
                    }
                } else if (strcmp(command, "get") == 0) {
                    strncpy(local_filename, argv[3], 256);
                    if (get(smb2, url->path, local_filename)) {
                        printf("OK: copy completed\n");
                    } else {
                        printf("ERROR: unable to copy\n");
                    }
                } else {
                    printf("ERROR: unknown command\n\n");
                    usage();
                }
                smb2_disconnect_share(smb2);
            }
            smb2_destroy_url(url);
        }
        smb2_destroy_context(smb2);
    }
    return result_code;
}
