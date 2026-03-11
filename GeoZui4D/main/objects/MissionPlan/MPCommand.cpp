// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "MPCommand.h"
#include "engine/PosableStateBase.h"
#include "GZincludes.h"
#include <iomanip>


#include "MPPoint.h"
#include "MPHandle.h"
#include "MPSegment.h"


using namespace std;

REGISTER_OBJECT(MPCommand, "");

MPCommand::MPCommand()
{
}

MPCommand::MPCommand(int cid, gz4d::xml::Node initcmd)
{
    cmdID = cid;
    
            color[0] =         color[1] =         color[2] = 0.8;	  color[3] = 1.0;
    selectedColor[0] = selectedColor[1] = selectedColor[2] = selectedColor[3] = 1.0;
    command(initcmd);
}


void MPCommand::getColor(gz4d::xml::Node node, float * const color)
{
/*    node.getFloat(color[0],"r");
    node.getFloat(color[1],"g");
    node.getFloat(color[2],"b");
    node.getFloat(color[3],"a");*/
}

void MPCommand::command(gz4d::xml::Node c)
{
    std::cout << "received: " << c.Name() << std::endl;

    //bool newcmd = (c.nodeName() == "newcmd");
/*    string cmd(c.nodeName());

    if (c.Name() == "getpos")
    {
        int id;
        if(c.getInt(id,"id"))
            ((MissionPlan*)parent)->sendPosition(id);
    }

    c.childNode();
    while (c.hasCurrentNode())
    {
        if(c.nodeName() == "color")
        {
            getColor(c,color);
        }
        else if (c.nodeName() == "selectedColor")
        {
            getColor(c,selectedColor);
        }
        else 
        {
            int id;
            if(c.getInt(id,"id"))
            {
                if(cmd == "newcmd")
                {
                    string newType;
                    c.getString( newType,"type");
                    MPItemBase * newitem = NULL;
                    if (newType == "handle")
                        newitem = new MPHandle(id,c);
                    if (newType == "point")
                        newitem = new MPPoint(id,c);
                    if (newType == "segment")
                        newitem = new MPSegment(id,c);
                    AddObject(newitem);
                    items[id] = newitem;
                }
                else
                    items[id]->command(c,cmd);
            }
        }
        c.nextNode();
    }
    c.parentNode();*/
}

void MPCommand::itemSelected(int item, bool s)
{
    selectedCmd = s;

    if (s)
    {
        stringstream s;
        s << "<mp><select id=\"" << cmdID << "\"><item id=\"" << item << "\"/></select></mp>";
        ((MissionPlan *)parent)->sendToGUI(s.str());
        ((MissionPlan *)parent)->cmdSelected(this);
    }
}

void MPCommand::itemMoved(int id, gz4d::Point<double> const &p)
{
    stringstream cmd;
    cmd << "<mp><update id=\"" << cmdID << "\"><item id=\"" << id << "\"><position x=\"" << fixed << setw(12) << p[0] << "\" y=\"" << setw(12) << p[1] << "\" z=\"" << p[2] << "\"/></item></update></mp>";
    ((MissionPlan *)parent)->sendToGUI(cmd.str());
}

