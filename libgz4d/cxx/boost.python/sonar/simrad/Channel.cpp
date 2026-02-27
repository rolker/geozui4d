#include "../../boost_python.h"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <gz4d/sonar/Simrad/net/Channel.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Channel)
{
    class_<gz4d::Simrad::net::Channel, bases<>, std::shared_ptr<gz4d::Simrad::net::Channel>, boost::noncopyable >("Channel", no_init)
            .def("__str__", &gz4d::Simrad::net::Channel::GetName)
            .def("GetBottomDetection", &gz4d::Simrad::net::Channel::GetBottomDetection,return_value_policy<copy_non_const_reference>())
            .def("GetTargetDetection", &gz4d::Simrad::net::Channel::GetTargetDetection,return_value_policy<copy_non_const_reference>())
            .def("GetSamples", &gz4d::Simrad::net::Channel::GetSamples,return_value_policy<copy_non_const_reference>())
            .def("UpdateSubscription", &gz4d::Simrad::net::Channel::UpdateSubscription)
            .def("GetParameters", &gz4d::Simrad::net::Channel::GetParameters, return_value_policy<copy_const_reference>())
            ;

    class_< std::vector<std::shared_ptr<gz4d::Simrad::net::Channel> > >("ChannelList")
            .def(vector_indexing_suite<std::vector<std::shared_ptr<gz4d::Simrad::net::Channel> >, true >())
            ;
}
