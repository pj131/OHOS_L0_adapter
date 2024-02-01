
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "string.h"
#include <stdarg.h>

int _open(const char* path, int oflag, ...);
int _close(int fd);
ssize_t _read(int fd, void* buf, size_t nbyte);
ssize_t _write(int fd, const void* buf, size_t nbyte);
off_t _lseek(int fd, off_t offset, int whence);
int _unlink(const char* path);