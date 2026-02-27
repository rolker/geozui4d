#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/ParameterManager.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(ParameterManager)
{
    class_<gz4d::Simrad::ParameterManager, bases<>, std::shared_ptr<gz4d::Simrad::ParameterManager>, boost::noncopyable >("ParameterManager", no_init)
            ;
}
