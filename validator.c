#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include "err.h"
#include "helper.h"
#include "dArray.h"

/* TODO
void updateTesterResults(pid_t testerPid, bool accepted) {
    //znajdz na liscie i zaaktualizuj
    //jesli nie ma na liscie to dodaj
}
*/

int main() {
    summary results;
    /* testers list */
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

        default:
            switch (fork()) {
                case -1:
                    syserr("Error in fork\n");
                    break;

                case 0: ;
                    const char *qName = "/validatorQ";

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

                            ret = mq_send(runInDesc, buffer, strlen(buffer) + 1, 1);
                            if (ret) {
                                syserr("Error in mq_send: ");
                            }
                            printf("Validator: wysłałem %s do run\n", buffer);
                        }
                        else {
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
                    mqd_t runOutDesc = mq_open(resultRunQName, O_RDONLY | O_CREAT, 0777, &attr);
                    if (runOutDesc == (mqd_t) -1) {
                        syserr("Error in mq_open (qName)");
                    }

                    while (!endSignalReceived) {
                        /* "A|pid:word" OR "N|pid:word" OR "!" */
                        ret = mq_receive(runOutDesc, buffer, MAXLEN, NULL);
                        if (ret < 0) {
                            syserr("Error in rec (runOutDesc): ");
                        }
                        printf("Validator: odebrałem %s od run\n", buffer);

                        if (strncmp(buffer, "!", 2) == 0) {
                            endSignalReceived = true;
                        }
                        else {
                            int testerPid = strtol(buffer + 2, NULL, 0);

                            char *msg = (char*) malloc((2 + strlen(strchr(buffer, ':') + 1) + 1) * sizeof(char));
                            ret = sprintf(msg, "A|%s", strchr(buffer, ':') + 1);
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

                            if (mq_close(resultDesc)) {
                                syserr("Error in close:");
                            }
                        }
                        //zaaktualizuj podsumowanie
                        //zaaktualizuj podsumowanie danego testera - potrzebujesz do tego listy testerów, żeby wiedzieć czy zaaktualizować czy dodać nowego
                        //zaaktualizuj liczniki
                        //zaczekaj na run? na koniec zaczekaj na wszystkie runy?
                    }

                    if (mq_unlink(resultRunQName)) {
                        syserr("Error in close:");
                    }
            }
    }

    //w momencie jak wysyłasz ! do testerów, rob unlink kolejki, nie close

    //wypisz results
    //wypisz podsumowanie dla każdego testera

    return 0;
}

//while ((wpid = wait(&status)) > 0); // this way, the father waits for all the child