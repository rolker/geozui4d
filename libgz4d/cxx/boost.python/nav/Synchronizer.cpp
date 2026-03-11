// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/nav/Synchronizer.h>
#include <gz4d/thread/Consumer.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Synchronizer)
{
    class_<gz4d::nav::Synchronizer, bases<gz4d::Producer<gz4d::nav::Fix::Ptr> >,  std::shared_ptr<gz4d::nav::Synchronizer>, boost::noncopyable>("Synchronizer",init<gz4d::nav::Track::Ptr &, optional<gz4d::time::Duration , gz4d::time::Duration> >())
            ;

    gz4d::WrapConsumer<gz4d::nav::Synchronizer, gz4d::nav::Fix::Ptr>("SynchronizerConsumer");
}
