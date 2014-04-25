CC= g++
CFLAGS=-c -Wall -std=c++11 -O3
INCLUDE= -I. -Iinclude -I/System/Library/Frameworks/OpenGL.framework/Headers -I/System/Library/Frameworks/ -I/opt/local/include
LDFLAGS= -L/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries -L/opt/local/lib
LIBS= -lSDL2 -lGL -lsamplerate

SOURCES= \
	src/apu.cpp     \
	src/bus.cpp     \
	src/cpu.cpp     \
	src/io.cpp      \
	src/main.cpp    \
	src/ppu.cpp     \
	src/rom.cpp

OBJECTS= $(SOURCES:.cpp=.o)

all: nes

nes: $(OBJECTS)
	$(CC) $(LDFLAGS) $(LIBS) $(OBJECTS) -o nes

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	rm src/*.o
