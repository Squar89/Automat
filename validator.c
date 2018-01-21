#include <stdbool.h>

#include "err.h"
#include "helper.h"

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

    //forkuj, dziecko odbiera słowa i przekazuje je do run, rodzic odbiera odpowiedzi od run i przekazuje je do testerow
    //gdy dziecko dostanie ! to wysyła ! do run i konczy sie, run wie ze ma sie skonczyc i wysyla ! do glownego i konczy wszystkie pomniejsze run,
    //glowny odbiera ! wypisuje wszystkie raporty i konczy 

    switch (fork()) {
        case -1:
            syserr("Error in fork\n");

        case 0:
            //exec run //TODO przenies wczytywanie do run, stworz na poczatku proces run, ktory bedzie naczelnikiem dla validator

        default:
            switch (fork()) {
                case -1:
                    syserr("Error in fork\n");

                case 0:
                    char word[MAXLEN];
                    char buffer[MAXLEN];
                    const char *qName = "/validatorQ";
                    const char *tempName = "/tempQ";

                    mgd_t desc = mq_open(qName, O_RDONLY | O_CREAT, 0777, NULL);
                    if (desc == (mqd_t) -1) {
                        syserr("Error in mq_open");
                    }

                    mgd_t tempDesc = mq_open(tempName, O_WRONLY | O_CREAT, 0777, NULL);
                    if (tempDesc == (mqd_t) -1) {
                        syserr("Error in mq_open");
                    }

                    while (!endSignalReceived) {
                        /* "pid:word" OR "!" */
                        ret = mq_receive(desc, buffer, MAXLEN, NULL);
                        if (ret < 0) {
                            syserr("Error in rec: ");
                        }

                        if (strncmp(buffer, "!", 2)) {
                            endSignalReceived = true;
                            //wyślij do run "!" TODO
                            /* TEMP CODE */
                            ret = mq_send(tempDesc, buffer, strlen(buffer), 1);
                            if (ret) {
                                syserr("Error in mq_send: ");
                            }
                            /* TEMP CODE */
                        }
                        else {
                            //przekaz do run słowo i pid testera TODO
                            /* TEMP CODE */
                            ret = mq_send(tempDesc, buffer, strlen(buffer), 1);
                            if (ret) {
                                syserr("Error in mq_send: ");
                            }
                            /* TEMP CODE */
                        }
                    }
                    
                    if (mq_close(desc)) {
                        syserr("Error in close:");
                    }

                    if (mq_close(tempDesc)) {
                        syserr("Error in close:");
                    }                    

                    exit(0);

                default:
                    /* TEMP CODE */
                    char buffer[MAXLEN];
                    const char *tempName = "/tempQ";

                    mgd_t tempDesc = mq_open(tempName, O_RDONLY | O_CREAT, 0777, NULL);
                    if (tempDesc == (mqd_t) -1) {
                        syserr("Error in mq_open");
                    }


                    /* TEMP CODE */
                    while (!endSignalReceived) {
                        /* TEMP CODE */
                        /* "pid:word" OR "!" */
                        ret = mq_receive(tempDesc, buffer, MAXLEN, NULL);
                        if (ret < 0) {
                            syserr("Error in rec: ");
                        }

                        if (strncmp(buffer, "!", 2)) {
                            endSignalReceived = true;
                        }
                        else {
                            int testerPid = strtol(buffer, buffer, MAXLEN, NULL);

                            char *msg = (char*) malloc(MAXLEN * sizeof(char));
                            ret = sprintf(msg, "A|%s", buffer + 1);
                            if (ret < 0) {
                                syserr("Error in sprintf: ");
                            }

                            /* TODO zrób to lepiej */
                            char *resultsQ = (char*) malloc(QNAMEMAXLEN * sizeof(char));
                            ret = sprintf(resultsQ, "/results:%d", testerPid);
                            if (ret < 0) {
                                free(msg);
                                free(resultsQ);
                                syserr("Error in sprintf: ");
                            }

                            mgd_t resultDesc = mq_open(resultsQ, O_WRONLY | O_CREAT, 0777, NULL);//TODO aktualnie nigdzie jej nie zamykasz
                            if (resultDesc == (mqd_t) -1) {
                                free(msg);
                                free(resultsQ);
                                syserr("Error in mq_open");
                            }

                            ret = mq_send(resultsQ, msg, strlen(msg), 1);
                            free(msg);
                            free(resultsQ);
                            if (ret) {
                                syserr("Error in mq_send: ");
                            }
                        }
                        /* TEMP CODE */
                        //odbierz odpowiedz
                        //zaaktualizuj podsumowanie
                        //zaaktualizuj podsumowanie danego testera - potrzebujesz do tego listy testerów, żeby wiedzieć czy zaaktualizować czy dodać nowego
                        //wyślij odpowiedz dla danego testera
                        //zaaktualizuj liczniki
                        //zaczekaj na run? na koniec zaczekaj na wszystkie runy?
                    }

                    /* TEMP CODE */
                    if (mq_close(tempDesc)) {
                        syserr("Error in close:");
                    }
                    /* TEMP CODE */
            }
    }

    //wypisz results
    //wypisz podsumowanie dla każdego testera

    return 0;
}

//while ((wpid = wait(&status)) > 0); // this way, the father waits for all the child