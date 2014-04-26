#ifndef CONTROLLER_STD_H
#define CONTROLLER_STD_H

#include "bus.h"

class Std_controller : public IController {
    public:
        ButtonState read();
        void strobe();
        void set_button_state(Button button, ButtonState state);

    private:
        int button_index { 0 };
        ButtonState button_state[8];

};

#endif