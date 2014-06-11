CC= g++
CFLAGS=-c -Wall -std=c++11 -O3 -DDEBUG_PPU_PRINT_FRAMERATE
INCLUDE= \
	-I. \
	-Iinclude \
	-I/System/Library/Frameworks/OpenGL.framework/Headers \
	-I/System/Library/Frameworks/ \
	-I/opt/local/include \
	-I/usr/include/GL \
	-I/usr/include/SDL2 

LDFLAGS= -L/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries -L/opt/local/lib
LIBS= -lSDL2 -lGL -lsamplerate

SOURCES= \
	src/apu.cpp     \
	src/cpu.cpp     \
	src/ppu.cpp     \
	src/rom.cpp     \
	src/nes.cpp     \
	src/controller_std.cpp \
	src/audio_sdl.cpp \
	src/audio_sox_pipe.cpp \
	src/video_sdl.cpp \
	src/input_sdl.cpp \
	src/input_script.cpp \
	src/input_script_recorder.cpp \
	src/mappers/sxrom.cpp \
	main.cpp

OBJECTS= $(SOURCES:.cpp=.o)

all: nes

nes: $(OBJECTS)
	$(CC) $(LDFLAGS) $(LIBS) $(OBJECTS) -o nes

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	rm *.o src/*.o
