// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/gPhoto/Camera.h>
#include <iostream>

void PrintConfig(gz4d::gphoto::Config &cfg, std::string const &indent = std::string(""))
{
    std::cout << indent << cfg.GetName() << "\t" << cfg.GetLabel() << std::endl;
    std::cout << indent << "    " << cfg.GetType() << "\t" << cfg.GetInfo() << std::endl;
    if(cfg.GetDataType() == "string")
        std::cout << indent << "    " << "value:\t" << cfg.Get() << std::endl;
    if(cfg.GetType() == "range")
        std::cout << indent << "    " << "value:\t" << cfg.GetFloat() << "\trange: " << cfg.GetMin() << ", " << cfg.GetMax() << " inc: " << cfg.GetIncrement() << std::endl;

    if(cfg.GetDataType() == "bool")
        std::cout << indent << "    " << "value:\t" << cfg.GetBool() << std::endl;

    if(cfg.GetType() == "choice")
    {
        std::cout << indent << "      (";
        for(std::vector<std::string>::const_iterator i = cfg.GetChoices().begin(); i != cfg.GetChoices().end(); ++i)
        {
            std::cout << *i;
            if(i+1 != cfg.GetChoices().end())
                std::cout << ", ";
        }
        std::cout << ")" << std::endl;
    }
    for (gz4d::gphoto::Config::iterator i = cfg.begin(); i != cfg.end(); ++i)
        PrintConfig(i->second,indent+"  ");
}

void Set(gz4d::gphoto::Config &cfg, std::map<std::string,std::string> &configs)
{
    std::map<std::string,std::string>::iterator c = configs.find(cfg.GetName());
    if (c != configs.end())
    {
        std::cout << "setting " << c->first << " to " << c->second << std::endl;
        cfg.Set(c->second);
    }
    for (gz4d::gphoto::Config::iterator i = cfg.begin(); i != cfg.end(); ++i)
        Set(i->second,configs);
}

int main(int argc, char * argv[])
{
    gz4d::gphoto::Camera camera;
    gz4d::gphoto::Config cfg = camera.GetConfig();
    PrintConfig(cfg);

    if(argc>1)
    {
        std::map<std::string,std::string> configs;
        for (int i = 1; i < argc-1; i += 2)
        {
            configs[argv[i]] = argv[i+1];
        }
        std::cout << std::endl;
        Set(cfg,configs);
        std::cout << std::endl;
        camera.SetConfig(cfg);
        cfg = camera.GetConfig();
        PrintConfig(cfg);
    }

}
