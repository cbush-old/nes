#include "input_script.h"

#include <iostream>
#include <map>

bool done = false;

using Button = IController::Button;
using ButtonState = IController::ButtonState;

static const std::map<char, Button> lookup {
  { '>', Button::CONTROL_RIGHT },
  { '<', Button::CONTROL_LEFT },
  { 'v', Button::CONTROL_DOWN },
  { '^', Button::CONTROL_UP },
  { '.', Button::CONTROL_START },
  { ',', Button::CONTROL_SELECT },
  { 'b', Button::CONTROL_B },
  { 'a', Button::CONTROL_A },
};

ScriptInputDevice::ScriptInputDevice(IController& controller, std::istream& source)
  : port(controller)
  , source(source)
  {}

void ScriptInputDevice::tick() {

  if (done) {
    return;
  }

  if (wait > 0) {
    --wait;
    return;
  }

  char c, mod = 0;

  while (source.good()) {

    source >> c;

    if (c == 'w') {

      source >> wait;
      return;

    } else if (c == ' ' 
      || c == '\n'
      || c == '\t'
      || c == '\r') {

      continue;

    } else if (c == '+' || c == '-') {

      mod = c;

    } else if (mod) {

      auto const& i = lookup.find(c);
      if (i != lookup.end()) {
        port.set_button_state(i->second, (mod == '+') * IController::BUTTON_ON);
        return;
      }
    }

  }

  std::cout << "Script: reached end of file or file error\n";
  done = true;

}
