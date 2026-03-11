// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "GZincludes.h"
#include"NetCDF.h"
#include<string.h>
#include"Flow.h"
#include <filesystem>
#include"Current.h"
#include <boost/property_tree/xml_parser.hpp>

using namespace std;

extern MainWindow * mainWindow;

REGISTER_OBJECT(NetCDF, "");

NetCDF::NetCDF()
{
}

int NetCDF::Load()
{
    if(!GetFilename().empty())
    {
        ncf = gz4d::netcdf::File(GetFilename());
        if(ncf)
        {
            bpt::ptree doc;
            gz4d::netcdf::Att a = ncf.GetAtt("gzx");
            if(a)
            {
                std::istringstream ss(a.As<std::string>(0));
                read_xml(ss,doc);

                if(doc.empty())
                {
                    cerr << "Error parsing " << a.As<std::string>(0) << endl;
                    return 1;
                }
                bpt::ptree node = doc.get_child("GZ");

                LoadXML(node,GetFilenameAsPath().parent_path().string(),ncf);
                return 0;
            }
        }
    }
    return 1;
}

/// Parses the XML to find out how to interpret the NetCDF file
Object * NetCDF::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object * obj;

    Object::LoadXML(xml,path, ncfile);

    boost::optional<std::string> attribute;
    
    if(!ncfile)
    {
        // first, get the NetCDF filename
        if(attribute = xml.get_optional<std::string>("<xmlattr>.ncfile"))
            ncf = gz4d::netcdf::File((fs::path(path)/(*attribute)).string());
        else
        {
            cerr << "Problems getting the netCDF filename\n";
            return NULL;
        }
    }
    else
        ncf = ncfile;
    if(ncf)
    {
        DumpHeader();
        for(bpt::ptree::const_iterator node = xml.begin(); node != xml.end(); ++node)
        {
            boost::optional<std::string> type = node->second.get_optional<std::string>("<xmlattr>.type");
            if(type)
            {
                obj = mainWindow->getScene3D()->NewObject(*type);
                if(obj && obj->LoadXML(xml,path,ncf))
                {
                    AddObject(obj);
                }
            }
        }
    }
    else
    {
        std::cerr << "Problems opening Netcdf file" << std::endl;
        return NULL;
    }
    return this;
}


void NetCDF::DumpHeader()
{
    std::cout << "*********************************************\n";
    std::cout << "*           NetCDF File Header              *\n";
    std::cout << "*********************************************\n";
    std::cout << "netcdf {" << std::endl;
    std::cout << "dimensions:" << std::endl;
    for(int i = 0; i < ncf.NumDims(); ++i)
    {
        gz4d::netcdf::Dim d = ncf.GetDim(i);
        std::cout << "\t" << d.Name() << " = ";
        if (d.IsUnlimited())
            std::cout << "UNLIMITED ; // (" << d.Size() << " currently)" << std::endl;
        else
            std::cout << d.Size() << " ;" << std::endl;
    }
    std::cout << "variables:" << std::endl;
    for(int i = 0; i < ncf.NumVars(); ++i)
    {
        gz4d::netcdf::Var v = ncf.GetVar(i);
        bool done = false;
        if(!done && v.Type<int8_t>())
        {
            std::cout << "\tbyte ";
            done = true;
        }
        if(!done && v.Type<char>())
        {
            std::cout << "\tchar ";
            done = true;
        }
        if(!done && v.Type<int16_t>())
        {
            std::cout << "\tshort ";
            done = true;
        }
        if(!done && v.Type<int32_t>())
        {
            std::cout << "\tint ";
            done = true;
        }
        if(!done && v.Type<float>())
        {
            std::cout << "\tfloat ";
            done = true;
        }
        if(!done && v.Type<double>())
        {
            std::cout << "\tdouble ";
            done = true;
        }
        if(!done)
            std::cout << "\t?????? ";

        std::cout << v.Name() << "(";
        for (int di = 0; di < v.NumDims(); ++di)
        {
            gz4d::netcdf::Dim d = v.GetDim(di);
            std::cout << d.Name();
            if(di < v.NumDims()-1)
                std::cout << ", ";
        }
        std::cout << ") ;" << std::endl;
        for(int j = 0; j < v.NumAtts(); ++j)
        {
            gz4d::netcdf::Att a = v.GetAtt(j);
            std::cout << "\t\t" << v.Name() << ":" << a.Name() << " = ";
            if(a.Type<char>())
            {
                std::cout << "\"" << a.As<std::string>(0) << "\"";
            }
            else
            {
                for(int k=0; k < a.NumVals(); ++k)
                {
                            
                    std::cout << a.As<std::string>(k);
                    if(k < a.NumVals()-1)
                        std::cout << ", ";
                }
            }
            std::cout << " ;" << std::endl;
        }
    }
    std::cout << "}" << std::endl;
    cout << "*********************************************\n";
    
}
