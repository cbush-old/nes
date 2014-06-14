CC= g++
CFLAGS= -Wall -std=c++11
INCLUDE=
LDFLAGS=
LIBS=

SOURCES= \
	get_mapper.cpp

OBJECTS= $(SOURCES:.cpp=)
BUILD = $(CC) $(CFLAGS) $(INCLUDE)

all: $(OBJECTS)

get_mapper:
	$(BUILD) get_mapper.cpp -o get_mapper

clean:
	rm $(OBJECTS)