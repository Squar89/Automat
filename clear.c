#include <mqueue.h>

int main() {
    const char *qName = "/validatorQ";
    const char *queryRunQName = "/queryRunQ";
    const char *resultRunQName = "/resultRunQ";

    mq_unlink(qName);
    mq_unlink(queryRunQName);
    mq_unlink(resultRunQName);
}