.PHONY : clean

CXX=g++

CPPFLAGS= -fPIC -g -Wall -I.
LDFLAGS= -shared

TARGET=libpiusb.so

all:
	$(CXX) libpiusb.cpp -fPIC -g -L. -o libpiusb.so -lusb-1.0 -shared

clean:
	rm -f libpiusb.so piusb

install: 
	cp libpiusb.so /usr/lib/libpiusb.so
	chmod 755 /usr/lib/libpiusb.so
	cp piusb.hpp /usr/include/piusb.hpp
	chmod 644 /usr/include/piusb.hpp

exe: 
	$(CXX) -Wall piusb.cpp -o piusb -lpiusb
