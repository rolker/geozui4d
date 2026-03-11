// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gz4d/netcdf.h>

int main(int argc, char** argv)
{
    int ret = 0;

    for(int i = 1; i < argc; ++i)
    {
        gz4d::netcdf::File ncfile(argv[i]);
        if (ncfile)
        {
            std::cout << "netcdf " << argv[i] << " {" << std::endl;
            std::cout << "dimensions:" << std::endl;
            for(int i = 0; i < ncfile.NumDims(); ++i)
            {
                gz4d::netcdf::Dim d = ncfile.GetDim(i);
                std::cout << "\t" << d.Name() << " = ";
                if (d.IsUnlimited())
                    std::cout << "UNLIMITED ; // (" << d.Size() << " currently)" << std::endl;
                else
                    std::cout << d.Size() << " ;" << std::endl;
            }
            std::cout << "variables:" << std::endl;
            for(int i = 0; i < ncfile.NumVars(); ++i)
            {
                gz4d::netcdf::Var v = ncfile.GetVar(i);
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
        }
        else
        {
            std::cerr << "Error opening file: " << argv[i] << std::endl;
            ret = 1;
        }
    }

    return ret;
}
