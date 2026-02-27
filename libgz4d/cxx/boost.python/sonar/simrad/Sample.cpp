#include "../../boost_python.h"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <gz4d/sonar/Simrad/net/Sample.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Sample)
{
    class_<gz4d::Simrad::SampleSet>("SampleSet")
            .def_readonly("time", &gz4d::Simrad::SampleSet::time)
            .def_readonly("z1", &gz4d::Simrad::SampleSet::z1)
            .def_readonly("sampleInterval", &gz4d::Simrad::SampleSet::sampleInterval)
            .def_readonly("soundSpeed", &gz4d::Simrad::SampleSet::soundSpeed)
            .def_readonly("samples", &gz4d::Simrad::SampleSet::samples)
            ;

    class_<std::vector<int16_t> >("Samples")
            .def(vector_indexing_suite<std::vector<int16_t> >())
            ;

    gz4d::WrapProducer<gz4d::Simrad::SampleSet>("SampleSetProducer");
    gz4d::WrapThreadedQueue<gz4d::Simrad::SampleSet>("SampleSetThreadedQueue");

    class_<gz4d::Simrad::SampleSubscription, bases<gz4d::Producer<gz4d::Simrad::SampleSet> >, std::shared_ptr<gz4d::Simrad::SampleSubscription>, boost::noncopyable >("SampleSubscription",no_init);

}
