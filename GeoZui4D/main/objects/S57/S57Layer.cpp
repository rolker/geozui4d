#include"S57Layer.h"
#include<iostream>
#include"S57_object.h"

#include"S57_land.h"
#include"S57_bridge.h"
#include"S57_building.h"
#include"S57_buoy.h"
#include "S57_area.h"
#include <gz4d/ogr.h>

using namespace std;

S57Layer::S57Layer(gz4d::ogr::Layer layer, bool publishObjects, LnamTable &lnam_table)
    : ObjectCollection(GL_TRUE, 0) //dont make the layer pickable
{
    int i;
    int fcount;
    S57_object * object;

    objectCount = 0;

    fcount = layer.size();
    S57_LayerInfo info = S57_object::getLayerInfo(layer.GetDefinition().GetName());
    if (!info.reg || !*info.reg)
        return;


    gz4d::ogr::Envelope e = layer.GetExtent();
    if (e)
    {
        gz4d::Vector<double,2> center = gz4d::Box<gz4d::Vector<double,2> >(e).getCenter();
        autoProjection(center[0],center[1]);
    }

    if(layer.GetSpatialRef())
        spatialReference = layer.GetSpatialRef();

    SetLabel(info.name);
    for(i=0;i<fcount;i++)
    {
        gz4d::ogr::Feature item = layer.GetNextFeature();
        object = (S57_object*) (*info.reg)->create();
//		if (strcmp(info->acronym, "SOUNDG") == 0)
//			DumpAttributes(item);
        if (info.acronym == "SOUNDG")
            SetInvisible();
        if(object)
        {
            object->InitializeFor(info.acronym);
            gz4d::ogr::Geometry geo = item.GetGeometry();
            object->ProcessGeometry(geo,spatialReference);
            object->SetAttributes(item, info, objectCount); //SetAttributes must come after ProcessGeometry
            AddObject(object, publishObjects);
            if (!object->getLNAM().empty())
                lnam_table[object->getLNAM()] = object;
            objectCount++;
        }
        else
            i=fcount; // short circuit unimplemented layers
    }
}

S57Layer::~S57Layer()
{
}

void S57Layer::DumpAttributes(gz4d::ogr::Feature item) const
{
    for(std::size_t fn = 0; fn < item.size(); ++fn)
    {
        gz4d::ogr::Feature::Field field = item[fn];
        std::cerr << "  " << field.GetDefinition().GetName() << ": ";
        if(field)
            std::cerr << " " << field.GetValue<std::string>();
        std::cout << std::endl;
    }
}



