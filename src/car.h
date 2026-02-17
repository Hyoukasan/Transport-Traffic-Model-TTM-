// edited, turn this file in

#ifndef _car_h
#define _car_h

#include <stdlib.h>
#include <stdbool.h>

typedef struct Car
{
    // MADE BY ME helper variable to help me visualize which car's 'turn' it is
    int carID;

    // the time step associated with the addCar event that added this car
    int stepAdded;

    // the intersection this car started from
    int origin;

    // the intersection this car is driving toward currently
    int next;

    // the intersection this car wants to ultimately stop at
    int destination;

    // track whether a car has moved during this time step (remember to reset to false before next time step)
    bool moved;

}  Car;

Car* createCar( int carID, int stepAdded, int origin, int next, int destination );
void freeCar( Car* c );

#endif