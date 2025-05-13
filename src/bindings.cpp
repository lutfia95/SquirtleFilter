#include <pybind11/pybind11.h>
#include "../include/SquirtleFilterWrapper.hpp"

namespace py = pybind11;

PYBIND11_MODULE(squirtlefilter, m) {
    py::class_<SquirtleFilterWrapper>(m, "SquirtleFilter")
        .def(py::init<uint64_t, double, uint8_t>(), 
             py::arg("expected_items"), py::arg("false_positive_rate"), py::arg("hash_functions"))
        .def("insert", &SquirtleFilterWrapper::insert)
        .def("contains", &SquirtleFilterWrapper::contains);
}
