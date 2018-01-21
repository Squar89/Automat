#ifndef _VTR_HELPER_
#define _VTR_HELPER_

#include <unistd.h>
#include <sys/types.h>
#include <mqueue.h>
#include <wait.h>

#define MAXLEN 1019
#define UINTMAXLEN 13
#define QNAMEMAXLEN 20

typedef struct {
    pid_t subjectPid;
    int received;
    int sent;
    int accepted;
} summary;

#endif /* _VTR_HELPER_ */