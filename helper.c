/*#include <helper.h>

void sigrtminCheck() {
    if (SIGRTMIN + 1 > SIGRTMAX) {
        syserr("SIGRTMIN + 1 exceeds SIGRTMAX\n");
    }
}

int sprintfCheck(char *str, char *format, ...) {
    int ret;

    va_list va;
    va_start (va, fmt);
    ret = vsprintf (buf, fmt, va);
    va_end (va);

    if (ret < 0) {
        syserr("Error in sprintfCheck: ");
    }
}*/