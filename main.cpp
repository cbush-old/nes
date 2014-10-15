#include <iostream>
#include <fstream>

#include "nes.h"
#include "bus.h"
#include "rom.h"

int main(int argc, char* argv[]){

  try {

    std::ifstream script (argv[2]);

    NES nes (argv[1], script);

    nes.run();

  } catch(int) {

    std::cout << "died\n";

  }

}
