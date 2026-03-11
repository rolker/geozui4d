// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_BINARY_WRAPPER_H
#define GZ4D_DATA_BINARY_WRAPPER_H

#include <gz4d/data/binary/Packet.h>

namespace gz4d
{
    namespace binary
    {

        class WrapperBase
        {
            protected:
                std::shared_ptr<std::vector<char> > _data;
                void Resize(std::size_t s)
                {
                    _data->resize(s);
                    reinterpret_cast<Packet *>(&_data->front())->size = s;
                }
            public:
                WrapperBase(std::size_t size):_data(new std::vector<char>(size)){}
                WrapperBase(std::shared_ptr<std::vector<char> > &d):_data(d){}
                WrapperBase const &operator=(WrapperBase const &o)
                {
                    _data = o._data;
                    return *this;
                }
                operator bool() const {return bool(_data);}
                template <typename T> void SendVia(T &s) {s.Send(*_data);}
                void StreamTo(std::basic_ostream< char > &strm) {strm.write(&_data->front(),_data->size());}
        };

        template <typename T> class Wrapper: public WrapperBase
        {
            public:
                Wrapper()
                :WrapperBase(sizeof(T))
                {
                    new(&_data->front()) T;
                }
                Wrapper(WrapperBase const &d):WrapperBase(d){}
                using WrapperBase::operator=;
                T* operator->() const
                {
                    return reinterpret_cast<T*>(&_data->front());
                }
                T &operator*() const
                {
                    return *reinterpret_cast<T*>(&_data->front());
                }

                void Resize()
                {
                    WrapperBase::Resize(sizeof(T)+sizeof(typename T::value_type)*((*this)->getArrayCount()));
                }
        };

    }
}

#endif
