#include "rom.h"

#include <fstream>
#include <unordered_map>

using std::ifstream;
using std::cout;
using std::setw;
using std::hex;
using std::vector;
using std::thread;
using std::string;
using std::exception;
using std::runtime_error;

static const unsigned VROM_Granularity = 0x400, VROM_Pages = 0x2000 / VROM_Granularity;
static const unsigned ROM_Granularity = 0x2000, ROM_Pages = 0x10000 / ROM_Granularity;

template<unsigned npages, unsigned granu>
static void set_pages(
  std::vector<uint8_t*>& b,
  std::vector<uint8_t>& r,
  unsigned size, 
  unsigned baseaddr, 
  unsigned index
){
  auto rs = r.size();
  for(unsigned v = rs + index * size, p = baseaddr / granu; 
    p < (baseaddr + size) / granu && p < npages; 
    ++p, v += granu)
    b[p] = &r[v%rs];
}

static const auto& set_ROM = set_pages<ROM_Pages, ROM_Granularity>;
static const auto& set_VROM = set_pages<VROM_Pages, VROM_Granularity>;

static const std::unordered_map<uint8_t, std::function<void(ROM&, uint8_t, uint16_t)>> mapper {
  { 0, [](ROM& this_, uint8_t value, uint16_t addr){}},
  { 2, [](ROM& this_, uint8_t value, uint16_t addr){
    set_ROM(this_.bank, this_.rom, 0x4000, 0x8000, value);
  }},
  { 7, [](ROM& this_, uint8_t value, uint16_t addr){    
    set_ROM(this_.bank, this_.rom, 0x8000, 0x8000, value&7);
    this_.nt[0] = this_.nt[1] = this_.nt[2] = this_.nt[3] = this_.nram.data() + 0x400 * ((value>>4)&1);
  }},
};

uint8_t& ROM::operator[](uint16_t addr){
  if((addr >> 13) == 3)
    return pram[addr & 0x1FFF];
  return bank[(addr/ROM_Granularity)%ROM_Pages][addr%ROM_Granularity];
}

ROM::ROM(std::string const& src):
  nram (0x800), 
  vram (0x2000),
  pram (0x2000),
  nt { nram.data(), nram.data() + 0x400, nram.data(), nram.data() + 0x400 },
  bank (ROM_Pages),
  vbank (VROM_Pages)
{
 
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
  
  std::cout << "Mapper " << mapper_id << '\n';
  
  auto i = mapper.find(mapper_id);
  
  if(i == mapper.end())
    throw runtime_error ("Unsupported mapper");
  
  writef = i->second;

  if(prg_rom_size) rom.resize(prg_rom_size * 0x4000);
  if(chr_rom_size) vram.resize(chr_rom_size * 0x2000);

  file.read((char*)rom.data(), prg_rom_size * 0x4000);
  file.read((char*)vram.data(), chr_rom_size * 0x2000);

  set_VROM(vbank, vram, 0x2000, 0, 0);
  
  for(unsigned v = 0; v < 4; ++v)
    set_ROM(bank, rom, 0x4000, v * 0x4000, v==3 ? -1 : 0);
  
}

