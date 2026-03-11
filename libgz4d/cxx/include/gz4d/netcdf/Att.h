// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_NETCDF_ATT_H
#define GZ4D_NETCDF_ATT_H

#include <gz4d/netcdf/netcdf.h>
#include <ncAtt.h>
#include <ncGroupAtt.h>
#include <ncVarAtt.h>
#include <string>

namespace gz4d
{
    namespace netcdf
    {
        class Var;
        class File;

        class Att
        {
            netCDF::NcGroupAtt groupAtt;
            netCDF::NcVarAtt varAtt;
            bool isGroupAtt;

            bool IsValid() const
            {
                if (isGroupAtt)
                    return !groupAtt.isNull();
                return !varAtt.isNull();
            }

            const netCDF::NcAtt& att() const
            {
                if (isGroupAtt) return groupAtt;
                return varAtt;
            }

            void CheckValid() const {if (!IsValid()) throw Exception("Invalid Attribute");}

            friend class Var;
            friend class File;
            Att(netCDF::NcGroupAtt a):groupAtt(a),isGroupAtt(true){}
            Att(netCDF::NcVarAtt a):varAtt(a),isGroupAtt(false){}

            public:
                Att():isGroupAtt(true){}

                operator bool() const {return IsValid();}

                std::string Name() const {CheckValid(); return att().getName();}
                long NumVals() const {CheckValid(); return att().getAttLength();}

                template <typename T> T As(int n) const;
                template <typename T> bool Type() const;
        };

        template<> inline int8_t Att::As<int8_t>(int n) const
        {
            CheckValid();
            signed char val;
            att().getValues(&val);
            return val;
        }
        template<> inline char Att::As<char>(int n) const
        {
            CheckValid();
            char val;
            att().getValues(&val);
            return val;
        }
        template<> inline int16_t Att::As<int16_t>(int n) const
        {
            CheckValid();
            short val;
            att().getValues(&val);
            return val;
        }
        template<> inline int32_t Att::As<int32_t>(int n) const
        {
            CheckValid();
            int val;
            att().getValues(&val);
            return val;
        }
        template<> inline float Att::As<float>(int n) const
        {
            CheckValid();
            float val;
            att().getValues(&val);
            return val;
        }
        template<> inline double Att::As<double>(int n) const
        {
            CheckValid();
            double val;
            att().getValues(&val);
            return val;
        }
        template<> inline std::string Att::As<std::string>(int n) const
        {
            CheckValid();
            std::string val;
            att().getValues(val);
            return val;
        }

        template<> inline bool Att::Type<int8_t>() const {CheckValid(); return att().getType() == netCDF::ncByte;}
        template<> inline bool Att::Type<char>() const {CheckValid(); return att().getType() == netCDF::ncChar;}
        template<> inline bool Att::Type<int16_t>() const {CheckValid(); return att().getType() == netCDF::ncShort;}
        template<> inline bool Att::Type<int32_t>() const {CheckValid(); return att().getType() == netCDF::ncInt;}
        template<> inline bool Att::Type<float>() const {CheckValid(); return att().getType() == netCDF::ncFloat;}
        template<> inline bool Att::Type<double>() const {CheckValid(); return att().getType() == netCDF::ncDouble;}
    }
}

#endif
