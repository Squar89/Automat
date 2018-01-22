#include <stdlib.h>
#include <wait.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "dArray.h"

#include "helper.h"
#include "err.h"

typedef struct Automaton {
    int N; int A; int Q; int U; int F;
    int starting;
    bool uni[109];
    bool exi[109];
    dArray ***map;
} Automaton;

int main() {
    int ret;
    int x, q, r;
    char buffer[MAXLEN];
    char c, a;
    bool endSignalReceived = false;
    Automaton automat;
    const char *queryRunQName = "/queryRunQ";
    const char *resultRunQName = "/resultRunQ";

    scanf("%d %d %d %d %d ", &automat.N, &automat.A, &automat.Q, &automat.U, &automat.F);
    scanf("%d ", &automat.starting);
    for (int i = 0; i < automat.F; i++) {
        scanf("%d ", &x);

        if (i < automat.U) {
            automat.uni[x] = 1;
        }
        else {
            automat.exi[x] = 1;
        }
    }

    automat.map = malloc((automat.Q + 2) * sizeof(dArray**));
    for (int i = 0; i < automat.Q + 2; i++) {
        automat.map[i] = malloc((automat.A + 2) * sizeof(dArray*));
        for (int j = 0; j < automat.A + 2; j++) {
            automat.map[i][j] = setup();
        }
    }

    while (true) {
        scanf("%d %c %d", &q, &a, &r);
        while (true) {
            push(automat.map[q][(int) a - (int)'a'], r); 

            scanf("%c", &c);
            if (c == '\n' || feof(stdin)) {
                break;
            }
            scanf("%d", &r);
        }
        if (feof(stdin)) {
            break;
        }
    }


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

    for (int i = 0; i < automat.Q + 2; i++) {
        for (int j = 0; j < automat.A + 2; j++) {
            clear(automat.map[i][j]);
        }
        free(automat.map[i]);
    }
    free(automat.map);

    return 0;
}