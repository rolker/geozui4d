// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/Target.h>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

using namespace boost::python;

struct TargetSubscriptionWrap: gz4d::Simrad::TargetSubscription, wrapper<gz4d::Simrad::TargetSubscription>
{
    void AddData(std::shared_ptr<std::vector<unsigned char> > &data)
    {
        this->get_override("AddData")(data);
    }
};

BOOST_PYTHON_MODULE(Target)
{
    class_<gz4d::Simrad::Target>("Target")
            .def_readonly("position", &gz4d::Simrad::Target::position)
            .def_readonly("value", &gz4d::Simrad::Target::value)
            ;

    class_<std::vector<gz4d::Simrad::Target> >("TargetList")
            .def(vector_indexing_suite<std::vector<gz4d::Simrad::Target> >())
            ;

    class_<gz4d::Simrad::TargetSet>("TargetSet")
            .def_readonly("time", &gz4d::Simrad::TargetSet::time)
            .def_readonly("targets", &gz4d::Simrad::TargetSet::targets)
            ;

    gz4d::WrapProducer<gz4d::Simrad::TargetSet>("TargetSetProducer");
    gz4d::WrapThreadedQueue<gz4d::Simrad::TargetSet>("TargetSetThreadedQueue");

    class_<TargetSubscriptionWrap,
    bases< gz4d::Producer<gz4d::Simrad::TargetSet> >,
      std::shared_ptr<TargetSubscriptionWrap>,
       boost::noncopyable>("TargetSubscription",no_init)
               .def("AddData", pure_virtual(&gz4d::Simrad::TargetSubscription::AddData))
            ;
}
