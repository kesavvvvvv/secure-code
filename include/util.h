#ifndef UTIL_H
#define UTIL_H

int ensure_dirs(void);
int write_all(int fd, const void *buf, size_t n);
int copy_file_rw(const char *src, const char *dst);
int file_exists(const char *path);
int path_is_within_root(const char *root, const char *path);
int prepare_jail_files(const char *host_prog, const char *jail_prog, const char *jail_root);

#endif

