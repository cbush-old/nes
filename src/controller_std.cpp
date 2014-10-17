#include "controller_std.h"

IController::ButtonState Std_controller::read() {
  std::lock_guard<std::mutex> lock(_mutex);
  if(button_index == -1) {
    return 1;
  }
  return button_state[button_index--];
}

void Std_controller::strobe() {
  button_index = 7;
}

void Std_controller::set_button_state(IController::Button button, IController::ButtonState state) {
  std::lock_guard<std::mutex> lock(_mutex);
  button_state[button] = state;
}

IController::ButtonState Std_controller::get_button_state(IController::Button button) {
  std::lock_guard<std::mutex> lock(_mutex);
  return button_state[button];
}
