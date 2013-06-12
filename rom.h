#ifndef ROM_H
#define ROM_H

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

class ROM {
  protected:
    std::vector<uint8_t> mem;

  public:
    uint8_t& operator[](uint16_t addr);
    void write(uint8_t value, uint16_t addr);

  public:
    ROM();
    ROM(std::string const& src);

};

#endif
