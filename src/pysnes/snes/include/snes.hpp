#pragma once
#include <memory>
#include <vector>
#include <string>

class SNES {
  public:
    SNES();
    ~SNES();

    void insert_cartridge(const std::string &rom_path);
    void power_on();
    void reset();
    void step();

    std::vector<uint32_t>& get_screen();
    void set_controller_state(int controller_num, uint8_t state);

  private:
    // This is the PIMPL pattern. All internal components
    // are hidden behind this single pointer.
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};
