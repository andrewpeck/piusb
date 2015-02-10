// Compiles with :
//      g++ -o motor motor.cpp -lusb-1.0
//
#include "piusb.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <time.h>

using namespace std;


#define ENDPOINT    1

#define VELOCITY 0x8

#define DEBUG       0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

int picard::usbOpen(int vid, int pid)
{
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    libusb_context *ctx = NULL; //a libusb session
    int r; //for return values
    int cnt; //holding number of devices in list
    r = libusb_init(&ctx); //initialize the library for the session we just declared
    if (r < 0) {
        printf("ERROR: Problem Initiating Device\n");
        return EXIT_FAILURE;
    }
    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation

    cnt = libusb_get_device_list(ctx, &devs); //get the list of devices

    if (cnt < 0) {
        printf("ERROR: Problem Getting Device\n");
        return EXIT_FAILURE;
    }

    debug_print("%i %s", cnt, "devices in list\n");

    dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);

    if(dev_handle == NULL) {
        printf("%s\n", "ERROR: Cannot open USB Device. Disconnected?");
        return EXIT_FAILURE;
    }
    else {
        debug_print("%s\n", "USB Device Opened");
    }

    //free the list, unref the devices in it
    libusb_free_device_list(devs, 1);


    //find out if kernel driver is attached
    if (libusb_kernel_driver_active(dev_handle, 0) == 1) {
        debug_print("%s\n", "Kernel Driver is Active... detaching");
        if (libusb_detach_kernel_driver(dev_handle, 0) == 0)
            debug_print("%s\n", "Kernel Driver Successfully Detached");
        else {
            printf("%s\n", "ERROR: Failed to Detach Kernel Driver");
            return EXIT_FAILURE;
        }
    }

    //claim interface 0 (the first) of device (mine had jsut 1)
    r = libusb_claim_interface(dev_handle, 0);
    if(r < 0) {
        printf("%s\n", "ERROR: Cannot Claim Interface\n");
        return EXIT_FAILURE;
    }
    else
        debug_print("%s\n", "Succesfully Claimed Interface\n");
    return EXIT_SUCCESS;
}

int picard::usbClose()
{
    int r;
    r = libusb_release_interface(dev_handle, 0); //release the claimed interface
    if(r!=0) {
        printf("ERROR: Failed to release USB Interface\n");
        return 1;
    }

    debug_print("%s\n", "Successfully Released Interface");

    libusb_close(dev_handle); //close the device we opened
    libusb_exit(ctx); //needs to be called to end the

    //delete[] data; //delete the allocated memory for data
    return 0;
}

int picard::usbWrite(unsigned char *data, int length)
{
    debug_print("%s", "Writing Data: ");
    for (int i=0; i<7; i++) {
        debug_print("%02X", data[i]);
    }
    debug_print("%s", "\n");

    //used to find out how many bytes were written
    int actual;
    int r = libusb_bulk_transfer(dev_handle, (ENDPOINT | LIBUSB_ENDPOINT_OUT), data, length, &actual, 0);
    if (r == 0 && actual == 8) //we wrote the 4 bytes successfully
        debug_print("%s\n", "Write Successful");
    else {
        printf("Write Failed");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int picard::usbRead (unsigned char *data, int length)
{
    //used to find out how many bytes were written
    int actual;
    int r = libusb_bulk_transfer(dev_handle, (ENDPOINT | LIBUSB_ENDPOINT_IN), data, length, &actual, 0);
    if (r == 0 && actual == length) //we wrote the 4 bytes successfully
        debug_print("%s\n", "Read Successful");
    else {
        printf("Read Failed");
        return EXIT_FAILURE;
    }

    debug_print("%s", "Read Data: ");
    for (int i=0; i<7; i++) {
        debug_print("%02X", data[i]);
    }
    debug_print("%s", "\n");
    return EXIT_SUCCESS;
}

motor::motor() {
    usbOpen(vendor_id_, product_id_);
    setVelocity(VELOCITY);
}

motor::~motor() {
    usbClose();
}


int motor::goHome()
{
    unsigned char data[8] = {0};

    data[0]= velocity_ | 0x1;

    usbWrite(data, sizeof(data)/sizeof(data[0]));
    return EXIT_SUCCESS;
}

int motor::setVelocity (int velocity)
{
    velocity_ = (0xF & (16-velocity)) << 4;
    unsigned char data[8] = {0};

    data[0] = velocity_;

    usbWrite(data, sizeof(data)/sizeof(data[0]));
    return EXIT_SUCCESS;
}

int motor::setPosition (int position)
{
    unsigned char data[8] = {0};

    data[0]= velocity_ | 0x8;
    data[1] = 0xFF & (position);
    data[2] = 0xFF & (position >> 8 );

    usbWrite(data, sizeof(data)/sizeof(data[0]));
    return EXIT_SUCCESS;
}

int motor::getPosition ()
{
    unsigned char data[8] = {0};
    usbRead(data,8);

    int position = (data[1]) | (data[2] << 8);

    return (position);
}

relay::relay()
{
    usbOpen(vendor_id_, product_id_);
}

relay::~relay()
{
    usbClose();
}

int relay::setState (int status)
{
    unsigned char data[8] = {0};
    status = status & 0xF;
    data[0]= status;

    usbWrite(data, sizeof(data)/sizeof(data[0]));

    return (status);
}

int relay::setState(int relay, bool on)
{
    if (relay < 0 || relay > 3)
        return EXIT_FAILURE;

    int status = getState();

    printf("status: %1X\n", status);


    // mask on the bit in question if state is on
    if (on)
        status |=   0x1 << relay;
    else
        status = status & (0xF & ~(0x1 << relay));

    printf("status: %1X\n", status);

    unsigned char data[8] = {0};
    data [0] = 0xF & status;

    usbWrite(data, sizeof(data)/sizeof(data[0]));

    return (status);
}

int relay::getState()
{
    unsigned char data[8] = {0};

    // This is here twice for a reason.
    // Do NOT take it out unless you know
    // what you are doing.
    //
    // The USB relay returns weird readings
    // on the first read (the Lord knows why).
    //
    // Read twice and we are OK
    usbRead(data,8);
    usbRead(data,8);

    int status = 0xF & data[0];
    return status;
}

int main() {
    motor motor;

    if (motor.getPosition() == 0)
        motor.setPosition(1500);
    else
        motor.setPosition(0);

    for (int i=0; i<40; i++) {
        printf("%i\n", motor.getPosition());
        //printf("%i", motor.getPosition());
    }

    relay relay;
    srand(time(NULL));

    int state = rand() % 16;
    relay.setState(state);
    //relay.setState(0, 0);
    //relay.switchRelay(1, 1);
    //relay.switchRelay(2, 1);
    //relay.switchRelay(3, 1);
}
