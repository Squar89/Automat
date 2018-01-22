#include <stdlib.h>
#include <wait.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "helper.h"
#include "err.h"

int main() {
    int ret;
    char buffer[MAXLEN];
    bool endSignalReceived = false;
    const char *queryRunQName = "/queryRunQ";
    const char *resultRunQName = "/resultRunQ";

    //read automaton TODO

    mqd_t runInDesc = mq_open(queryRunQName, O_RDONLY | O_CREAT, 0777, &attr);
    if (runInDesc == (mqd_t) -1) {
        syserr("Error in mq_open (queryRunQName)");
    }
    while (!endSignalReceived) {
        /* "pid:word" OR "!" */
        ret = mq_receive(runInDesc, buffer, MAXLEN, NULL);
        if (ret < 0) {
            syserr("Error in rec (queryRunQName): ");
        }
        printf("Run: odebrałem %s od validator\n", buffer);

        if (strncmp(buffer, "!", 2) == 0) {
            endSignalReceived = true;
        }

        switch (fork()) {
            case -1:
                syserr("Error in fork\n");
                break;

            case 0: ;
                mqd_t runOutDesc = mq_open(resultRunQName, O_WRONLY | O_CREAT, 0777, &attr);
                if (runOutDesc == (mqd_t) -1) {
                    syserr("Error in mq_open (qName)");
                }

                if (strncmp(buffer, "!", 2) == 0) {
                    ret = mq_send(runOutDesc, buffer, strlen(buffer) + 1, 1);
                    if (ret) {
                        syserr("Error in mq_send: ");
                    }
                    printf("Run: wysłałem %s do validator\n", buffer);
                }
                else {
                    //przetwórz zapytanie TODO
                    char *msg = (char*) malloc((2 + strlen(buffer) + 1) * sizeof(char));
                    ret = sprintf(msg, "A|%s", buffer);
                    if (ret < 0) {
                        syserr("Error in sprintf: ");
                    }

                    ret = mq_send(runOutDesc, msg, strlen(msg) + 1, 1);
                    //free(msg);TODO
                    if (ret) {
                        free(msg);
                        syserr("Error in mq_send: ");
                    }
                    printf("Run: wysłałem %s do validator\n", msg);
                    free(msg);
                }

                if (mq_close(runOutDesc)) {
                    syserr("Error in close:");
                }

                exit(0);

            default:
                break;
        }
    }

    if (mq_unlink(queryRunQName)) {
        syserr("Error in close:");
    }

    while (wait(0) > 0);//wait for all child processes

    return 0;
}