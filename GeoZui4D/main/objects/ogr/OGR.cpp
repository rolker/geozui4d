#include"OGR.h"
#include"OGR_Layer.h"
#include<gz4d/ogr.h>
#include "OGR_Limits.h"

REGISTER_OBJECT(OGR,"");

OGR::OGR()
{
}

Object * OGR::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object::LoadXML(xml,path,ncfile);

    float minSpan = 0.0;
    OGR_Limits limits;

    minSpan = xml.get("<xmlattr>.minimum_span",minSpan);
    limits.minX = xml.get("<xmlattr>.minimum_x",limits.minX);
    limits.maxX = xml.get("<xmlattr>.maximum_x",limits.maxX);
    limits.minY = xml.get("<xmlattr>.minimum_y",limits.minY);
    limits.maxY = xml.get("<xmlattr>.maximum_y",limits.maxY);

    std::vector<std::string> layers;
    for(bpt::ptree::const_iterator n = xml.begin(); n != xml.end(); ++n)
        if(n->first == "Layer")
            layers.push_back(n->second.data());

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.filename"))
    {
        bfs::path fn = bfs::path(path)/(*attribute);
        if((*attribute)[0] == '/')
            fn = *attribute;
        gz4d::ogr::DataSource dataSource(fn.string());
        if(dataSource)
        {
            int layerCount = dataSource.size();
            for (int i = 0; i < layerCount; ++i)
            {
                OGR_Layer * l = NULL;
                
                if(!layers.empty())
                {
                    std::string lname = dataSource[i].GetDefinition().GetName();
                    if(std::find(layers.begin(),layers.end(),lname) != layers.end())
                        l = new OGR_Layer(dataSource[i], limits, minSpan);
                }
                else
                    l = new OGR_Layer(dataSource[i], limits, minSpan);
                if(l)
                    AddObject(l);
            }
        }
        else
            return NULL;
    }
    else
        return NULL;

    return this;
}

