#include <mqueue.h>

int main() {
    const char *tempName = "/tempQ";
    const char *qName2 = "/validatorQ";

    mq_unlink(tempName);
    mq_unlink(qName2);
}