// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_NETCDF_VAR_H
#define GZ4D_NETCDF_VAR_H

#include <gz4d/netcdf/netcdf.h>
#include <gz4d/netcdf/Dim.h>
#include <gz4d/netcdf/Att.h>
#include <vector>
#include <string>

namespace gz4d
{
    namespace netcdf
    {
        class File;

        class Var
        {
            netCDF::NcVar var;
            std::vector<size_t> cursor;

            bool IsValid()
            {
                return !var.isNull();
            }
            void CheckValid()
            {
                if (!IsValid()) throw Exception("Invalid variable");
            }

            friend class File;
            Var(netCDF::NcVar v):var(v)
            {
                if (!var.isNull())
                    cursor.resize(var.getDimCount(), 0);
            }
            public:
                Var(){}

                operator bool(){return IsValid();}

                std::string Name() {CheckValid(); return var.getName();}

                template <typename T> bool Type();
                long NumVals()
                {
                    CheckValid();
                    std::vector<netCDF::NcDim> dims = var.getDims();
                    long total = 1;
                    for (size_t i = 0; i < dims.size(); ++i)
                        total *= dims[i].getSize();
                    return total;
                }

                int NumDims() {CheckValid(); return var.getDimCount();}
                Dim GetDim(int n) {CheckValid(); return Dim(var.getDim(n));}
                std::vector<long> Edges()
                {
                    CheckValid();
                    std::vector<netCDF::NcDim> dims = var.getDims();
                    std::vector<long> ret;
                    for (size_t i = 0; i < dims.size(); ++i)
                        ret.push_back(dims[i].getSize());
                    return ret;
                }

                int NumAtts() {CheckValid(); return var.getAttCount();}
                Att GetAtt(std::string const &name) {CheckValid(); return Att(var.getAtt(name));}
                Att GetAtt(int n)
                {
                    CheckValid();
                    std::map<std::string, netCDF::NcVarAtt> atts = var.getAtts();
                    int idx = 0;
                    for (auto it = atts.begin(); it != atts.end(); ++it, ++idx)
                    {
                        if (idx == n) return Att(it->second);
                    }
                    throw Exception("Attribute index out of range");
                }

                template <typename T> void AddAtt(std::string const &name, T value)
                {
                    // Not directly supported in same way; use putAtt on the var
                    CheckValid();
                }

                template <typename T> void Get(T* vals, long c0=0, long c1=0, long c2=0, long c3=0, long c4=0)
                {
                    CheckValid();
                    int ndims = var.getDimCount();
                    std::vector<size_t> count(ndims, 1);
                    long counts[] = {c0, c1, c2, c3, c4};
                    for (int i = 0; i < ndims && i < 5; ++i)
                    {
                        if (counts[i] > 0) count[i] = counts[i];
                    }
                    try
                    {
                        var.getVar(cursor, count, vals);
                    }
                    catch (netCDF::exceptions::NcException& e)
                    {
                        throw Exception(std::string("Get failed: ") + e.what());
                    }
                }

                template <typename T> void Get(std::vector<T> &vals, long c0, long c1=0, long c2=0, long c3=0, long c4=0)
                {
                    Get(&vals.front(), c0, c1, c2, c3, c4);
                }
                template <typename T> void Get(std::vector<T> &vals) {Get(vals, vals.size());}
                template <typename T> void Get(T* vals, std::vector<long> const &counts)
                {
                    CheckValid();
                    std::vector<size_t> count(counts.begin(), counts.end());
                    try
                    {
                        var.getVar(cursor, count, vals);
                    }
                    catch (netCDF::exceptions::NcException& e)
                    {
                        throw Exception(std::string("Get failed: ") + e.what());
                    }
                }
                template <typename T> void Get(std::vector<T> &vals, std::vector<long> const &counts)
                {
                    Get(&vals.front(), counts);
                }

                void SetCur(long c0 = -1, long c1=-1, long c2=-1, long c3=-1, long c4=-1)
                {
                    CheckValid();
                    int ndims = var.getDimCount();
                    cursor.resize(ndims, 0);
                    long vals[] = {c0, c1, c2, c3, c4};
                    for (int i = 0; i < ndims && i < 5; ++i)
                    {
                        if (vals[i] >= 0) cursor[i] = vals[i];
                        else cursor[i] = 0;
                    }
                }
                void SetCur(std::vector<long> &cur)
                {
                    CheckValid();
                    cursor.assign(cur.begin(), cur.end());
                }

                void Sync() {CheckValid(); /* no-op in netcdf-c++4, sync is on File */}
        };

        template<> inline bool Var::Type<int8_t>() {CheckValid(); return var.getType() == netCDF::ncByte;}
        template<> inline bool Var::Type<char>() {CheckValid(); return var.getType() == netCDF::ncChar;}
        template<> inline bool Var::Type<int16_t>() {CheckValid(); return var.getType() == netCDF::ncShort;}
        template<> inline bool Var::Type<int32_t>() {CheckValid(); return var.getType() == netCDF::ncInt;}
        template<> inline bool Var::Type<float>() {CheckValid(); return var.getType() == netCDF::ncFloat;}
        template<> inline bool Var::Type<double>() {CheckValid(); return var.getType() == netCDF::ncDouble;}
    }
}

#endif
