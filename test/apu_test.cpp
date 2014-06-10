#include "apu.cpp"

int main(int argc, char* argv[]) {

  uint16_t length = 8;

  Pulse pulse;
  pulse.reg0 = 0;
  pulse.reg1 = 0;
  pulse.reg2 = 0;

  pulse.duty = 3;
  pulse.timer = length;

  for (int i = 0; i < length; ++i) {
    int t = pulse.t;
    std::cout << t << " " << (int)pulse.sample() << "\n";
  }
  std::cout << "\n";

}