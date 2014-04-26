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

#include "bus.h"

int main(int argc, char* argv[]){

  try {

    NES nes (argv[1]);

  } catch(int){

    std::cout << "died\n";

  }

}
