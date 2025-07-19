#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "snes.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pysnes_cpp, m) {
    py::class_<SNES>(m, "SNES")
        .def(py::init<>())
        .def("insert_cartridge", &SNES::insert_cartridge)
        .def("power_on", &SNES::power_on)
        .def("reset", &SNES::reset)
        .def("step", &SNES::step)
        .def("get_screen", [](SNES &snes) {
            auto &screen = snes.get_screen();
            return py::array_t<uint32_t>(
                {240, 256},
                {sizeof(uint32_t) * 256, sizeof(uint32_t)},
                screen.data(),
                py::cast(snes)
            );
        })
        .def("set_controller_state", [](SNES &snes, int controller, uint8_t state) {
            snes.set_controller_state(controller, state);
        });
}