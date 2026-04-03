#include "application.h"
#include <stdio.h>

int main(void){
    FILE *test = fopen("test_start.txt", "w");
    fprintf(test, "Program started\n");
    fflush(test);
    fclose(test);

    if(application_init("Transport Traffic Model")){
        return 0;
    }

    while(application_is_running()){
        application_update();
    }

    application_shutdown();

    return 0;
}

