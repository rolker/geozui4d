// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/ogr.h>
#include <gz4d/ogr/link.h>
#include <iostream>

int main(int argc, char * argv[])
{
    std::vector<std::string> args;

    bool showFeatures = false;
    for(int i = 1; i < argc; ++i)
    {
        std::string a(argv[i]);
        if (a == "-f")
            showFeatures = true;
        else
            args.push_back(a);
    }

    
    for(std::vector<std::string>::iterator a = args.begin(); a != args.end(); ++a)
    {
        gz4d::ogr::DataSource ds(*a);
        //std::cout << ds.GetName() << std::endl;
        std::cout << "  " << ds.size() << " layers" << std::endl;
        for(int l = 0; l < ds.size(); ++l)
        {
            gz4d::ogr::Layer layer = ds[l];
            gz4d::ogr::Feature::Definition def = layer.GetDefinition();
            std::cout << "    layer definition name: " << def.GetName() << std::endl;
            std::cout << "      " << def.size() << " fields" << std::endl;
            for (int f = 0; f < def.size(); ++f)
            {
                gz4d::ogr::Feature::Field::Definition fdef = def[f];
                std::cout << "        " << fdef.GetName() << " (" << fdef.GetType() << ")" << std::endl;
            }
            std::cout << "    layer with " << layer.size() << " features" << std::endl;
            layer.ResetReading();
            if(showFeatures)
            {
                for (int f = 0; f < layer.size(); ++f)
                {
                    gz4d::ogr::Feature feature = layer.GetNextFeature();
                    for(int fn = 0; fn < feature.size(); ++fn)
                    {
                        gz4d::ogr::Feature::Field field = feature[fn];
                        std::cout << "          " << f << ":" << fn << "\t" << field.GetDefinition().GetName();
                        if(field)
                            std::cout << ": " << field.GetValue<std::string>();
                        std::cout << std::endl;
                    }
                }
            }
        }
    }
    return 0;
}
