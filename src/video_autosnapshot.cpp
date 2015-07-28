#include "video_autosnapshot.h"
#include "image.h"
#include <stdexcept>
#include <sstream>
#include <string>

extern uint32_t palette_index_to_rgba(PaletteIndex i);

AutoSnapshotVideoDevice::AutoSnapshotVideoDevice(std::string const& ROM_name, size_t delay_seconds /* = 3 */)
  : _delay_seconds(delay_seconds)
{
  auto pos = ROM_name.rfind('/');
  auto pos2 = ROM_name.rfind('.');
  _ROM_name = ROM_name.substr(pos + 1, pos2 - pos - 1);
}

void AutoSnapshotVideoDevice::put_pixel(uint8_t x, uint8_t y, PaletteIndex i) {
  _buffer[y * 256 + (x % 256)] = palette_index_to_rgba(i);
}

void AutoSnapshotVideoDevice::on_frame() {
  if (++_frame > _delay_seconds * 60) {
    std::stringstream ss;
    ss << "screenshots/" << _ROM_name << ".png";
    logi("create %s", ss.str().c_str());
    save_image(ss.str().c_str(), 256, 240, _buffer.data());
    throw std::runtime_error("quit");
  }
}
