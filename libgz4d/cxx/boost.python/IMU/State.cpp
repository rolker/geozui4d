#include "../boost_python.h"
#include <gz4d/IMU.h>
#include <gz4d/thread/Producer.h>


using namespace boost::python;

BOOST_PYTHON_MODULE(State)
{
    class_<gz4d::IMU::State>("State")
            .def_readwrite("time", &gz4d::IMU::State::time)
            .def_readwrite("mag", &gz4d::IMU::State::mag)
            .def_readwrite("accel", &gz4d::IMU::State::accel)
            .def_readwrite("gyro", &gz4d::IMU::State::gyro)
            ;
    
    gz4d::WrapProducer<gz4d::IMU::State>("StateProducer");
    gz4d::WrapThreadedQueue<gz4d::IMU::State>("StateThreadedQueue");

}
