.PHONY : clean

CC=g++

CPPFLAGS= -fPIC -g -L.
LDFLAGS= -shared -lusb-1.0

SOURCES = $(shell echo *.cpp)
HEADERS = $(shell echo *.hpp)
OBJECTS=$(SOURCES:.cpp=.o)

TARGET=libpiusb.so

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

install: 
	cp libpiusb.so /usr/lib/libpiusb.so

exe: 
	$(CC) piusb.cpp -o piusb -L. -lpiusb

$(TARGET) : $(OBJECTS)
	$(CC) $(CPPFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

