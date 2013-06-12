#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

void print_mapper(string const& src){

  ifstream file(src);
  
  if(!(
    file.get() == 'N' &&
    file.get() == 'E' &&
    file.get() == 'S' &&
    file.get() == 0x1A)){
    
    throw runtime_error ("Not iNES format");
    
  }
  
  int
    prg_rom_size { file.get() },
    chr_rom_size { file.get() },
    flag6 { file.get() },
    flag7 { file.get() },
    prg_ram_size { file.get() },
    flag9 { file.get() },
    flag10 { file.get() };
    
  file.seekg(0x10);
  
  int mapper_id { (flag6 >> 4) | (flag7 & 0xf0) };
  
  cout << src << ": Mapper " << mapper_id << '\n';

}

int main(int argc, char* argv[]){
  
  if(argc < 2){
    cout << "Usage: " << argv[0] << " [roms...]\n";
    return 1;
  }
  
  vector<string> args (argv + 1, argv + argc);
  
  for(auto& i : args){
    print_mapper(i);
  }

}

