#include <stdio.h>
#include "application_headless.h"

int main(void) {
    printf("Starting TTM in headless mode...\n");

    if (application_init_headless()) {
        return 1;
    }

    while (application_is_running_headless()) {
        application_update_headless();
    }

    application_shutdown_headless();

    return 0;
}