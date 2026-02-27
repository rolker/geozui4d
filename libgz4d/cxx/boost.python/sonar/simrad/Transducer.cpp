#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/Transducer.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Transducer)
{
    class_<gz4d::Simrad::Transducer, bases<>, std::shared_ptr<gz4d::Simrad::Transducer> >("Transducer",no_init)
            .def("GetChannels", &gz4d::Simrad::Transducer::GetChannels)
            ;

}
