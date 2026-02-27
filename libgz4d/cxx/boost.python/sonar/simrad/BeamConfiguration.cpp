#include "../../boost_python.h"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <gz4d/sonar/Simrad/net/BeamConfiguration.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(BeamConfiguration)
{
    class_<gz4d::Simrad::BeamConfiguration, bases<>, std::shared_ptr<gz4d::Simrad::BeamConfiguration>, boost::noncopyable >("BeamConfiguration", no_init)
            .def("__str__", &gz4d::Simrad::BeamConfiguration::GetName)
            .def("GetParameters", &gz4d::Simrad::BeamConfiguration::GetParameters, return_value_policy<copy_const_reference>())
            ;

    class_< std::vector<gz4d::Simrad::BeamConfiguration::Ptr > >("ChannelList")
            .def(vector_indexing_suite<std::vector<gz4d::Simrad::BeamConfiguration::Ptr >, true >())
            ;
}
