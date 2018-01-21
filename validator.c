#include "err.h"
#include "helper.h"

void updateTesterResults(pid_t testerPid, bool accepted) {
    //znajdz na liscie i zaaktualizuj
    //jesli nie ma na liscie to dodaj
}

int main() {
    summary results;
    /* testers list */
    bool endSignalReceived = false;

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

                    mgd_t desc = mq_open(qName, O_RDONLY | O_CREAT);
                    if (desc == (mqd_t) -1) {
                        syserr("Error in mq_open");
                    }

                    while (!endSignalReceived) {
                        /* "pid:word" OR "!" */
                        ret = mq_receive(desc, buffer, MAXLEN, NULL);
                        if (ret < 0) {
                            syserr("Error in rec: ");
                        }

                        if (strncmp(buffer, "!", 3)) {
                            endSignalReceived = true;
                            //wyślij do run "!" TODO
                        }
                        else {
                            //przekaz do run słowo i pid testera TODO
                        }
                    }
                    
                    if (mq_close(desc)) {
                        syserr("Error in close:");
                    }

                    exit(0);

                default:
                    while (true) {//zatrzymuj
                        //odbierz odpowiedz
                        //zaaktualizuj podsumowanie
                        //zaaktualizuj podsumowanie danego testera - potrzebujesz do tego listy testerów, żeby wiedzieć czy zaaktualizować czy dodać nowego
                        //wyślij odpowiedz dla danego testera
                        //zaaktualizuj liczniki
                        //zaczekaj na run? na koniec zaczekaj na wszystkie runy?
                    }
            }
    }

    //wypisz results
    //wypisz podsumowanie dla każdego testera

    return 0;
}

//while ((wpid = wait(&status)) > 0); // this way, the father waits for all the child