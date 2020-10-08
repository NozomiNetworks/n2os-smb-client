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
#include <sys/stat.h>

#include "deps/libsmb2/include/smb2/smb2.h"
#include "deps/libsmb2/include/smb2/libsmb2.h"
#include "deps/json-c/json.h"

#define MAXCMDSIZE 8
#define MAXPATHSIZE 1024
#define MAXBUF (1024 * 64)

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
#define EWRITEERROR 13
#define ESMBWRITE 14

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
                    "12 - Local filesystem error\n"
                    "13 - Local filesystem write error\n"
                    "14 - SMB write error\n"
                    , VERSION);

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
        struct json_object *json_listing, *json_entry;
        json_listing = json_object_new_array();

        while ((ent = smb2_readdir(smb2, dir))) {
            json_entry = json_object_new_object();
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
            json_object_object_add(json_entry, "name", json_object_new_string(ent->name));
            json_object_object_add(json_entry, "type", json_object_new_string(type));
            json_object_object_add(json_entry, "size", json_object_new_int64(ent->st.smb2_size));
            json_object_object_add(json_entry, "time", json_object_new_int64(t));
            json_object_array_add(json_listing, json_entry);
        }
        printf("%s", json_object_to_json_string_ext(json_listing, JSON_C_TO_STRING_PRETTY));
        json_object_put(json_listing); // Delete the json object
        smb2_closedir(smb2, dir);
    }

    return result_code;
}

int get(struct smb2_context *smb2, const char *source_file, const char *destination_file) {
    struct smb2fh *fh;
    int count, result_code = 0, nwritten;
    int fd;
    uint8_t buf[MAXBUF];
    uint32_t pos = 0;

    if (source_file == NULL) {
        printf("Invalid source path\n");
        result_code = EINVALIDPATH;
    } else {
        if (destination_file == NULL) destination_file = strdup(source_file);

        fh = smb2_open(smb2, source_file, O_RDONLY);
        if (fh == NULL) {
            fprintf(stderr, "smb2_open failed. %s\n", smb2_get_error(smb2));
            result_code = ESMBOPEN;
        } else {
            fd = creat(destination_file, S_IRUSR | S_IWUSR);
            if (fd == -1) {
                fprintf(stderr, "Failed to create local file %s (%s)\n", destination_file, strerror(errno));
                result_code = ELOCALFSERROR;
            } else {
                pos = 0;
                while ((count = smb2_pread(smb2, fh, buf, MAXBUF, pos)) != 0) {
                    if (count == -EAGAIN) {
                        continue;
                    }
                    if (count < 0) {
                        fprintf(stderr, "Failed to read file. %s\n", smb2_get_error(smb2));
                        result_code = ESMBPREAD;
                        break;
                    }
                    nwritten = write(fd, buf, count);
                    if (nwritten < 0) {
                        fprintf(stderr, "Failed writing to file. Error no: %i\n", errno);
                        result_code = EWRITEERROR;
                        break;
                    }
                    pos += count;
                };
                close(fd);
            }
            smb2_close(smb2, fh);
        }
    }

    return result_code;
}

int put(const char *source_file, struct smb2_context *smb2, const char *destination_file) {
    struct smb2fh *fh;
    int count;
    int fd, result_code = 0;
    uint8_t buf[MAXBUF];

    if (source_file == NULL) {
        fprintf(stderr, "Invalid source path\n");
        result_code = EINVALIDPATH;
    } else {
        if (destination_file == NULL) destination_file = basename(strdup(source_file));
        fd = open(source_file, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "Failed to open local file %s (%s)\n", source_file, strerror(errno));
            result_code = ELOCALFSERROR;
        } else {
            fh = smb2_open(smb2, destination_file, O_WRONLY | O_CREAT);
            if (fh == NULL) {
                fprintf(stderr, "smb2_open failed. %s\n", smb2_get_error(smb2));
                result_code = ESMBOPEN;
            } else {
                while ((count = read(fd, buf, MAXBUF)) > 0) {
                    int write_status = smb2_write(smb2, fh, buf, count);
                    if (write_status < 0) {
                        fprintf(stderr, "smb2_write failed. Error code %i\n", write_status);
                        result_code = ESMBWRITE;
                        break;
                    }
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
                fprintf(stderr, "smb2_connect_share failed. %s\n", smb2_get_error(smb2));
                result_code = ESMBCONNECT;
            } else {
                if (strcmp(command, "ls") == 0) {
                    ls(smb2, url->path);
                } else if (strcmp(command, "put") == 0) {
                    strncpy(local_filename, argv[2], 256);
                    result_code = put(local_filename, smb2, url->path);
                    if (result_code == 0) {
                        printf("OK: copy completed\n");
                    } else {
                        printf("ERROR: unable to copy\n");
                    }
                } else if (strcmp(command, "get") == 0) {
                    strncpy(local_filename, argv[3], 256);
                    result_code = get(smb2, url->path, local_filename);
                    if (result_code == 0) {
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
