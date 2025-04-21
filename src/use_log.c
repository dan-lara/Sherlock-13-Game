#include <stdio.h>
#include <stdlib.h>
#include "lib/inc/Logger.h"
#include "lib/src/Logger.c"
int main() {
    init_logger("system.log",0);

    log_message(LOG_INFO, "This is an info message.");
    log_message(LOG_WARNING, "This is a warning message.");
    log_message(LOG_ERROR, "This is an error message.");

    close_logger();
    return 0;
}