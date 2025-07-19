#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <cstdint>

class Controller {
public:
    Controller();
    ~Controller();

    // Controller state
    // A, B, Select, Start, Up, Down, Left, Right
    uint8_t buttons = 0x00;

    uint8_t read();
    void strobe(uint8_t data);

private:
    uint8_t snapshot = 0x00;
    bool strobe_mode = false;
};

#endif // CONTROLLER_HPP
