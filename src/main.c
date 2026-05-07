#include <stdio.h>

#include "application.h"

int main(void){
    if(application_init("Transport Traffic Model")){
        fprintf(stderr, "Application failed!\n");
        return 0;
    }

    while(application_is_running()){
        application_update();
    }

    application_shutdown();
    return 0;
}

