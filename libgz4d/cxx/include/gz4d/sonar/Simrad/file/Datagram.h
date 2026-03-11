// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_DATAGRAM_H
#define GZ4D_SIMRAD_DATAGRAM_H

#include <gz4d/sonar/Simrad.h>
#include <vector>
#include <memory>

namespace gz4d
{
    namespace Simrad
    {

        class GZ4DEXPORT DatagramWrapperBase
        {
            protected:
                std::shared_ptr<std::vector<char> > data;
            public:
                DatagramWrapperBase(){}
                DatagramWrapperBase(std::shared_ptr<std::vector<char> > &d)
                :data(d)
                {
                }
                
                DatagramWrapperBase const &operator=(DatagramWrapperBase const &o)
                {
                    data = o.data;
                    return *this;
                }
                
                operator bool() const
                {
                    return bool(data);
                }
                
        };

        template <typename T> class DatagramWrapper: public DatagramWrapperBase
        {
            public:
                DatagramWrapper(){}
                DatagramWrapper(DatagramWrapperBase const &d):DatagramWrapperBase(d){}
                using DatagramWrapperBase::operator=;
                T* operator->() const
                {
                    return reinterpret_cast<T*>(&data->front());
                }
        };
    }
}

#endif
