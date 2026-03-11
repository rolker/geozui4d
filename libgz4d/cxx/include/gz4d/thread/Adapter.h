// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_ADAPTER_H
#define GZ4D_ADAPTER_H

#include <gz4d/gz4d.h>
#include <gz4d/util/thread/Consumer.h>
#include <gz4d/util/thread/Producer.h>

namespace gz4d
{
    template <typename inType, typename outType>
    class Adapter: public Consumer<inType>, public Producer<outType>
    {
        Adapter(Adapter const &a){}
        public:
            Adapter(){}
    };
}

#endif
