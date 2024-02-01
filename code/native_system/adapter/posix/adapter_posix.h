#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "string.h"
#include "osi_api.h"
#include "osi_log.h"

// extern int gettimeofday(struct timeval *tv, void *tz);
// time
// int clock_gettime(clockid_t clock_id, struct timespec *tp);
// posix
void _exit(int code);
int kill(pid_t pid, int code);
unsigned sleep(unsigned seconds);
void _fini(void);