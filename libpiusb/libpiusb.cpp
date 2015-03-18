// Compiles with :
//      g++ -o motor motor.cpp -lusb-1.0
//
#include "libpiusb.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <unistd.h>

#define ENDPOINT    1

#define VELOCITY 0x8

#define DEBUG       0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

int picard::usbOpen(int vid, int pid)
{
    /* pointer pointer to list of devices */
    libusb_device **devs;
    /* create a libusb instance */
    libusb_context *ctx = NULL;

    /* initialize libusb instance */
    int status = libusb_init(&ctx);
    if (status < 0) {
        fprintf(stderr, "ERROR: Problem Initiating Device\n");
        return EXIT_FAILURE;
    }

    /* Set verbosity level */
    libusb_set_debug(ctx, 3);

    /* get the list of devices */
    int cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "ERROR: Problem Getting Device\n");
        return EXIT_FAILURE;
    }

    debug_print("%i %s", cnt, "devices in list\n");

    /* Open the desired device and return a handle */
    dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);

    if(dev_handle == NULL) {
        fprintf(stderr, "ERROR: Cannot open USB Device. Disconnected?\n");
        return EXIT_FAILURE;
    }
    else {
        debug_print("%s\n", "USB Device Opened");
    }

    /* Done with the list, free it */
    libusb_free_device_list(devs, 1);

    /*   Check if the kernel driver is attached
     *   If so, detach it.  */
    if (libusb_kernel_driver_active(dev_handle, 0) == 1) {
        debug_print("%s\n", "Kernel Driver is Active... detaching");
        if (libusb_detach_kernel_driver(dev_handle, 0) == 0)
            debug_print("%s\n", "Kernel Driver Successfully Detached");
        else {
            fprintf(stderr, "ERROR: Failed to Detach Kernel Driver\n");
            return EXIT_FAILURE;
        }
    }

    /* claim usb interface */
    status = libusb_claim_interface(dev_handle, 0);
    if(status < 0) {
        fprintf(stderr, "ERROR: Cannot Claim Interface\n");
        return EXIT_FAILURE;
    }
    else
        debug_print("%s\n", "Succesfully Claimed Interface\n");

    /* fini */
    return EXIT_SUCCESS;
}

int picard::usbClose()
{
    /* release the claimed interface */
    int status = libusb_release_interface(dev_handle, 0);
    if(status!=0) {
        fprintf(stderr, "ERROR: Failed to release USB Interface\n");
        return 1;
    }

    debug_print("%s\n", "Successfully Released Interface");

    /* Close the device */
    libusb_close(dev_handle);
    /* Close our usb instance */
    libusb_exit(ctx);

    return EXIT_SUCCESS;
}

int picard::usbWrite(unsigned char *data, int length)
{
    debug_print("%s", "Writing Data: ");
    for (int i=0; i<7; i++) {
        debug_print("%02X", data[i]);
    }
    debug_print("%s", "\n");

    int count;
    int status = libusb_bulk_transfer(dev_handle, (ENDPOINT | LIBUSB_ENDPOINT_OUT), data, length, &count, 0);
    if (status == 0 && count == 8)
        debug_print("%s\n", "Write Successful");
    else {
        printf("Write Failed");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int picard::usbRead (unsigned char *data, int length)
{
    int count;
    int status = libusb_bulk_transfer(dev_handle, (ENDPOINT | LIBUSB_ENDPOINT_IN), data, length, &count, 0);
    if (status == 0 && count == length)
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

twister::twister() {
    usbOpen(vendor_id, product_id);
    setVelocity(VELOCITY);
}

twister::~twister() {
    usbClose();
}

int twister::setZero()
{
    unsigned char data[8] = {0};
    usbRead(data,8);
    data[0] = velocity_ | 0x1;
    usbWrite(data, sizeof(data)/sizeof(data[0]));
    return EXIT_SUCCESS;
}

int twister::setVelocity (int velocity)
{
    velocity_ = (0xF & (16-velocity)) << 4;
    unsigned char data[8] = {0};

    if (velocity >10 || velocity < 1)
        return EXIT_FAILURE;

    data[0] = velocity_;
    usbWrite(data, sizeof(data)/sizeof(data[0]));
    return EXIT_SUCCESS;
}

int twister::setPosition (int position)
{
    if (position > 0x7FF || position < (-1)*0x7FF)
        return EXIT_FAILURE;

    unsigned char data[8] = {0};

    data[0] = velocity_ | 0x8;
    data[1] = 0xFF & (position);
    data[2] = 0xFF & (position >> 8 );

    usbWrite(data, sizeof(data)/sizeof(data[0]));
    return EXIT_SUCCESS;
}

int twister::getPosition ()
{
    unsigned char data[8] = {0};
    usbRead(data,8);

    int position_u = (data[1]) | (data[2] << 8);

    //Cast the unsigned int to the signed version
    int position = *(int16_t*)&position_u;

    return (position);
}

motor::motor() {
    usbOpen(vendor_id, product_id);
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

    if (velocity >10 || velocity < 1)
        return EXIT_FAILURE;

    data[0] = velocity_;
    usbWrite(data, sizeof(data)/sizeof(data[0]));
    return EXIT_SUCCESS;
}

int motor::setPosition (int position)
{
    if (position > 1900 || position < 0)
        return EXIT_FAILURE;

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
    usbOpen(vendor_id, product_id);
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

    debug_print("status: %1X\n", status);


    // mask on the bit in question if state is on
    if (on)
        status |=   0x1 << relay;
    else
        status = status & (0xF & ~(0x1 << relay));

    debug_print("status: %1X\n", status);

    unsigned char data[8] = {0};
    data [0] = 0xF & status;

    usbWrite(data, sizeof(data)/sizeof(data[0]));

    return (status);
}

int relay::getState()
{
    unsigned char data[8] = {0};

    /*
     *  This is here twice for a reason.
     *  Do NOT take it out unless you know
     *  what you are doing.
     *
     *  The USB relay returns weird readings
     *  on the first read (the Lord knows why).
     *
     *  Read twice and we are OK
     *
     *  Who knows why, but its not worth
     *  figuring out right now
     */
    usbRead(data,8);
    usbRead(data,8);

    int status = 0xF & data[0];
    return status;
}

void laser::setOn()
{
    unsigned char data[8] = {0};
    data[0] = 0x04;
    usbWrite(data, sizeof(data)/sizeof(data[0]));
}

void laser::setOff()
{
    unsigned char data[8] = {0};
    usbWrite(data, sizeof(data)/sizeof(data[0]));
}

//bool laser::getStatus()
//{
//    unsigned char data[8] = {0};
//    usbRead(data,8);
//    usbRead(data,8);
//
//    int status = 0xFF & data[0];
//    return ((bool) status);
//}

laser::laser()
{
    usbOpen(vendor_id, product_id);
}

laser::~laser()
{
    usbClose();
}
