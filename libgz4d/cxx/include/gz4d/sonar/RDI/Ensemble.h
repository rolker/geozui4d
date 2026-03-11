// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SONAR_RDI_ENSEMBLE_H
#define GZ4D_SONAR_RDI_ENSEMBLE_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Sonar.h>
#include <vector>
#include <memory>
//#include <stdint.h>

namespace gz4d
{
    namespace RDI
    {
        class OldEnsemble;
        class Header;
        class DataSection;

        class EnsembleWrapper
        {
            protected:
                std::shared_ptr<std::vector<char> > data;
            public:
                EnsembleWrapper(){}
                EnsembleWrapper(std::shared_ptr<std::vector<char> > &d)
                :data(d)
                {
                }
                
                EnsembleWrapper const &operator=(EnsembleWrapper const &o)
                {
                    data = o.data;
                    return *this;
                }
                
                operator bool() const
                {
                    return bool(data);
                }
                
                OldEnsemble * old() const
                {
                    Header * h = reinterpret_cast<Header*>(&data->front());
                    if(h->IsValid())
                        return NULL;
                    return reinterpret_cast<OldEnsemble*>(&data->front());
                }
                
                Header * GetHeader() const
                {
                    return reinterpret_cast<Header*>(&data->front());
                }
                
                DataSection * GetData(uint8_t i) const
                {
                    return reinterpret_cast<DataSection*>(&((*data)[GetHeader()->Offsets[i]]));
                }
                
                uint16_t GetDataSize(uint8_t i) const
                {
                    if(i < GetDataCount()-1)
                        return GetHeader()->Offsets[i+1]-GetHeader()->Offsets[i];
                    return GetHeader()->EnsembleSize-GetHeader()->Offsets[i];
                }
                
                uint8_t GetDataCount() const
                {
                    return GetHeader()->DataCount;
                }
                
                template<typename T> T * GetData() const
                {
                    T *ret;
                    for(uint8_t i= 0; i < GetDataCount(); ++i)
                    {
                        ret = reinterpret_cast<T*>(GetData(i));
                        if(ret->IsValid())
                            return ret;
                    }
                    return NULL;
                }
                template<typename T> uint16_t GetDataSize() const
                {
                    for(uint8_t i= 0; i < GetDataCount(); ++i)
                        if(reinterpret_cast<T*>(GetData(i))->IsValid())
                            return GetDataSize(i);
                    return 0;
                }
        };
    }
}

#endif
