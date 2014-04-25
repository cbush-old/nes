#include <iostream>
#include <thread>
#include <cmath>
#include <ctime>
#include <map>
#include <iomanip>
#include <functional>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

#include "bit.h"
#include "bus.h"
#include "rom.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::thread;
using std::string;
using std::ifstream;
using std::exception;
using std::runtime_error;

int main(int argc, char* argv[]){

  //vector<string> args (argv, argv + argc);

  try {
  
    bus::play(argv[1]);
    bus::start();

  } catch(int){
  
    cout << "died\n";
    
  }
  
}
