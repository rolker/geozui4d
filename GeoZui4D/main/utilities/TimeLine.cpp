// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// TimeLine.cpp
/** 
 *	Class for maintaining a track of locations that can be represented
 *	graphically with a line.  The TimeLineNode data type is intended to be
 *	derived by child classes to facilitate the addition of more
 *	information.
 *  
 *	Created by Matthew Plumlee 12/9/2004
 */
///////////////////////////////////////////////////////////////////////////////
#include "utilities/TimeLine.h"
#include "objects/Segment.h"
#include "engine/PosableStateWrapper.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
TimeLine::TimeLine(Line * l)
{
    if(!l)
    {
        line=new Line;
        line->setSize(0.02f);
    }
    else
        line = l;

    line->setTimeLine(this);
    resetIndexedNode();

    selectedNode = NULL;
    selectedNodeIndex = -1;
}

/*---------------------------------------------------------------------------*/
TimeLine::~TimeLine()
{
    LineNodeType *doomed;

    iterator = start;
    while (iterator)
    {
        doomed = (LineNodeType*) iterator->data;
        if (doomed)
            delete doomed;
        iterator->data = NULL;
        iterator = iterator->next;
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
TimeLine::TimeSequenceNode *TimeLine::insert(double t, TimeLineNode *info, double x, double y, double z)
{
	TimeSequence::TimeSequenceNode *c = NULL, *p, *n;
	LineNodeType *prev = NULL, *newguy = NULL;
	
	if(!start)
		line->SetPosition(x,y,z); // make sure line's pos doesn't stay at 0,0,0

	if (!end || end->time != t)
		newguy = info->lineNode = line->newLineNode();
	else
		info->lineNode = ((TimeLineNode*)end->data)->lineNode;


        info->lineNode->position[X] = x;
        info->lineNode->position[Y] = y;
        info->lineNode->position[Z] = z;
	
	if (!end || end->time != t)
	{
		c = TimeSequence::internalInsert(t, info);
		if (!c)
			return NULL;
		p = c->prev;
		n = c->next;
		if (!p)
		{
			newguy->next = line->lineNodes;
			line->lineNodes = newguy;
			prev = NULL;
		}
		else
		{
			prev = ((TimeLineNode*)p->data)->lineNode;
			newguy->next = prev->next;
			prev->next = newguy;
		}
		if (newguy->next)
		{
                    newguy->segment = line->newSegment(info->lineNode->position,newguy->next->position);
		}
		if (prev)
		{
			if (!prev->segment)
                            prev->segment = line->newSegment(prev->position,info->lineNode->position);
		}
	}

	line->lineDLNeedsRebuild = GL_TRUE;
	line->tubeDLNeedsRebuild = GL_TRUE;
	return c;
}

/*---------------------------------------------------------------------------*/
void TimeLine::insert(double t, double x, double y, double z)
{
	TimeLineNode *info = new TimeLineNode;
	insert(t, info, x, y, z);
}

/*---------------------------------------------------------------------------*/
LineNodeType *TimeLine::insertFromLine(double t, gz4d::Point<double> const &point, LineNodeType *&previous, LineNodeType *lnode)
{
	TimeLineNode *info = new TimeLineNode;
	TimeSequenceNode *node;

	info->lineNode = lnode;
		
	node = insert(t, info, point[X], point[Y], point[Z]);
	previous = NULL;
	if (node->prev && node->prev->data)
		previous = ((TimeLineNode *)(node->prev->data))->lineNode;
	
	return info->lineNode;
}

/*---------------------------------------------------------------------------*/
/**	This function removes the data of the TimeSequenceNode from the Line, but
 *	does not delete anything or remove anything from the TimeLine.  This is
 *	done so the data can be deleted as the right type.
 */
void TimeLine::removeFromLine(TimeSequenceNode *c)
{
	LineNodeType *d;

	if (!c)
		return;

	if (!line)
		return;

	d = ((TimeLineNode *)c->data)->lineNode;
	line->remove(d);
}

/*---------------------------------------------------------------------------*/
/**	returns position at current iteration node
 */
gz4d::Point<double> TimeLine::getPosition()
{
	if (iterator && iterator->data)
		return ((TimeLineNode *)(iterator->data))->lineNode->position;
	else
		return gz4d::Point<double>::Invalid();
}

/*---------------------------------------------------------------------------*/
/**	returns position at time t
 */
bool TimeLine::getPosition(double t, gz4d::Point<double> &point)
{
	TimeLineNode *info2;
	double weight;

	if (getPosition(t, weight, info2, point))
		return true;
	return false;
}

/*---------------------------------------------------------------------------*/
/**	returns position at time t with support information to make interpolations
 *	on other data actually in the derived TimeLineNode data type.
 */
TimeLineNode *TimeLine::getPosition(double t, double &weight, TimeLineNode *&info2, gz4d::Point<double> &point)
{
    TimeLineNode *info1;
    void *v;

    info1 = (TimeLineNode*)get(t, weight, v);
    if (!info1)
            return NULL;
    info2 = (TimeLineNode*) v;

    point = info1->lineNode->position + (info2->lineNode->position-info1->lineNode->position)*weight;

    return info1;
}

/*---------------------------------------------------------------------------*/
void TimeLine::DrawLowResSection(float degradation, Window3D *win, double startTime, double endTime)
{
    line->DrawLowResSection(degradation,win,((TimeLineNode*)get(startTime))->lineNode,((TimeLineNode*)get(endTime))->lineNode);
}

/*---------------------------------------------------------------------------*/
/**	We're using a 1-based indexing scheme here.
 */
TimeLineNode *TimeLine::indexNodeAt(int idx)
{
	if (nodeIndex < 1 || indexedNode == NULL)
	{
		nodeIndex = 1;
		indexedNode = start;
	}

	if (nodeIndex < idx)
	{
		for (; nodeIndex < idx && indexedNode; nodeIndex++)
			indexedNode = indexedNode->next;
	}
	else
	{
		for (; nodeIndex > idx && indexedNode; nodeIndex--)
			indexedNode = indexedNode->prev;
	}
	if (!indexedNode)
		return NULL;
	return (TimeLineNode *)indexedNode->data;
}

/*---------------------------------------------------------------------------*/
GLboolean TimeLine::indexContentsModifiable(PosableStateWrapper *wrapper)
{
	if (indexNodeAt(wrapper->getIntegerIndex()))
		return GL_TRUE;
	return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void TimeLine::wrapperChangedValue(PosableStateWrapper *wrapper, long &changeMask)
{
	TimeLineNode *lineNode;

	if (!wrapper)
	{
		changeMask = ChangeNone;
		return;
	}

	//find the indexed node
	lineNode = indexNodeAt(wrapper->getIntegerIndex());
	if (!lineNode)
	{
		changeMask = ChangeNone;
		return;
	}

        lineNode->lineNode->position[X] = wrapper->getPosition(changeMask)[X];
        lineNode->lineNode->position[Y] = wrapper->getPosition(changeMask)[Y];
}

/*---------------------------------------------------------------------------*/
void TimeLine::updateWrapper(PosableStateWrapper *wrapper, long &changeMask)
{

    if (wrapper)
    {
        if (changeMask & ChangeIndex)
            changeMask |= ChangeTranslate;
        else
            changeMask &= ChangeTranslate;

        selectedNodeIndex = wrapper->getIntegerIndex();
        selectedNode = indexNodeAt(selectedNodeIndex);
        if (!selectedNode)
        {
            changeMask = ChangeNone;
            return;
        }
        wrapper->beginLocalSet();
        wrapper->setPosition(selectedNode->lineNode->position, changeMask);
        wrapper->endLocalSet();
    }
}

/*---------------------------------------------------------------------------*/
PosableWrapperList *TimeLine::findWrapperNode(PosableStateWrapper *wrapper, GLboolean mustExist, GLboolean getParentNode)
{
    if (!wrapper)
        return NULL;

    return &wrappers;
}

void TimeLine::getHtmlInfo(std::stringstream &html, short level)
{
    TimeSequence::getHtmlInfo(html, level);
    html << "<h" << level << ">TimeLine</h" << level << ">";
    line->getHtmlInfo(html,level -1);
    html << "<hr><br>";
}
