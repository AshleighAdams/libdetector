# Doesn't work yet

CC = g++

LIBS = -lrt -lcv -lcxcore -lhighgui

CFLAGS = -static -std=c++0x -c

LIB=libdetector.a
LIBDEST=./

COMPILE = $(CC) $(CFLAGS) -c

OBJFILES := $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))

all: libdetector

libdetector: $(OBJFILES)
	$(CC) -o libdetector $(OBJFILES) -lrt

%.o: %.cpp
	$(COMPILE) -o $@ $<
