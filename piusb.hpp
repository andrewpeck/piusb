#include <libusb-1.0/libusb.h>
/*
 * Methods common to the Picard USB Communications
 * devices
 */
class picard {
public:
    int usbOpen(int vid, int pid);
    int usbClose();
    int usbWrite(unsigned char *data, int length);
    int usbRead (unsigned char *data, int length);

private:
    libusb_device_handle *dev_handle;
    libusb_context *ctx; //a libusb session
};

/*
 * Class for
 * devices
 */
class motor : public picard {
public:
    motor();
    ~motor();
    int setVelocity(int velocity);
    int setPosition(int position);
    int getPosition();
    int goHome();
private:
    int velocity_;
    // Motor
    static const int vendor_id_  = 0x0461;
    static const int product_id_ = 0x0020;
};

class relay : public picard {
public:
    relay();
    ~relay();
    int getState ();
    int setState (int status);
    int setState (int relay, bool on);
private:
    static const int vendor_id_  = 0x0461;
    static const int product_id_ = 0x0010;
};
