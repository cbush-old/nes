#ifndef VIDEO_AUTOSNAPSHOT_H
#define VIDEO_AUTOSNAPSHOT_H

#include "bus.h"

class AutoSnapshotVideoDevice : public IVideoDevice
{
public:
    AutoSnapshotVideoDevice(std::string const &ROM_name, size_t delay_seconds = 3);
    ~AutoSnapshotVideoDevice() {}

public:
    void on_frame() override;
    void put_pixel(uint8_t x, uint8_t y, PaletteIndex i) override;

private:
    Framebuffer _buffer;
    size_t _frame{ 0 }, _delay_seconds{ 3 };
    std::string _ROM_name;
};

#endif
