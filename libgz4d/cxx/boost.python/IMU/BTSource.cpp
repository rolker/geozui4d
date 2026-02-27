#include "../boost_python.h"
#include <gz4d/IMU/BTSource.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(BTSource)
{
    class_<gz4d::IMU::BTSource, bases<gz4d::Producer<gz4d::IMU::RawState> >, std::shared_ptr<gz4d::IMU::BTSource>, boost::noncopyable>("BTSource")
            .def("Open", &gz4d::IMU::BTSource::Open)
            ;

}
