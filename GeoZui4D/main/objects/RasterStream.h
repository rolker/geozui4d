// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef RASTERSTREAM_H
#define RASTERSTREAM_H

#include <gz4d/TimeSequence.h>
#include <gz4d/util/thread/Adapter.h>
#include <gz4d/util/SocketProducer.h>

template <typename Type>
class RasterStream: public gz4d::TimeSequence<typename Type::Ptr>, public gz4d::Consumer<typename gz4d::TimeSequence<typename Type::Ptr>::ValueType >
{
    public:
        typedef typename gz4d::TimeSequence<typename Type::Ptr>::ValueType ValueType;
        class Adapter: public gz4d::Adapter<typename std::shared_ptr<gz4d::Socket::Packet>,typename gz4d::TimeSequence<typename Type::Ptr >::ValueType >
        {
            public:
                ~Adapter(){}
            private:
                void operator()(){gz4d::Consumer<typename std::shared_ptr<gz4d::Socket::Packet> >::operator()();}
                void Consume(std::shared_ptr<gz4d::Socket::Packet> &p);
                typename Type::Ptr raster;
                gz4d::Time time;
        };

        void Start(gz4d::Socket const &s);

    private:
        void Consume(ValueType &value);
        Adapter adapter;
        gz4d::SocketProducer producer;
};

#endif
