#include "../../boost_python.h"
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <gz4d/sonar/Simrad/net/Parameter.h>

using namespace boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getDouble_overloads,Get,1,2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getString_overloads,Get,1,2)

BOOST_PYTHON_MODULE(Parameter)
{
    class_<gz4d::Simrad::Parameter, bases<gz4d::Producer<gz4d::Time> >, std::shared_ptr<gz4d::Simrad::Parameter>, boost::noncopyable >("Parameter", no_init)
            .def("Get", &gz4d::Simrad::Parameter::Get<double>, getDouble_overloads())
            .def("Get", &gz4d::Simrad::Parameter::Get<std::string>, getString_overloads())
            .def("GetLatestTime", &gz4d::Simrad::Parameter::GetLatestTime)
            ;

    class_<std::map<std::string,gz4d::Simrad::Parameter::Ptr> >("ParameterMap")
            .def(map_indexing_suite<std::map<std::string,gz4d::Simrad::Parameter::Ptr> >())
            ;
}
