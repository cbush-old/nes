#include <iostream>

#include "bus.h"
#include "rom.h"

int main(int argc, char* argv[]){

  try {

    ROM rom (argv[1]);
    NES nes (rom);

  } catch(int){

    std::cout << "died\n";

  }

}
