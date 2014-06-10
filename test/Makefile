CC= g++
CFLAGS=-c -Wall -std=c++11 -O3 -DDEBUG_PPU_PRINT_FRAMERATE
INCLUDE= \
	-I. \
	-I../include \
	-I../src \
	-I/System/Library/Frameworks/OpenGL.framework/Headers \
	-I/System/Library/Frameworks/ \
	-I/opt/local/include \
	-I/usr/include/GL \
	-I/usr/include/SDL2 

LDFLAGS= -L/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries -L/opt/local/lib
LIBS= -lSDL2 -lGL -lsamplerate

SOURCES= \
	apu_test.cpp

OBJECTS= $(SOURCES:.cpp=.o)

all: tests

tests: $(OBJECTS)
	$(CC) $(LDFLAGS) $(LIBS) $(OBJECTS) -o test

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	rm *.o test
