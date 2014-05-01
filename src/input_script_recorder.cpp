#include "input_script_recorder.h"
#include <map>
#include <sstream>

ScriptRecorder::ScriptRecorder(IController& controller)
  : port(controller)
  {}

ScriptRecorder::~ScriptRecorder() {}

static const char lookup[] { "><v^.,ba" };

void ScriptRecorder::tick() {

  std::stringstream ss;

  for (int i = 0; i < 8; ++i) {

    IController::Button button = (IController::Button)i;

    auto const& it = pressed.find(button);

    IController::ButtonState state = port.get_button_state(button);

    if (it == pressed.end()) {

      if (state == IController::BUTTON_ON) {
        ss << "+" << lookup[button];
        pressed.insert(button);
      }

    } else {

      if (state == IController::BUTTON_OFF) {
        ss << "-" << lookup[button];
        pressed.erase(it);
      }

    }
  }

  if (ss.str().size() == 0) {
    ++wait;
  } else {
    if (wait > 0) {
      --wait;
      out << "w" << wait;
      std::cout << "w" << wait;
      wait = 0;
    }
    std::cout << ss.str() << "\n";
    out << ss.str();
    out.flush();

  }

}