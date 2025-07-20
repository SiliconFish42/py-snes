#pragma once
#include <cstdint>

class Controller {
  public:
    Controller();
    ~Controller();

    uint8_t read();
    void write(uint8_t data);

    void reset();

    uint8_t buttons = 0x00;
  private:
    uint8_t snapshot = 0x00;
};
