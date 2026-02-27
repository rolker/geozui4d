#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/BottomDetection.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(BottomDetection)
{
    class_<gz4d::Simrad::BottomDetectionSubscription, bases<gz4d::Producer<gz4d::Simrad::TargetSet> >, std::shared_ptr<gz4d::Simrad::BottomDetectionSubscription>, boost::noncopyable >("BottomDetectionSubscription",no_init);
}
