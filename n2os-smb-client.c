// n2os-smb-client SMB v2/3 custom client
// Copyright (c) 2013-present, Nozomi Networks Inc. All rights reserved.

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libgen.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "deps/json-c/json.h"
#include "deps/libsmb2/include/smb2/smb2.h"
#include "deps/libsmb2/include/smb2/libsmb2.h"

#define DEFAULT_TIMEOUT 60
#define MAXPATHSIZE 1024
#define MAXFILENAMELEN 256
#define MAXBUF (1024 * 64)
#define ENV_PASSWORD_VAR "N2OS_SMB_PASSWORD"

#define VERSION "0.3.6"
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
#define EUNLINKERROR 15

#define IS_VALID_FILE(s) ((s) && *(s))

int usage(void) {
  fprintf(stderr,
          "n2os-smb-client v.%s - (c) 2020-present Nozomi Networks Inc.\n\n"
          "Usage:\n"
          "n2os-smb-client ls <smb2-url>\n"
          "n2os-smb-client del <smb2-url>\n"
          "n2os-smb-client get <smb2-url> [<local-filename>]\n"
          "n2os-smb-client put <local-filename> <smb2-url>\n\n"
          "Password can be passed using the %s environment variable.\n"
          "URL format: "
          "smb://[<domain;][<username>@]<host>[:<port>]/<share>/<path>\n\n"
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
          "15 - SMB unlink error\n",
          VERSION, ENV_PASSWORD_VAR);

  return ECMDLINE;
}

typedef void (*destroy_fd_t)(int);
typedef void (*destroy_t)(void *);
typedef void (*destroy_smb_t)(struct smb2_context *smb, void *);

void ignore_fd(int fd) {}
void close_fd(int fd) { close(fd); }

void ignore_smb(struct smb2_context *smb, void *ptr) {}
void close_smb_dir(struct smb2_context *smb, void *dir) {
  smb2_closedir(smb, (struct smb2dir *)dir);
}
void close_smb_file(struct smb2_context *smb, void *fh) {
  smb2_close(smb, (struct smb2fh *)fh);
}

void ignore_ptr(void *ptr) {}
void destroy_url(void *url) { smb2_destroy_url((struct smb2_url *)url); }
void destroy_context(void *smb2) {
  smb2_destroy_context((struct smb2_context *)smb2);
}
void disconnect_share(void *smb2) {
  smb2_disconnect_share((struct smb2_context *)smb2);
}
void free_memory(void *ptr) { free(ptr); }

int ls(struct smb2_context *smb2, const char *path) {
  struct smb2dir *dir;
  struct smb2dirent *ent;
  char const *link;

  dir = smb2_opendir(smb2, path);
  if (dir == NULL) {
    printf("smb2_opendir failed. %s\n", smb2_get_error(smb2));
    return ESMBOPENDIR;
  }

  struct json_object *json_listing;
  struct json_object *json_entry;
  json_listing = json_object_new_array();

  while ((ent = smb2_readdir(smb2, dir))) {
    json_entry = json_object_new_object();
    char const *type;
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
    json_object_object_add(json_entry, "name",
                           json_object_new_string(ent->name));
    json_object_object_add(json_entry, "type", json_object_new_string(type));
    json_object_object_add(json_entry, "size",
                           json_object_new_int64(ent->st.smb2_size));
    json_object_object_add(json_entry, "time", json_object_new_int64(t));
    json_object_object_add(json_entry, "nlink",
                           json_object_new_int(ent->st.smb2_size));
    json_object_object_add(json_entry, "ino",
                           json_object_new_int64(ent->st.smb2_ino));
    json_object_object_add(json_entry, "atime",
                           json_object_new_int64(ent->st.smb2_atime));
    json_object_object_add(json_entry, "atime_nsec",
                           json_object_new_int64(ent->st.smb2_atime_nsec));
    json_object_object_add(json_entry, "mtime",
                           json_object_new_int64(ent->st.smb2_mtime));
    json_object_object_add(json_entry, "mtime_nsed",
                           json_object_new_int64(ent->st.smb2_mtime_nsec));
    json_object_object_add(json_entry, "ctime",
                           json_object_new_int64(ent->st.smb2_ctime));
    json_object_object_add(json_entry, "ctime_nsec",
                           json_object_new_int64(ent->st.smb2_ctime_nsec));
    json_object_object_add(json_entry, "btime",
                           json_object_new_int64(ent->st.smb2_btime));
    json_object_object_add(json_entry, "btime_nsec",
                           json_object_new_int64(ent->st.smb2_btime_nsec));

    json_object_array_add(json_listing, json_entry);
  }
  printf("%s",
         json_object_to_json_string_ext(json_listing, JSON_C_TO_STRING_PRETTY));
  json_object_put(json_listing); // Delete the json object
  smb2_closedir(smb2, dir);

  return 0;
}

int get(struct smb2_context *smb2, const char *source_file,
        const char *destination_file) {
  struct smb2fh *fh;
  int count;
  int result_code = 0;
  int nwritten;
  int fd;
  uint8_t buf[MAXBUF];
  unsigned int pos = 0;
  unsigned int max_read = MAXBUF;

  destroy_fd_t dispose_of_fd = ignore_fd;
  destroy_smb_t dispose_of_smb = ignore_smb;
  destroy_t dispose_of_filename = ignore_ptr;

  if (!IS_VALID_FILE(source_file)) {
    printf("Invalid source path\n");
    return EINVALIDPATH;
  }

  if (!IS_VALID_FILE(destination_file)) {
    destination_file = basename(strdup(source_file));
    dispose_of_filename = free_memory;
  }

  fh = smb2_open(smb2, source_file, O_RDONLY);
  if (fh == NULL) {
    fprintf(stderr, "smb2_open failed. %s\n", smb2_get_error(smb2));
    return ESMBOPEN;
  } else {
    dispose_of_smb = close_smb_file;
  }

  fd = creat(destination_file, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    fprintf(stderr, "Failed to create local file %s (%s)\n", destination_file,
            strerror(errno));
    result_code = ELOCALFSERROR;
    goto error;
  } else {
    dispose_of_fd = close_fd;
  }

  max_read = smb2_get_max_read_size(smb2);
  if (max_read == 0 || max_read > MAXBUF) {
    max_read = MAXBUF;
  }

  pos = 0;
  while ((count = smb2_pread(smb2, fh, buf, max_read, pos)) != 0) {
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
      fprintf(stderr, "Failed to write file %s. Error code %i (%s)\n",
              destination_file, errno, strerror(errno));
      result_code = EWRITEERROR;
      break;
    }
    pos += count;
  }

error:
  dispose_of_fd(fd);
  dispose_of_smb(smb2, fh);
  dispose_of_filename((char *)destination_file);

  return result_code;
}

int del(struct smb2_context *smb2, const char *filename) {
  int result_code = 0;
  int unlink_error = 0;

  if ((unlink_error = smb2_unlink(smb2, filename)) != 0) {
    fprintf(stderr, "Unlink error %i\n", unlink_error);
    result_code = EUNLINKERROR;
  }
  return result_code;
}

int put(const char *source_file, struct smb2_context *smb2,
        const char *destination_file) {
  struct smb2fh *fh;
  unsigned int count;
  int fd;
  int result_code = 0;
  uint8_t buf[MAXBUF];
  unsigned int max_write = MAXBUF;

  destroy_fd_t dispose_of_fd = ignore_fd;
  destroy_smb_t dispose_of_smb = ignore_smb;
  destroy_t dispose_of_filename = ignore_ptr;

  if (!IS_VALID_FILE(source_file)) {
    fprintf(stderr, "Invalid source path\n");
    return EINVALIDPATH;
  }

  if (!IS_VALID_FILE(destination_file)) {
    destination_file = basename(strdup(source_file));
    dispose_of_filename = free_memory;
  }

  fd = open(source_file, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Failed to open local file %s. Error code %i (%s)\n",
            source_file, errno, strerror(errno));
    return ELOCALFSERROR;
  } else {
    dispose_of_fd = close_fd;
  }

  fh = smb2_open(smb2, destination_file, O_WRONLY | O_CREAT);
  if (fh == NULL) {
    fprintf(stderr, "smb2_open failed. Error code %s\n", smb2_get_error(smb2));
    result_code = ESMBOPEN;
    goto error;
  } else {
    dispose_of_smb = close_smb_file;
  }

  max_write = smb2_get_max_write_size(smb2);
  if (max_write == 0) {
    max_write = MAXBUF; // try to ignore and continue
  }

  while ((count = read(fd, buf, MAXBUF)) > 0) {
    const uint8_t *buf_start = buf;
    do {
      unsigned int delta = (count > max_write) ? max_write : count;

      int write_status = smb2_write(smb2, fh, buf_start, delta);
      if (write_status < 0) {
        fprintf(stderr, "smb2_write failed. Error code %i\n", write_status);
        result_code = ESMBWRITE;
        goto error;
      }

      count -= delta;
      buf_start += delta;
    } while (count > 0);
  }

error:
  dispose_of_smb(smb2, fh);
  dispose_of_fd(fd);
  dispose_of_filename((char *)destination_file);

  return result_code;
}

void set_password_from_env(struct smb2_context *smb2) {
  char const *name = NULL;

  name = getenv(ENV_PASSWORD_VAR);
  if (name != NULL) {
    smb2_set_password(smb2, name);
  }
}

enum { CMD_LS = 1, CMD_GET, CMD_PUT, CMD_DEL };

int main(int argc, char *argv[]) {
  struct smb2_context *smb2;
  struct smb2_url *url;
  char *command;
  char local_filename[MAXPATHSIZE + MAXFILENAMELEN + 1] = {0};
  char smb_share[MAXPATHSIZE + 1] = {0};
  int result_code = 0;
  int what = 0;

  destroy_t dispose_of_url = ignore_ptr;
  destroy_t dispose_of_connection = ignore_ptr;
  destroy_t dispose_of_context = ignore_ptr;

  if (argc < 2)
    return usage();

  command = argv[1];
  if (strcmp(command, "ls") == 0) {
    what = CMD_LS;
    if (argc < 3)
      return usage();

    strncpy(smb_share, argv[2], MAXPATHSIZE);
  } else if (strcmp(command, "put") == 0) {
    what = CMD_PUT;
    if (argc < 4)
      return usage();

    strncpy(local_filename, argv[2], MAXFILENAMELEN);
    strncpy(smb_share, argv[3], MAXPATHSIZE);
  } else if (strcmp(command, "get") == 0) {
    what = CMD_GET;
    if (argc < 3)
      return usage();

    strncpy(smb_share, argv[2], MAXPATHSIZE);
    if (argc > 3)
      strncpy(local_filename, argv[3], MAXFILENAMELEN);
  } else if (strcmp(command, "del") == 0) {
    what = CMD_DEL;
    if (argc < 3)
      return usage();

    strncpy(smb_share, argv[2], MAXPATHSIZE);
  } else {
    fprintf(stderr, "Error: unknown command\n\n");
    return usage();
  }

  smb2 = smb2_init_context();
  if (smb2 == NULL) {
    fprintf(stderr, "Failed to init context\n");
    result_code = ESMBPARSE;
    goto error;
  } else {
    dispose_of_context = destroy_context;
  }

  set_password_from_env(smb2);

  smb2_set_security_mode(smb2, SMB2_NEGOTIATE_SIGNING_ENABLED);
  smb2_set_timeout(smb2, DEFAULT_TIMEOUT);
  // unless the url ends in '?sec=krb5', default to ntlm
  smb2_set_authentication(smb2, SMB2_SEC_NTLMSSP);

  url = smb2_parse_url(smb2, smb_share);
  if (url == NULL) {
    fprintf(stderr, "Failed to parse url: %s\n", smb2_get_error(smb2));
    result_code = ESMBPARSE;
    goto error;
  } else {
    dispose_of_url = destroy_url;
  }

  if (url->domain) {
    smb2_set_user(smb2, url->user);
    smb2_set_domain(smb2, url->domain);
  }

  if (smb2_connect_share(smb2, url->server, url->share, url->user) < 0) {
    fprintf(stderr, "smb2_connect_share failed. %s\n", smb2_get_error(smb2));
    result_code = ESMBCONNECT;
    goto error;
  } else {
    dispose_of_connection = disconnect_share;
  }

  switch (what) {
  case CMD_LS:
    result_code = ls(smb2, url->path);
    goto error; /* this command must not print anything */

  case CMD_PUT:
    result_code = put(local_filename, smb2, url->path);
    break;

  case CMD_GET:
    result_code = get(smb2, url->path, local_filename);
    break;

  case CMD_DEL:
    result_code = del(smb2, url->path);
    break;
  }

  if (result_code == 0) {
    printf("OK: command %s completed successfully\n", command);
  } else {
    printf("ERROR: unable to execute %s\n", command);
  }

error:
  dispose_of_url(url);
  dispose_of_connection(smb2);
  dispose_of_context(smb2);

  return result_code;
}
