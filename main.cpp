#include <iostream>
#include <fstream>

#include "nes.h"
#include "bus.h"
#include "rom.h"

int main(int argc, char* argv[]){

  try {

    std::ifstream script (argv[2]);

    ROM rom (argv[1]);
    NES nes (rom, script);

  } catch(int){

    std::cout << "died\n";

  }

}
