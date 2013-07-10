all:
	g++ -std=c++11 -c *.cpp -O3 -I. -Iinclude; make o

o:
	g++ *.o -lSDL2 -lGL

clean:
	rm *.o

ppu:
	g++ -std=c++11 -c ppu.cpp -O3 -I. -Iinclude; make o

apu:
	g++ -std=c++11 -c apu.cpp -O3 -I. -Iinclude; make o

cpu:
	g++ -std=c++11 -c cpu.cpp -O3 -I. -Iinclude; make o

rom:
	g++ -std=c++11 -c rom.cpp -O3 -I. -Iinclude; make o

io:
	g++ -std=c++11 -c io.cpp -O3 -I. -Iinclude; make o
