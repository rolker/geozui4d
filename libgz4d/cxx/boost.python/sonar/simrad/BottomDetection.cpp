// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/BottomDetection.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(BottomDetection)
{
    class_<gz4d::Simrad::BottomDetectionSubscription, bases<gz4d::Producer<gz4d::Simrad::TargetSet> >, std::shared_ptr<gz4d::Simrad::BottomDetectionSubscription>, boost::noncopyable >("BottomDetectionSubscription",no_init);
}
