// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "engine/Window3D.h"
#include"S57_beacon.h"
#include<gz4d/glo.h>
#include<GL/glut.h>
#include "engine/PosableStateBase.h"
#include "engine/gzCommandInterface.h"
#include "engine/Scene3D.h"
#include "utilities/DottedAddress.h"
#include "S57_supplemental.h"
#include <map>

REGISTER_OBJECT(S57_beacon,"");

#include <iostream>

static const char beaconShapeName[][32] = {"Unknown",
                                "Stake, pole, perch, or post",
                                "Withy (tree without roots)",
                                "Beacon tower",
                                "Lattice beacon",
                                "Pile beacon",
                                "Cairn (mound of stones)",
                                "Buoyant beacon" 
                               };

/*---------------------------------------------------------------------------*/
S57_beacon::S57_beacon()
{
    pointSize = 4;
    shape = SHP_DEFAULT;
}

/*---------------------------------------------------------------------------*/
S57_beacon::~S57_beacon()
{
}

/*---------------------------------------------------------------------------*/
void S57_beacon::ShowS57Info(Scene3D *scene)
{
    ShowS57ShapeInfo(scene);
    S57_object::ShowS57Info(scene);
    if (children.size() > 0)
    {
        int i, size = children.size();
	    //scene->appendInfo("====================\n");
        for (i = size - 1; i >= 0; i--)
        {
            children[i]->ShowS57Info(scene);
        }
    }
}

/*---------------------------------------------------------------------------*/
void S57_beacon::ShowS57ShapeInfo(Scene3D *scene)
{
	char infoLine[512];

    if (generateSpecialPurposeString(infoLine))
        scene->appendInfo(infoLine);
    if (shape > 0 && shape <= SHP_BUOYANT)
    {
        sprintf(infoLine, "Shape:\t%s\n", beaconShapeName[shape]);
        scene->appendInfo(infoLine);
    }
}

/*---------------------------------------------------------------------------*/
void S57_beacon::SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber)
{
    S57_object::SetAttributes(item, info, objectNumber);

    // beacon shape
    if(item["BCNSHP"])
        shape = item["BCNSHP"].GetValue<int>();
    categoryList = item["CATSPM"].GetValue<std::vector<int> >();
}

/*---------------------------------------------------------------------------*/
void S57_beacon::resolveReferences(LnamTable &lnam_table, std::string const &s57file)
{
    S57_object::resolveReferences(lnam_table, s57file);
    S57_supplemental *child;

    if (lnam_refs.empty())
        return;

    //std::cerr << "Objects under " << getAddress()->getName() << ":" << std::endl;
    for (std::size_t i = 0; i < lnam_refs.size(); i++)
    {
        if (!lnam_table.count(lnam_refs[i]))
            std::cerr << "[Missing object " << lnam_refs[i] << "]" << std::endl;
        else
        {
            DYNAMIC_CAST(child, S57_supplemental *, lnam_table[lnam_refs[i]]);
            if (!child)
                std::cerr << "Unsure how to associate with child " << lnam_table[lnam_refs[i]]->getAddress()->getAddress() << std::endl;
            else
            {
                children.push_back(child);
                child->setParentS57(this);
            }
        }
    }
}
