#include "../boost_python.h"
#include <gz4d/IMU/Calibration.h>

using namespace boost::python;

gz4d::IMU::State (gz4d::IMU::Calibration::*call1)(gz4d::IMU::State const&) const = &gz4d::IMU::Calibration::operator();
gz4d::IMU::State (gz4d::IMU::Calibration::*call2)(gz4d::IMU::RawState const&) const = &gz4d::IMU::Calibration::operator();

BOOST_PYTHON_MODULE(Calibration)
{
    class_<gz4d::IMU::Calibration>("Calibration")
            .def_readwrite("samplePeriod", &gz4d::IMU::Calibration::samplePeriod)
            .def_readwrite("timeVariance", &gz4d::IMU::Calibration::timeVariance)
            .def_readwrite("mag_center", &gz4d::IMU::Calibration::mag_center)
            .def_readwrite("mag_axis", &gz4d::IMU::Calibration::mag_axis)
            .def_readwrite("accel_neg1", &gz4d::IMU::Calibration::accel_neg1)
            .def_readwrite("accel_zero", &gz4d::IMU::Calibration::accel_zero)
            .def_readwrite("accel_pos1", &gz4d::IMU::Calibration::accel_pos1)
            
            .def("__call__",call1)
            .def("__call__",call2)
            ;
}
