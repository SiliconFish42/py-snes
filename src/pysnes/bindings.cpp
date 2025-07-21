#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "snes.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pysnes_cpp, m) {
    py::class_<SNES>(m, "SNES")
        .def(py::init<>(), "Create a new SNES emulator instance.")
        .def("insert_cartridge", &SNES::insert_cartridge, py::arg("rom_path"), "Insert a ROM cartridge by file path.")
        .def("power_on", &SNES::power_on, "Power on the SNES (reset CPU and PPU).")
        .def("reset", &SNES::reset, "Reset the SNES (CPU, PPU, Cartridge, Bus).")
        .def("step", &SNES::step, "Execute one CPU instruction.")
        .def("get_screen", [](SNES &snes) {
            auto &screen = snes.get_screen();
            constexpr ssize_t height = 224; // PPU::kScreenHeight
            constexpr ssize_t width = 256;  // PPU::kScreenWidth
            std::vector<ssize_t> shape = {height, width};
            std::vector<ssize_t> strides = {sizeof(uint32_t) * width, sizeof(uint32_t)};
            return py::array_t<uint32_t>(
                py::buffer_info(
                    screen.data(),
                    sizeof(uint32_t),
                    py::format_descriptor<uint32_t>::format(),
                    2,
                    shape,
                    strides
                ),
                py::cast(snes)
            );
        }, "Get the current screen framebuffer as a (224, 256) array of 32-bit ARGB pixels.")
        .def("get_framebuffer_rgb", [](SNES &snes) {
            auto rgb = snes.get_framebuffer_rgb();
            constexpr ssize_t height = 224;
            constexpr ssize_t width = 256;
            constexpr ssize_t channels = 3;
            std::vector<ssize_t> shape = {height, width, channels};
            std::vector<ssize_t> strides = {width * channels, channels, 1};
            return py::array_t<uint8_t>(
                py::buffer_info(
                    rgb.data(),
                    sizeof(uint8_t),
                    py::format_descriptor<uint8_t>::format(),
                    3,
                    shape,
                    strides
                ),
                py::cast(snes)
            );
        }, "Get the framebuffer as a (224, 256, 3) uint8 RGB array.")
        .def("set_controller_state", [](SNES &snes, int controller, uint8_t state) {
            // Controller is 1-based (1 or 2)
            snes.set_controller_state(controller, state);
        }, py::arg("controller"), py::arg("state"), "Set the button state for a controller (1 or 2). Buttons packed as bits.");
}
