#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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
    char buffer[MAXLEN];
    const char *tempName = "/tempQ";/*TEMP TODO USUN*/
    const char *qName2 = "/validatorQ";

    /*
    mq_unlink(tempName);
    mq_unlink(qName2);
    return 0;*/

    
    //forkuj, dziecko odbiera słowa i przekazuje je do run, rodzic odbiera odpowiedzi od run i przekazuje je do testerow
    //gdy dziecko dostanie ! to wysyła ! do run i konczy sie, run wie ze ma sie skonczyc i wysyla ! do glownego i konczy wszystkie pomniejsze run,
    //glowny odbiera ! wypisuje wszystkie raporty i konczy 

    switch (fork()) {
        case -1:
            syserr("Error in fork\n");
            break;

        case 0:
            //exec run //TODO przenies wczytywanie do run, stworz na poczatku proces run, ktory bedzie naczelnikiem dla validator
            /* TEMP */exit(0);/* TEMP */

        default:
            switch (fork()) {
                case -1:
                    syserr("Error in fork\n");
                    break;

                case 0: /*empty statement */;
                    const char *qName = "/validatorQ";

                    mqd_t desc = mq_open(qName, O_RDONLY | O_CREAT, 0777, &attr);
                    if (desc == (mqd_t) -1) {
                        syserr("Error in mq_open (qName)");
                    }

                    mqd_t tempDesc = mq_open(tempName, O_WRONLY | O_CREAT, 0777, &attr);
                    if (tempDesc == (mqd_t) -1) {
                        syserr("Error in mq_open (tempName)");
                    }

                    while (!endSignalReceived) {
                        /* "pid:word" OR "!" */
                        ret = mq_receive(desc, buffer, MAXLEN, NULL);
                        if (ret < 0) {
                            syserr("Error in rec (qName): ");
                        }
                        printf("Validator: odebrałem %s\n od testera\n", buffer);

                        if (strncmp(buffer, "!", 2)) {
                            endSignalReceived = true;
                            //wyślij do run "!" TODO
                            /* TEMP CODE */
                            ret = mq_send(tempDesc, buffer, strlen(buffer) + 1, 1);
                            if (ret) {
                                syserr("Error in mq_send: ");
                            }
                            printf("Validator: wysłałem %s\n do validator\n", buffer);
                            /* TEMP CODE */
                        }
                        else {
                            //przekaz do run słowo i pid testera TODO
                            /* TEMP CODE */
                            ret = mq_send(tempDesc, buffer, strlen(buffer) + 1, 1);
                            if (ret) {
                                syserr("Error in mq_send: ");
                            }
                            printf("Validator: wysłałem %s\n do validator\n", buffer);
                            /* TEMP CODE */
                        }
                    }
                    
                    if (mq_unlink(qName)) {
                        syserr("Error in close:");
                    }

                    if (mq_close(tempDesc)) {
                        syserr("Error in close:");
                    }                    

                    exit(0);

                default: /* empty statement */;
                    /* TEMP CODE */
                    mqd_t tempDesc2 = mq_open(tempName, O_RDONLY | O_CREAT, 0777, &attr);
                    if (tempDesc2 == (mqd_t) -1) {
                        syserr("Error in mq_open (tempName2)");
                    }


                    /* TEMP CODE */
                    while (!endSignalReceived) {
                        /* TEMP CODE */
                        /* "pid:word" OR "!" */
                        ret = mq_receive(tempDesc2, buffer, MAXLEN, NULL);
                        if (ret < 0) {
                            syserr("Error in rec (tempName): ");
                        }
                        printf("Validator: odebrałem %s od validator\n", buffer);

                        if (strncmp(buffer, "!", 2)) {
                            endSignalReceived = true;
                        }
                        else {
                            int testerPid = strtol(buffer, NULL, 0);

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
                        /* TEMP CODE */
                        //odbierz odpowiedz
                        //zaaktualizuj podsumowanie
                        //zaaktualizuj podsumowanie danego testera - potrzebujesz do tego listy testerów, żeby wiedzieć czy zaaktualizować czy dodać nowego
                        //wyślij odpowiedz dla danego testera
                        //zaaktualizuj liczniki
                        //zaczekaj na run? na koniec zaczekaj na wszystkie runy?
                    }

                    /* TEMP CODE */
                    if (mq_unlink(tempName)) {
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