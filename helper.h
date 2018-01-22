#ifndef _VTR_HELPER_
#define _VTR_HELPER_

#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <mqueue.h>
#include <wait.h>

#define MAXLEN 1019
#define UINTMAXLEN 13
#define QNAMEMAXLEN 20
#define PIDMAXLEN 8

typedef struct {
    pid_t subjectPid;
    int received;
    int sent;
    int accepted;
} summary;

struct mq_attr attr = {
    .mq_maxmsg = 10,           /* Max. # of messages on queue */
    .mq_msgsize = MAXLEN             /* Max. message size (bytes) */
};


#endif /* _VTR_HELPER_ */