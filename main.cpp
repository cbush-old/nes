#include <iostream>
#include <fstream>

#include "nes.h"
#include "bus.h"
#include "rom.h"

int main(int argc, char* argv[]){

  ROM *rom { NULL };

  try {

    std::ifstream script (argv[2]);

    rom = load_ROM(argv[1]);
    NES nes (*rom, script);

    nes.run();

  } catch(int) {

    std::cout << "died\n";

  }

  if (rom) {

    unload_ROM(rom);

  }

}
