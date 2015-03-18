#include "libpiusb.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>
int main() {
    laser laser;
    if (laser.getStatus())
        laser.setOff();
    else
        laser.setOn();

    //laser.setOn();
    //srand(time(NULL));
    //int state = rand() % 1000;

    //twister twister;
    //twister.setVelocity(1);


    //printf("%i\n", twister.getPosition());
    //twister.setPosition(400);
    //for (int i=0; i<40; i++) {
    //    printf("extend: %i\n", twister.getPosition());
    //}

    //twister.setZero();
    //twister.setPosition(-500);
    //for (int i=0; i<40; i++) {
    //    printf("contract: %i\n", twister.getPosition());
    //}

    //motor motor;

    //if (motor.getPosition() == 0)
    //    motor.setPosition(1500);
    //else
    //    motor.setPosition(0);

    //for (int i=0; i<40; i++) {
    //    printf("%i\n", motor.getPosition());
    //    //printf("%i", motor.getPosition());
    //}

    //relay relay;
    //relay.setState(state);
    //sleep(1);
    //relay.setState(0);
    //sleep(1);
    //relay.setState(0, 1);
    //sleep(1);
    //relay.setState(1, 1);
    //sleep(1);
    //relay.setState(2, 1);
    //sleep(1);
    //relay.setState(3, 1);
}
