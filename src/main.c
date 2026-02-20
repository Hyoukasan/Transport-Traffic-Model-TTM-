#include "application.h"

int main(void){
    application_init(800,600, "Prototype");

    while(1){
        application_update();
    }
}