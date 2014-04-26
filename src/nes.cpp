#include "bus.h"
#include "rom.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "io.h"

NES::NES(std::string const& game)
    : video (new SDLVideoDevice())
    , audio (new SDLAudioDevice())
    , controller {
        new GamePad(),
        new GamePad(),
    }
    , input (new SDLInputDevice(*controller[0]))
    , rom (new ROM(game))
    , ppu (new PPU(this, rom, input, video))
    , apu (new APU(this))
    , cpu (new CPU(apu, ppu, rom, controller[0], controller[1]))
    {
        cpu->run();
    }

NES::~NES() {
    delete apu;
    delete cpu;
    delete ppu;
    delete controller[0];
    delete controller[1];
    delete video;
    delete audio;
    delete input;
}

void NES::pull_NMI() {
    cpu->pull_NMI();
}

void NES::pull_IRQ() {
    cpu->pull_IRQ();
}

void NES::reset_IRQ() {
    cpu->reset_IRQ();
}
