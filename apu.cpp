#include "apu.h"

#include <unistd.h>

// bisqwit's APU hacked in here


const uint8_t APU::LengthCounters[32] { 
  10,254,20, 2,40, 4,80, 6,160, 8,60,10,14,12,26,14,
  12, 16,24,18,48,20,96,22,192,24,72,26,16,28,32,30 };
const uint16_t APU::NoisePeriods[16] { 2,4,8,16,32,48,64,80,101,127,190,254,381,508,1017,2034 };
const uint16_t APU::DMCperiods[16] { 428,380,340,320,286,254,226,214,190,160,142,128,106,84,72,54 };


void APU::write(uint8_t value, uint8_t index){
  Channel& ch = channel[(index/4)%5];
  switch(index < 0x10 ? index&3 : index)
  {
    case 0: 
      if(ch.linear_counter_control) 
        ch.linear_counter = value&0x7F; 
      ch.reg0 = value; 
      break;
    case 1: 
      ch.reg1 = value; 
      ch.sweep_delay = ch.sweep_period;
      break;
    case 2: 
      ch.reg2 = value; 
      break;
    case 3:
      ch.reg3 = value;
      if(ChannelsEnabled[index/4])
        ch.length_counter = LengthCounters[ch.length_counter_load];
      ch.linear_counter = ch.linear_counter_load;
      ch.env_delay = ch.env_period;
      ch.envelope = 15;
      if(index < 8) 
        ch.phase = 0;
      break;
    case 0x10: 
      ch.reg3 = value; 
      ch.wavelength = DMCperiods[value&0x0F]; 
      break;
    case 0x12: 
      ch.reg0 = value; 
      ch.address = (ch.reg0 | 0x300) << 6; 
      break;
    
    case 0x13: 
      ch.reg1 = value;
      ch.length_counter = ch.PCM_length * 16 + 1; 
      break; // sample length
    case 0x11: 
      ch.linear_counter = value & 0x7F; 
      break; // dac value
    case 0x15:
      for(unsigned c=0; c<5; ++c)
        ChannelsEnabled[c] = value & (1 << c);
      for(unsigned c=0; c<5; ++c)
        if(!ChannelsEnabled[c])
          channel[c].length_counter = 0;
        else if(c == 4 && channel[c].length_counter == 0)
          channel[c].length_counter = ch.PCM_length * 16 + 1;
      break;
    case 0x17:
      IRQdisable = value & 0x40;
      FiveCycleDivider = value & 0x80;
      hz240counter = { 0,0 };
      if(IRQdisable) 
        PeriodicIRQ = DMC_IRQ = false;
  }
}


APU::APU():ChannelsEnabled(5, false){}

APU::~APU(){}


template<int C>
void channel_tick(){

}

void APU::tick(){
  // Divide CPU clock by 7457.5 to get a 240 Hz, which controls certain events.
  if((hz240counter.lo += 2) >= 14915)
  {
    
    hz240counter.lo -= 14915;
    
    if(++hz240counter.hi >= 4 + FiveCycleDivider) 
      hz240counter.hi = 0;

    // 60 Hz interval: IRQ. IRQ is not invoked in five-cycle mode (48 Hz).
    //if(!IRQdisable && !FiveCycleDivider && hz240counter.hi==0)
    //  CPU::intr = PeriodicIRQ = true;

    // Some events are invoked at 96 Hz or 120 Hz rate. Others, 192 Hz or 240 Hz.
    bool 
      HalfTick = (hz240counter.hi&5)==1, 
      FullTick = hz240counter.hi < 4;
      
    for(unsigned c=0; c<4; ++c)
    {
      Channel& ch = channel[c];
      int wl = ch.wavelength;

      // Length tick (all channels except DMC, but different disable bit for triangle wave)
      if(HalfTick && ch.length_counter
      && !(c==2 ? ch.linear_counter_control : ch.length_counter_control))
        ch.length_counter -= 1;

      // Sweep tick (square waves only)
      if(HalfTick && c < 2 && ch.count(ch.sweep_delay, ch.sweep_period))
        if(wl >= 8 && ch.sweep_enabled && ch.sweep_shift_count)
        {
          int 
            s = wl >> ch.sweep_shift_count, 
            d[4] = {s, s, ~s, -s};
          wl += d[ch.sweep_negative * 2 + c];
          if(wl < 0x800) ch.wavelength = wl;
        }

      // Linear tick (triangle wave only)
      if(FullTick && c == 2)
        ch.linear_counter = ch.linear_counter_control ? 
          ch.linear_counter_load : 
          (ch.linear_counter > 0 ? ch.linear_counter - 1 : 0);

      // Envelope tick (square and noise channels)
      if(FullTick && c != 2 && ch.count(ch.env_delay, ch.env_period))
        if(ch.envelope > 0 || ch.loop_envelope)
          ch.envelope = (ch.envelope - 1) & 15;
    }
  }

  // Mix the audio: Get the momentary sample from each channel and mix them.
  #define s(c) channel[c].tick<c==1 ? 0 : c>(*this)
  auto v = [](float m, float n, float d) { return n!=0.f ? m/n : d; };
  short sample = 30000 * (
    v(
      95.88f,  
      (100.f + v(
        8128.f, 
        s(0) + s(1), 
        -100.f)
      ), 
      0.f)
    + v(
      159.79f, 
      (100.f + v(
        1.0, 
        s(2)/8227.f + s(3)/12241.f + s(4)/22638.f,
        -100.f
      )), 
      0.f
    ) - 0.5f
    );
    
  #undef s
  // I cheat here: I did not bother to learn how to use SDL mixer, let alone use it in <5 lines of code,
  // so I simply use a combination of external programs for outputting the audio.
  // Hooray for Unix principles! A/V sync will be ensured in post-process.
  //return; // Disable sound because already device is in use
  //static FILE* fp = popen("resample mr1789800 r48000 | aplay -fdat 2>/dev/null", "w");
  
  static FILE* fp = popen(
    "sox -t raw -c1 -b 16 -e signed-integer -r1789773 - -t raw -c2 - rate 48000 \
| aplay -fdat",
    "w"
  );
  
  fputc(sample, fp);
  fputc(sample/256, fp);
}

uint8_t APU::read(){
  uint8_t res = 0;
  for(unsigned c=0; c<5; ++c) 
    res |= (channel[c].length_counter ? 1 << c : 0);
  
  if(PeriodicIRQ) 
    res |= 0x40; 
  
  PeriodicIRQ = false;

  if(DMC_IRQ)
    res |= 0x80;
  
  DMC_IRQ = false;
  //CPU::intr = false;
  return res;
  
}

