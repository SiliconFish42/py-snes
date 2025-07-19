#include "controller.hpp"

Controller::Controller() {}
Controller::~Controller() = default;

uint8_t Controller::read() {
    uint8_t data = 0x00;
    if (strobe_mode) {
        // The first bit read is the 'A' button
        data = (snapshot & 0x80) > 0;
    } else {
        // Read the next bit
        data = (snapshot & 0x80) > 0;
        snapshot <<= 1;
    }
    return data | 0x40; // The unused bits are often read as 1
}

void Controller::strobe(uint8_t data) {
    if ((data & 1) == 1) {
        strobe_mode = true;
        snapshot = buttons;
    } else {
        strobe_mode = false;
    }
}
