#ifndef APU_H
#define APU_H

#include <iostream>
#include <vector>
#include <cstdint>
#include "bit.h"

class APU {
  
  friend class CPU;
  
  private:
    void write(uint8_t, uint8_t);
    uint8_t read();

  private:
    static const uint8_t LengthCounters[32];
    static const uint16_t NoisePeriods[16];
    static const uint16_t DMCperiods[16];

    bool FiveCycleDivider = false, IRQdisable = true;
    std::vector<bool> ChannelsEnabled;
    bool PeriodicIRQ = false, DMC_IRQ = false;

    struct Channel {
      inline bool count(int& v, int reset){ 
        return --v < 0 ? (v=reset), true : false; 
      }
      
      int length_counter, linear_counter, address, envelope,
        sweep_delay, env_delay, wave_counter, hold, phase, level;

      union {
        uint32_t value;
        
        bit< 0, 8> reg0;
        bit< 8, 8> reg1;
        bit<16, 8> reg2;
        bit<24, 8> reg3;
        
        // Byte 0
        bit< 0, 4> env_period;
        bit< 0, 4> fixed_volume;
        bit< 4, 1> constant_volume;
        bit< 5, 1> loop_envelope;
        bit< 5, 1> length_counter_control;
        bit< 6, 2> duty;
        // triangle
        bit< 0, 7> linear_counter_load;
        bit< 7, 1> linear_counter_control;
        
        // Byte 1
        bit< 8, 3> sweep_shift_count;
        bit<11, 1> sweep_negative;
        bit<12, 3> sweep_period;
        bit<15, 1> sweep_enabled;
        bit< 8, 8> PCM_length;
        
        // Byte 2
        bit<16, 8> timer_low;
        // noise
        bit<16, 4> noise_period;
        bit<23, 1> noise_loop;

        bit<16, 11> wavelength;        
        // Byte 3
        bit<24, 3> timer_high;
        bit<27, 5> length_counter_load;
        bit<30, 1> loop_enabled;
        bit<31, 1> IRQ_enabled;
        
      };
      
      template<unsigned c>
      int tick(APU& apu){

        if(!apu.ChannelsEnabled[c]) 
          return c==4 ? 64 : 8;
          
        int wl = (wavelength + 1) * (c >= 2 ? 1 : 2);
        
        if(c == 3)
          wl = NoisePeriods[noise_period];
          
        int volume = length_counter ? constant_volume ? fixed_volume : envelope : 0;
        
        // Sample may change at wavelen intervals.
        auto& S = level;
        
        if(!count(wave_counter, wl)) 
          return S;
          
        switch(c)
        {
          default:// Square wave. With four different 8-step binary waveforms (32 bits of data total).
            if(wl < 8) return S = 8;
            return S = (0xF33C0C04u & (1u << (++phase % 8 + duty * 8))) ? volume : 0;

          case 2: // Triangle wave
            if(length_counter && linear_counter && wl >= 3) ++phase;
            return S = (phase & 15) ^ ((phase & 16) ? 15 : 0);

          case 3: // Noise: Linear feedback shift register
            if(!hold) hold = 1;
            hold = (hold >> 1)| (((hold ^ (hold >> (noise_loop ? 6 : 1))) & 1) << 14);
            return S = (hold & 1) ? 0 : volume;
          /*
          case 4: // Delta modulation channel (DMC)
         
            // hold = 8 bit value, phase = number of bits buffered
            if(phase == 0) // Nothing in sample buffer?
            {
              if(!length_counter && loop_enabled) // Loop?
              {
                length_counter = PCM_length * 16 + 1;
                address = (reg0 | 0x300) << 6;
              }
              if(length_counter > 0) // Load next 8 bits if available
              {
                // Note: Re-entrant! But not recursive, because even
                // the shortest wave length is greater than the read time.
                // TODO: proper clock
                if(wavelength > 20)
                  for(unsigned t=0; t<3; ++t) 
                    bus::cpu().read(uint16_t(address) | 0x8000); // timing
                hold  = bus::cpu().read(uint16_t(address++) | 0x8000); // Fetch byte
                phase = 8;
                --length_counter;
              }
              else // Otherwise, disable channel or issue IRQ
                ChannelsEnabled[4] = IRQ_enabled && (CPU::intr = DMC_IRQ = true);
            }
            if(ch.phase != 0) // Update the signal if sample buffer nonempty
            {
              int v = ch.linear_counter;
              if(ch.hold & (0x80 >> --ch.phase)) v += 2; else v -= 2;
              if(v >= 0 && v <= 0x7F) ch.linear_counter = v;
            }
            return S = ch.linear_counter;
          */
        }
      }
    } channel[5];
    
    struct { short lo, hi; } hz240counter { 0, 0 };
    
  public:
    APU();
    ~APU();
    
  public:
    void tick();
    
  
};

#endif
