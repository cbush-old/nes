#include "apu.cpp"

int main(int argc, char* argv[]) {

  uint16_t length = 2048;

  Noise noise;
  noise.reg0 = 0;
  noise.reg1 = 0;
  noise.reg2 = 0;

  for (int i = 0; i < length; ++i) {
    noise.update();
    std::cout << (int)noise.sample();
  }
  std::cout << "\n";

}