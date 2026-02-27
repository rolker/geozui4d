#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/Subscription.h>

using namespace boost::python;

struct SubscriptionWrap: gz4d::Simrad::Subscription, wrapper<gz4d::Simrad::Subscription>
{
    void AddData(std::shared_ptr<std::vector<unsigned char> > &data)
    {
        this->get_override("AddData")(data);
    }
};

BOOST_PYTHON_MODULE(Subscription)
{
    class_<SubscriptionWrap, bases<>, std::shared_ptr<SubscriptionWrap>, boost::noncopyable>("Subscription",no_init)
            .def("AddData", pure_virtual(&gz4d::Simrad::Subscription::AddData))
            ;

}
