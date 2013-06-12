all:
	g++ -std=c++11 nes.cpp -lSDL2 -lGL

debug:
	g++ -std=c++11 nes.cpp -lSDL2 -lGL -DDEBUG_CPU

pputest:
	g++ -std=c++11 nes.cpp -lSDL2 -lGL -DPPU_TEST

render1:	
	g++ -std=c++11 nes.cpp -lSDL2 -lGL -DRENDER1
