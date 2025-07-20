#include "controller.hpp"

Controller::Controller() {}
Controller::~Controller() {}

uint8_t Controller::read() {
    // On a read, we return the least significant bit of the latched state
    // and then shift the bits for the next read.
    uint8_t data = (snapshot & 0x80) > 0;
    snapshot <<= 1;
    return data;
}

void Controller::write(uint8_t data) {
    // Writing to the controller port latches the current button state
    if (data & 1) {
        snapshot = buttons;
    }
}

void Controller::reset() {}