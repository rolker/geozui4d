// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "MPItemBase.h"
#include "MPCommand.h"
#include "engine/InputMediator.h"

MPItemBase::MPItemBase()
{
    itemID = 0;
    init();
}

MPItemBase::MPItemBase(int id, gz4d::xml::Node cmd, int clickType)
: ObjectCollection(true,clickType)
{
    itemID = id;
    init();
    inConstructor = true;
    MPItemBase::command(cmd,std::string());
    inConstructor = false;
}

void MPItemBase::init()
{
    highLevelEventMediator->PushInteractor(this);
    size = 0.5;
            color[0] =         color[1] =         color[2] = 0.8;	  color[3] = 1.0;
    selectedColor[0] = selectedColor[1] = selectedColor[2] = selectedColor[3] = 1.0;
}


void MPItemBase::command(gz4d::xml::Node c, std::string const &cmd)
{
    if (cmd == "select")
    {
        if (!selectedFlag)
        {
//            setSelectedObject( this);
//            ((MPCommand*)parent)->itemSelected(itemID);
        }
    }
/*    c.getFloat(size,"size");
    c.childNode();
    while(c.hasCurrentNode())
    {
        if(c.nodeName() == "color")
            MPCommand::getColor(c,color);
        else if(c.nodeName() == "selectedColor")
            MPCommand::getColor(c,selectedColor);
        c.nextNode();
    }
    c.parentNode();*/
}

int MPItemBase::HandleInput(unsigned int inputType, InputInfo *info)
{
    if (inputType == Object::objectSelectedEvent())
    {
        if (info->val.p == pose)
        {
            ((MPCommand*)parent)->itemSelected(itemID);
        }
    }
    return PASS_EVENT_THROUGH;
}
