// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// RubberBandObject.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  This object is used to illustrate a box for rubber-banding effects,
 *  initially created for selecting regions in GUTMs.
 *  
 *  Created by mdp 10/15/2004
 */
/*---------------------------------------------------------------------------*/
#include<gz4d/glo.h>
#include <GL/glut.h>
#include "objects/RubberBandObject.h"
#include "widgets/Sv_Standards.h"
#include "interactors/PointHandle.h"
#include "engine/TranslatableState.h"
#include "engine/InputMediator.h"
#include "engine/PosableStateWrapper.h"
#include <string.h>
#include "engine/Window3D.h"

extern PointHandle *pointHandle;

REGISTER_OBJECT(RubberBandObject, "rubberband");

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
RubberBandObject::RubberBandObject()
{
    SetTypeAndVersion("RUBBERBAND_OBJECT", 1, 0);
    oppositeCorner = new TranslatableState(this);
    isEditableFlag = true;
    selectedCorner = -1;
    canUseInternalHandleFlag = GL_TRUE;
    allowBaseExternalEdit = false;
    useMinMax = GL_FALSE;
    zChangeable = GL_TRUE;
    step = 0.0;
    verticalBoost = 0.0;
}

/*---------------------------------------------------------------------------*/
RubberBandObject::~RubberBandObject()
{
    delete oppositeCorner;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void RubberBandObject::setValidRange(float mx, float Mx, float my, float My,
                                     float step_amount)
{
    minX = mx;
    minY = my;
    maxX = Mx;
    maxY = My;
    if (step >= 0.0)
        step = step_amount;
    useMinMax = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void RubberBandObject::setKeepPointsOnTop(GLboolean b)
{
    if (keepPointsOnTop == b)
        return;

    keepPointsOnTop = b;
    if (!b)
        verticalBoost = 0.0f;
    else
        verticalBoost = oppositeCorner->getPosition()[Z] - pose->getPosition()[Z];
}

/*---------------------------------------------------------------------------*/
/** This function renders the rubberband in the passed window.
 */
void RubberBandObject::DrawLowRes(float degradation, Window3D *inWin)
{
    gz4d::Point<double> pos0 = pose->getPosition(), pos1 = oppositeCorner->getPosition();
    
    gz4d::Point<double> op, zero(0.0, 0.0, verticalBoost);
    
    op = pos1-pos0;

    if (!visible)
        return;

    glDisable(GL_LIGHTING);        
    glLineWidth(1.0);
    if (!renderingForPicking)
    {
        glPushMatrix();
        glLoadIdentity();
        pose->applyTransforms(inWin->getViewInfo());
    }
    //draw the strip
    if (!selectedFlag || !renderingForPicking)
    {
        glColor3f(0.9f, 0.8f, 0.4f);
        glBegin(GL_QUAD_STRIP);
            glVertex3f(0.0,0.0,0.0);
            glVertex3f(0.0, 0.0, op[Z]);
            glVertex3f(0.0, op[Y], 0.0);
            glVertex3f(0.0, op[Y], op[Z]);
            glVertex3f(op[X], op[Y], 0.0);
            glVertex3f(op[X],op[Y], op[Z]);
            glVertex3f(op[X], 0.0, 0.0);
            glVertex3f(op[X], 0.0, op[Z]);
            glVertex3f(0.0,0.0,0.0);
            glVertex3f(0.0, 0.0, op[Z]);
        glEnd();

    }
    //draw the handles
    if (selectedFlag)
    {
        if (renderingForPicking)
            glLoadName(0);
        pointHandle->drawIn(inWin, zero, (selectedCorner == 0)? 1 : 2, true);
        if (renderingForPicking)
            glLoadName(1);
        pointHandle->drawIn(inWin, op, (selectedCorner == 1)? 1 : 2, true);
    }
    glEnable(GL_LIGHTING);
    if (!renderingForPicking)
        glPopMatrix();
}


/*---------------------------------------------------------------------------*/
void RubberBandObject::setCorner(int index, float x, float y, float z)
{
    PosableStateBase *p;
    if (index == 0)
        p = pose;
    else
        p = oppositeCorner;
    p->setPosition(x, y, z);
}

/*---------------------------------------------------------------------------*/
gz4d::Point<double> RubberBandObject::getCorner(int index)
{
    PosableStateBase *p;
    if (index == 0)
        p = pose;
    else
        p = oppositeCorner;
    return p->getPosition();
}

/*---------------------------------------------------------------------------*/
int RubberBandObject::pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
    int numNames = names[0];

    if (pickType == PICK_CLICK1 && !selectedFlag)
    {
        setSelectedObject(this);
        if (isEditableFlag)
            selectedCorner = names[1];
        //ShowInfo(&names[1], numNames);
        highLevelEventMediator->Trigger(objectSelectedInputType->value, pose, (void*)selectedCorner);
        return GL_TRUE;
    }
    else
    {
        if (pickType == PICK_PRESS1 && isEditableFlag)
        {
            if (numNames >= 2)
                selectedCorner = names[2];
            //ShowInfo(&names[1], numNames);
            highLevelEventMediator->Trigger(objectSelectedInputType->value, pose, (void*)selectedCorner);
        }

        return Object::pickAction(pickType, inputInfo, names, hits);
    }
}

/*---------------------------------------------------------------------------*/
GLboolean RubberBandObject::indexContentsModifiable(PosableStateWrapper *wrapper)
{
    int idx = wrapper->getIntegerIndex();

    if (idx == 0 || idx == 1)
        return GL_TRUE;
    return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void RubberBandObject::wrapperChangedValue(PosableStateWrapper *wrapper, long &changeMask)
{
    int idx;
    PosableStateBase *p;
    gz4d::Point<double> desiredPos, oldPos;
    gz4d::Point<double> newPos;
    GLboolean wrapperNeedsUpdate = GL_FALSE;

    if (!wrapper)
    {
        changeMask = ChangeNone;
        return;
    }

    idx = wrapper->getIntegerIndex();
    if (idx != 0 && idx != 1)
    {
        changeMask = ChangeNone;
        return;
    }

    if (idx == 0)
        p = pose;
    else
        p = oppositeCorner;

    desiredPos = wrapper->getPosition(changeMask);
    newPos = desiredPos;
    oldPos = p->getPosition(changeMask);
    if (useMinMax)
    {
        if (step > 0.0)
        {
            idx = (int)floor((newPos[X] - minX) / step + 0.5f);
            newPos[X] = minX + (idx * step);
            idx = (int)floor((newPos[Y] - minY) / step + 0.5f);
            newPos[Y] = minY + (idx * step);
        }

        if (newPos[X] < minX)
            newPos[X] = minX;
        else if (newPos[X] > maxX)
            newPos[X] = maxX;

        if (newPos[Y] < minY)
            newPos[Y] = minY;
        else if (newPos[Y] > maxY)
            newPos[Y] = maxY;

        if (newPos[X] != desiredPos[X] || newPos[Y] != desiredPos[Y])
            wrapperNeedsUpdate = GL_TRUE;
    }
    if (!zChangeable && (newPos[Z] != oldPos[Z]))
    {
        newPos[Z] = oldPos[Z];
        wrapperNeedsUpdate = GL_TRUE;
    }

    p->setPosition(newPos, changeMask);
    if (wrapperNeedsUpdate)
        wrapper->setPosition(newPos, changeMask);

    if (keepPointsOnTop && zChangeable)
        verticalBoost = oppositeCorner->getPosition()[Z] - pose->getPosition()[Z];
}

/*---------------------------------------------------------------------------*/
void RubberBandObject::updateWrapper(PosableStateWrapper *wrapper, long &changeMask)
{
    int idx;
    PosableStateBase *p;

    if (wrapper)
    {
        if (changeMask & ChangeIndex)
            changeMask |= ChangeTranslate;
        else
            changeMask &= ChangeTranslate;

        idx = wrapper->getIntegerIndex();
        if (idx != 0 && idx != 1)
        {
            changeMask = ChangeNone;
            return;
        }

        if (idx == 0)
            p = pose;
        else
            p = oppositeCorner;
        wrapper->beginLocalSet();
        wrapper->setPosition(p->getPosition(), changeMask);
        wrapper->endLocalSet();
    }
}

/*---------------------------------------------------------------------------*/
PosableWrapperList *RubberBandObject::findWrapperNode(PosableStateWrapper *wrapper,
                                                GLboolean mustExist,
                                                GLboolean getParentNode)
{
    if (!wrapper)
        return NULL;

    return &wrappers;
}
