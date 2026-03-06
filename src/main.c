#include "application.h"

int main(void){
    if(!application_init(1280, 720, "Transport Traffic Model")){
        return 1;
    }

    while(application_is_running()){
        application_update();
    }

    application_shutdown();

    return 0;
}

