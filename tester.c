#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#include "err.h"
#include "helper.h"

bool finish = false;

void sigHandler() {
    finish = true;
}

int main() {
    char resultsQName[QNAMEMAXLEN];
    summary result;
    int ret, customSignal;

    if (SIGRTMIN + 1 > SIGRTMAX) {
        syserr("SIGRTMIN + 1 exceeds SIGRTMAX\n");
    }
    customSignal = SIGRTMIN + 1;
    signal(customSignal, sigHandler);

    pid_t childPid;
    pid_t originPid = getpid();
    printf("PID: %d\n", originPid);

    ret = sprintf(resultsQName, "/results:%d", originPid);
    if (ret < 0) {
        syserr("Error in sprintf: ");
    }

    switch (childPid = fork()) {
        case -1:
            syserr("Error in fork\n");
            break;

        case 0: ;
            const char *qName = "/validatorQ";
            int count = 0;
            char word[MAXLEN];

            mqd_t validatorDesc = mq_open(qName, O_WRONLY | O_CREAT, 0777, &attr);
            if (validatorDesc == (mqd_t) -1) {
                syserr("Error in mq_open");
            }

            while (!finish) {
                word[0] = '\0';
                fgets(word, MAXLEN, stdin);
                char *pos;
                if ((pos=strchr(word, '\n')) != NULL) {
                    *pos = '\0';
                }

                if (feof(stdin)) {
                    finish = true;
                }
                if (strncmp(word, "!", 2) == 0) {
                    finish = true;
                    ret = mq_send(validatorDesc, word, strlen(word) + 1, 1);
                    if (ret) {
                        syserr("Error in mq_send: ");
                    }
                }
                else {
                    count++;

                    char *msg = (char*) malloc((PIDMAXLEN + 1 + strlen(word) + 1) * sizeof(char));
                    ret = sprintf(msg, "%d:%s", originPid, word);
                    if (ret < 0) {
                        syserr("Error in sprintf: ");
                    }

                    ret = mq_send(validatorDesc, msg, strlen(msg) + 1, 1);
                    free(msg);
                    if (ret) {
                        syserr("Error in mq_send: ");
                    }
                }
            }

            mqd_t countDesc = mq_open(resultsQName, O_WRONLY | O_CREAT, 0777, &attr);
            if (countDesc == (mqd_t) -1) {
                syserr("Error in mq_open");
            }

            char *msg = (char*) malloc(UINTMAXLEN * sizeof(char));
            ret = sprintf(msg, "%d", count);
            if (ret < 0) {
                free(msg);
                syserr("Error in sprintf: ");
            }

            ret = mq_send(countDesc, msg, strlen(msg) + 1, 1);
            free(msg);
            if (ret) {
                syserr("Error in mq_send\n");
            }

            if (mq_close(validatorDesc)) {
                syserr("Error in close:");
            }

            if (mq_close(countDesc)) {
                syserr("Error in close:");
            }

            exit(0);

        default: ;
            char buffer[MAXLEN];

            mqd_t resultsDesc = mq_open(resultsQName, O_RDONLY | O_CREAT, 0777, &attr);
            if (resultsDesc == (mqd_t) -1) {
                syserr("Error in mq_open");
            }

            result.received = 0;
            result.sent = -1;
            result.accepted = 0;
            while (!finish || result.sent == -1) {
                /* "A|word" OR "N|word OR "!" OR "sent" */
                ret = mq_receive(resultsDesc, buffer, MAXLEN, NULL);
                if (ret < 0) {
                    syserr("Error in rec: ");
                }

                /* end signal received */
                if (strncmp(buffer, "!", 2) == 0) {
                    kill(childPid, customSignal);
                    finish = true;
                }
                /* number of sent queries received */
                else if (buffer[0] != 'A' && buffer[0] != 'N') {
                    result.sent = strtol(buffer, NULL, 0);
                }
                /* result of a query received */
                else {
                    printf("%s %c\n", buffer + 2, buffer[0]);
                    result.received++;
                    if (buffer[0] == 'A') {
                        result.accepted++;
                    }
                }

                if (result.sent != -1 && result.sent == result.received) {
                    finish = true;
                }
            }

            if (mq_unlink(resultsQName)) {
                syserr("Error in close:");
            }

            if (wait(0) == -1) {
                syserr("Error in wait\n");
            }
    }

    printf("Snt: %d\nRcd: %d\nAcc: %d\n", result.sent, result.received, result.accepted);

    return 0;
}