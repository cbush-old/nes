#include "nes.h"

#include "rom.h"
#include "cpu2.h"
#include "ppu.h"
#include "apu.h"

#include "controller_std.h"
#include "audio_sdl.h"
#include "audio_sox_pipe.h"
#include "video_sdl.h"
#include "input_sdl.h"
#include "input_script.h"
#include "input_script_recorder.h"

#include <iostream>

NES::NES(IROM& rom, std::istream& script)
    : video (new SDLVideoDevice())
    , audio (new SDLAudioDevice())
    , controller {
        new Std_controller(),
        new Std_controller(),
    }
    , input {
        new SDLInputDevice(*controller[0]),
        new ScriptInputDevice(*controller[0], script),
        //new ScriptRecorder(*controller[0]),
    }
    , rom (rom)
    , ppu (new PPU(this, &rom, video))
    , apu (new APU(this, audio))
    , cpu (new CPU(this, apu, ppu, &rom, controller[0], controller[1]))
    {}

NES::~NES() {
    delete apu;
    delete cpu;
    delete ppu;
    delete controller[0];
    delete controller[1];
    delete video;
    delete audio;
    for (auto& i : input) {
        delete i;
    }
}

void NES::pull_NMI() {
    cpu->pull_NMI();
}

void NES::pull_IRQ() {
    cpu->pull_IRQ();
}

void NES::on_frame() {
    for (auto& i : input) {
        i->tick();
    }

    ppu->set_frameskip(((SDLInputDevice*)(input[0]))->get_frameskip());

}

void NES::on_cpu_tick() {
    apu->tick();
    ppu->tick();
    ppu->tick();
    ppu->tick();
}

void NES::run() {
    cpu->run();
}
