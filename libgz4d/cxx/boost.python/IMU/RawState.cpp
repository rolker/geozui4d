#include "../boost_python.h"
#include <gz4d/IMU.h>
#include <gz4d/thread/Producer.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(RawState)
{
    class_<gz4d::IMU::RawState>("RawState")
            .def_readwrite("time", &gz4d::IMU::RawState::time)
            .def_readwrite("count", &gz4d::IMU::RawState::count)
            .def_readwrite("mag", &gz4d::IMU::RawState::mag)
            .def_readwrite("accel", &gz4d::IMU::RawState::accel)
            .def_readwrite("gyro", &gz4d::IMU::RawState::gyro)
            ;

    gz4d::WrapProducer<gz4d::IMU::RawState>("RawStateProducer");
    gz4d::WrapThreadedQueue<gz4d::IMU::RawState>("RawStateThreadedQueue");
}

