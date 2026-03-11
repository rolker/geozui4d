// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////

#include"objects/Line.h"

REGISTER_OBJECT(Line, "");

#include"objects/Segment.h"
#include"utilities/TimeLine.h"
#include<iostream>
#include <gz4d/ios.h>

using namespace std;

Line::Line()
{
    lineNodes = NULL;
    lineDisplayList = tubeDisplayList = 0;
    lineDLNeedsRebuild = tubeDLNeedsRebuild = true;

    highlightColor[0]=0.4f;
    highlightColor[1]=0.4f;
    highlightColor[2]=1.0;

    haveHighlight = false;

    time = NULL;
    lastStartNode = NULL;
    lastStopNode = NULL;

    drawTubeFlag = true;
    deletingInfo = true;
    drawForPicking = true;
}

Line::~Line()
{
    LineNodeType *doomed = lineNodes;

    if (!lineNodes)
        return;

    do
    {
        lineNodes = lineNodes->next;
        if (deletingInfo)
        {
            delete doomed->height;
            delete doomed->width;
            delete doomed->segment;
        }
        delete doomed;
    } while (lineNodes);
}

LineNodeType * Line::newLineNode()
{
    LineNodeType * ret;

    ret = new LineNodeType;

    ret->height=NULL;
    ret->width=NULL;
    ret->segment=NULL;
    ret->next=NULL;

    return ret;
}

void Line::remove(LineNodeType *node)
{
    LineNodeType *prev;

    if (!node)
        return;

    if (lineNodes == node)
        lineNodes = node->next;

    for (prev = lineNodes; prev; prev = prev->next)
    {
        if (prev->next == node)
        {
            prev->next = node->next;
            break;
        }
    }
    if (deletingInfo)
    {
        delete node->height;
        delete node->width;
        delete node->segment;
    }
    delete node;
}


Object * Line::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    string nodestr;
    gz4d::Point<double> p;
    LineNodeType * cursor = NULL, *prev = NULL;

    double t;

    Object::LoadXML(xml,path, ncfile);

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.color"))
    {
        std::stringstream ss(*attribute);
        ss >> *startColor;
        *endColor = *startColor;
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.startColor"))
    {
        std::stringstream ss(*attribute);
        ss >> *startColor;
    }
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.endColor"))
    {
        std::stringstream ss(*attribute);
        ss >> *endColor;
    }
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.drawTube"))
        drawTubeFlag = gz4d::IsTrue(*attribute);
    
    boost::optional<float> float_attrib;
    if(float_attrib = xml.get_optional<float>("<xmlattr>.width"))
    {
        startWidth = *float_attrib;
        endWidth = startWidth;
    }
    
    if(float_attrib = xml.get_optional<float>("<xmlattr>.height"))
    {
        startHeight = *float_attrib;
        endHeight=startHeight;
    }
    
    for(bpt::ptree::const_iterator n = xml.begin(); n != xml.end(); ++n)
        if(n->first == "Node")
        {
        if(sscanf(n->second.data().c_str(),"%lf %lf %lf",&p[0],&p[1],&p[2])==3)
        {
            boost::optional<double> double_attrib;
            if(double_attrib = n->second.get_optional<double>("<xmlattr>.time"))
            {
                t = *double_attrib;
                if(!time)
                    time = new TimeLine(this);
                cursor = time->insertFromLine(t, p, prev,newLineNode());
            }
            else
            {
                if (!lineNodes)
                {
                    SetPosition(p);
                    lineNodes = newLineNode();
                    cursor = lineNodes;
                }
                else
                {
                    cursor->next = newLineNode();
                    prev = cursor;
                    cursor = cursor->next;
                }
            }

            if(attribute = n->second.get_optional<std::string>("<xmlattr>.color"))
            {
                gz4d::Color c;
                std::stringstream ss(*attribute);
                ss >> c;
                cursor->color = std::shared_ptr<gz4d::Color>(new gz4d::Color(c));
            }
            
            if(float_attrib = n->second.get_optional<float>("<xmlattr>.width"))
            {
                cursor->width = new float;
                *cursor->width = *float_attrib;
            }

            if(float_attrib = n->second.get_optional<float>("<xmlattr>.height"))
            {
                cursor->height = new float;
                *cursor->height = *float_attrib;
            }

            cursor->position = p;

            if (prev)
            {
                prev->segment = newSegment();
                prev->segment->setPositions(gz4d::Point<float>(prev->position-GetPosition()),gz4d::Point<float>(cursor->position-GetPosition()));
                prev->segment->setCapTypes(Segment::Sphere,Segment::Sphere);
                prev->segment->setColor(prev->color,cursor->color);
                prev->segment->setScales(prev->width,prev->height,cursor->width,cursor->height);
            }
        }
    }
    return this;
}

void Line::DrawLowResSection(float degradation, Window3D *win, LineNodeType *startNode, LineNodeType *endNode)
{
    LineNodeType *cursor, *o = NULL, *tooFar = NULL;
    float lineWidth;

    if (!lineNodes)
        return;

    if(startNode)
        cursor = startNode;
    else
        cursor = lineNodes;


    if(endNode)
        tooFar = endNode->next;


    if(startNode != lastStartNode || endNode != lastStopNode)
    {
        lastStartNode = startNode;
        lastStopNode = endNode;
        lineDLNeedsRebuild = true;
        tubeDLNeedsRebuild = true;
    }
    if(degradation >= 0.5f || !drawTubeFlag)// 0.95)
    {
        if (lineDLNeedsRebuild)
        {
            if(!lineDisplayList)
                lineDisplayList = glGenLists(1);
            if(lineDisplayList)
                glNewList(lineDisplayList,GL_COMPILE);
            glDisable(GL_LIGHTING);
            glGetFloatv(GL_LINE_WIDTH, &lineWidth);
            if ((*startColor)[3] < 1.0)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            }
            glLineWidth(2.0);
            glBegin(GL_LINE_STRIP);
                while (cursor && cursor->segment && cursor != tooFar)
                {
                    cursor->segment->DrawStartPoint(win);
                    o = cursor;
                    cursor = cursor->next;
                }
                if (o && o->segment)
                    o->segment->DrawEndPoint(win);
            glEnd();
            glLineWidth(lineWidth);
            glEnable(GL_LIGHTING);
            if(lineDisplayList)
            {
                glEndList();
                lineDLNeedsRebuild = 0;
            }
        }
        glCallList(lineDisplayList);
    }
    else
    {
        if (tubeDLNeedsRebuild)
        {
            if(!tubeDisplayList)
                tubeDisplayList = glGenLists(1);
            if(tubeDisplayList)
                glNewList(tubeDisplayList,GL_COMPILE);
            while (cursor && cursor->next != tooFar)
            {
                if(cursor->segment)
                        cursor->segment->Draw(degradation,win);
                cursor = cursor->next;
            }

            if(tubeDisplayList)
            {
                glEndList();
                tubeDLNeedsRebuild = 0;
            }
        }
        glCallList(tubeDisplayList);
    }
}


void Line::DrawLowRes(float degradation, Window3D *win)
{
    if (!drawForPicking && renderingForPicking)
        return;

    if(time)
    {
        DrawLowResSection(degradation,win,NULL,(LineNodeType*)(time->get(lastIdleTime)));
    }
    else
        DrawLowResSection(degradation,win,NULL,NULL);
}

void Line::getHtmlInfo(stringstream &html, short level)
{
	html << "<h" << level << ">Line</h" << level << ">";
	if (lineNodes)
		html << "First node: " << lineNodes->position[0] << ", " << lineNodes->position[1] << ", " << lineNodes->position[2] << "<br>";
}

