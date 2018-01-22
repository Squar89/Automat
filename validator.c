#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include "err.h"
#include "helper.h"
#include "dArray.h"

int main() {
    summary results[MAXPID];
    bool seen[MAXPID] = {0};
    bool endSignalReceived = false;
    int ret;
    char buffer[MAXLEN];
    const char *queryRunQName = "/queryRunQ";
    const char *resultRunQName = "/resultRunQ";

    //forkuj, dziecko odbiera słowa i przekazuje je do run, rodzic odbiera odpowiedzi od run i przekazuje je do testerow
    //gdy dziecko dostanie ! to wysyła ! do run i konczy sie, run wie ze ma sie skonczyc i wysyla ! do glownego i konczy wszystkie pomniejsze run,
    //glowny odbiera ! wypisuje wszystkie raporty i konczy 

    switch (fork()) {
        case -1:
            syserr("Error in fork\n");
            break;

        case 0:
            execl("run", "run",NULL);
            syserr("Error in exec\n");

        default: ;
            int pipe_dsc[2];
            if (pipe(pipe_dsc) == -1) syserr("Error in pipe\n");

            switch (fork()) {
                case -1:
                    syserr("Error in fork\n");
                    break;

                case 0: ;
                    const char *qName = "/validatorQ";
                    int receivedCount = 0;

                    if (close(pipe_dsc[0]) == -1){
                        syserr("Error in close(pipe_dsc[0])\n");
                    }

                    mqd_t desc = mq_open(qName, O_RDONLY | O_CREAT, 0777, &attr);
                    if (desc == (mqd_t) -1) {
                        syserr("Error in mq_open (qName)");
                    }

                    mqd_t runInDesc = mq_open(queryRunQName, O_WRONLY | O_CREAT, 0777, &attr);
                    if (runInDesc == (mqd_t) -1) {
                        syserr("Error in mq_open (queryRunQName)");
                    }

                    while (!endSignalReceived) {
                        /* "pid:word" OR "!" */
                        ret = mq_receive(desc, buffer, MAXLEN, NULL);
                        if (ret < 0) {
                            syserr("Error in rec (qName): ");
                        }
                        printf("Validator: odebrałem %s od testera\n", buffer);

                        if (strncmp(buffer, "!", 2) == 0) {
                            endSignalReceived = true;

                            char *msg = (char*) malloc(UINTMAXLEN * sizeof(char));
                            ret = sprintf(msg, "%d", receivedCount);
                            if (ret < 0) {
                                free(msg);
                                syserr("Error in sprintf: ");
                            }
                            if (write(pipe_dsc[1], msg, sizeof(msg)) != sizeof(msg)) {
                                free(msg);
                                syserr("Error in write\n");
                            }
                            free(msg);
                            if (close(pipe_dsc[1]) == -1){
                                syserr("Error in close(pipe_dsc[1])\n");
                            }

                            ret = mq_send(runInDesc, buffer, strlen(buffer) + 1, 1);
                            if (ret) {
                                syserr("Error in mq_send: ");
                            }
                            printf("Validator: wysłałem %s do run\n", buffer);
                        }
                        else {
                            receivedCount++;

                            ret = mq_send(runInDesc, buffer, strlen(buffer) + 1, 1);
                            if (ret) {
                                syserr("Error in mq_send: ");
                            }
                            printf("Validator: wysłałem %s do run\n", buffer);
                        }
                    }
                    
                    if (mq_unlink(qName)) {
                        syserr("Error in close:");
                    }

                    if (mq_close(runInDesc)) {
                        syserr("Error in close:");
                    }                    

                    exit(0);

                default: ;
                    pid_t thisPid = getpid();
                    int sentCount = 0;
                    int totalReceived;
                    if (close(pipe_dsc[1]) == -1){
                        syserr("Error in close(pipe_dsc[1])\n");
                    }
                    mqd_t runOutDesc = mq_open(resultRunQName, O_RDONLY | O_CREAT, 0777, &attr);
                    if (runOutDesc == (mqd_t) -1) {
                        syserr("Error in mq_open (qName)");
                    }

                    results[thisPid].subjectPid = thisPid;
                    results[thisPid].received = 0;
                    results[thisPid].sent = 0;
                    results[thisPid].accepted = 0;

                    while (!endSignalReceived || sentCount < totalReceived) {
                        /* "A|pid:word" OR "N|pid:word" OR "!" */
                        ret = mq_receive(runOutDesc, buffer, MAXLEN, NULL);
                        if (ret < 0) {
                            syserr("Error in rec (runOutDesc): ");
                        }
                        printf("Validator: odebrałem %s od run\n", buffer);

                        if (strncmp(buffer, "!", 2) == 0) {
                            endSignalReceived = true;

                            if (read(pipe_dsc[0], buffer, MAXLEN - 1) == 0) {
                                syserr("Error in read\n");
                            }
                            int k = strlen(buffer);
                            buffer[k] = '\0';
                            
                            totalReceived = strtol(buffer, NULL, 0);
                            printf("==================\n%d\n===========\n", totalReceived);

                            if (close(pipe_dsc[0]) == -1){
                                syserr("Error in close(pipe_dsc[0])\n");
                            }
                        }
                        else {
                            int testerPid = strtol(buffer + 2, NULL, 0);

                            char *msg = (char*) malloc((2 + strlen(strchr(buffer, ':') + 1) + 1) * sizeof(char));
                            if (*buffer == 'A') {
                                ret = sprintf(msg, "A|%s", strchr(buffer, ':') + 1);
                            }
                            else {
                                ret = sprintf(msg, "N|%s", strchr(buffer, ':') + 1);   
                            }
                            if (ret < 0) {
                                syserr("Error in sprintf: ");
                            }

                            char *resultsQ = (char*) malloc((9 + PIDMAXLEN + 1) * sizeof(char));
                            ret = sprintf(resultsQ, "/results:%d", testerPid);
                            if (ret < 0) {
                                free(msg);
                                free(resultsQ);
                                syserr("Error in sprintf: ");
                            }

                            mqd_t resultDesc = mq_open(resultsQ, O_WRONLY | O_CREAT, 0777, &attr);
                            if (resultDesc == (mqd_t) -1) {
                                free(msg);
                                free(resultsQ);
                                syserr("Error in mq_open");
                            }

                            ret = mq_send(resultDesc, msg, strlen(msg) + 1, 1);
                            //free(msg);
                            //free(resultsQ); TODO
                            if (ret) {
                                free(msg);
                                free(resultsQ);
                                syserr("Error in mq_send: ");
                            }
                            printf("Validator: wysłałem %s do testera\n", msg);
                            free(msg);
                            free(resultsQ);
                            sentCount++;

                            if (mq_close(resultDesc)) {
                                syserr("Error in close:");
                            }

                            if (!seen[testerPid]) {
                                seen[testerPid] = 1;
                                results[testerPid].subjectPid = testerPid;
                                results[testerPid].received = 0;
                                results[testerPid].sent = 0;
                                results[testerPid].accepted = 0;
                            }
                            results[testerPid].sent++;
                            results[testerPid].received++;
                            if (*buffer == 'A') {
                                results[thisPid].accepted++;
                                results[testerPid].accepted++;
                            }

                            results[thisPid].sent++;
                            results[thisPid].received++;
                        }
                    }

                    if (mq_unlink(resultRunQName)) {
                        syserr("Error in close:");
                    }

                    printf("Rcd: %d\nSnt: %d\nAcc: %d\n", results[thisPid].received, results[thisPid].sent, results[thisPid].accepted);

                    for (int pid = 0; pid < MAXPID; pid++) {
                        if (seen[pid]) {
                            printf("PID: %d\nRcd: %d\nAcc: %d\n", pid, results[pid].received, results[pid].accepted);
                        }

                        char *closingQ = (char*) malloc((9 + PIDMAXLEN + 1) * sizeof(char));
                        ret = sprintf(closingQ, "/results:%d", pid);
                        if (ret < 0) {
                            free(closingQ);
                            syserr("Error in sprintf: ");
                        }

                        mqd_t closingDesc = mq_open(closingQ, O_WRONLY, 0777, &attr);
                        if (closingDesc == (mqd_t) -1) {
                            free(closingQ);
                        }
                        else {
                            ret = mq_send(closingDesc, "!\0", 2, 1);
                            free(closingQ);
                            if (ret) {
                                syserr("Error in mq_send: ");
                            }

                            if (mq_close(closingDesc)) {
                                syserr("Error in close:");
                            }
                        }
                    }
            }
    }

    while (wait(0) > 0);//wait for all child processes

    return 0;
}