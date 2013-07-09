all:
	g++ -std=c++11 -c *.cpp -O3 -I. -Iinclude; make o

o:
	g++ *.o -lSDL2 -lGL

clean:
	rm *.o

ppu:
	g++ -std=c++11 -c ppu.cpp -O3 -I. -Iinclude; make o

cpu:
	g++ -std=c++11 -c cpu.cpp -I. -Iinclude; make o

io:
	g++ -std=c++11 -c io.cpp -I. -Iinclude; make o
