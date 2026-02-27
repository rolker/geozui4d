/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// InterpolationSupport.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  This is a PosableWrapperSupport that provides a way to index between
 *  integer index points.  Currently, only cubic splines and linear
 *  interpolation are supported.
 *
 *  Created by mdp 2/19/2002
 */
/*---------------------------------------------------------------------------*/
#include "engine/PosableStateWrapper.h"
#include "engine/InterpolationSupport.h"
#include "PosableStateUser.h"
#include "engine/InputMediator.h"
#include "utilities/CubicSpline.h"
#include "utilities/InverseCubicSpline.h"
#include "utilities/ForwardCubicSpline.h"
#include <string.h>

const EventType *InterpolationSupport::interpolationBoundsInputType =
                Interactor::newEventType("interpolationBounds", Interactor::DataAsAddress);

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
InterpolationSupport::InterpolationSupport()
{
    wrappers = NULL;
    splineRepresentations = NULL;
    invsplineRepresentations = NULL;
    attribSplineRepresentations = NULL;
    numLines = 0;
    setNumLines(1);
    numAttachedToAttribute = 0;
}

/*---------------------------------------------------------------------------*/
InterpolationSupport::~InterpolationSupport()
{
    unsigned int i, j;

    //clean the wrapper list
    for (i = 0; i < numLines; i++)
    {
        if (wrappers[i])
        {
            while (wrappers[i]->next)
                wrappers[i]->removeWrapper(wrappers[i]->wrapper);
            delete wrappers[i];
        }

        if (splineRepresentations[i])
            delete splineRepresentations[i];

        if (attribSplineRepresentations)
        {
            for (j = 0; j < numWrapableAttributes; j++)
            {
                if (attribSplineRepresentations[j][i])
                    delete attribSplineRepresentations[j][i];
            }
        }
        if (invsplineRepresentations[i])
            delete invsplineRepresentations[i];
    }
    delete wrappers;
    delete [] splineRepresentations;
    if (attribSplineRepresentations)
    {
        for (j = 0; j < numWrapableAttributes; j++)
        {
            if (attribSplineRepresentations[j])
                delete [] attribSplineRepresentations[j];
        }
    }
    delete [] invsplineRepresentations;
}

/*-----------------------------------Mutators--------------------------------*/
/*---------------------------------------------------------------------------*/
void InterpolationSupport::setNumLines(unsigned int nLines)
{
    unsigned int i, attrib, lesser;
    CubicSpline **newSplines;
    InverseCubicSpline **newInvSplines;
    PosableWrapperList **newWrappers;
    ForwardCubicSpline ***newAttribSplines = NULL;

    if (nLines == numLines)
        return;

    newSplines = new CubicSpline*[nLines];
    if (attribSplineRepresentations)
    {
        newAttribSplines = new ForwardCubicSpline**[numWrapableAttributes];
        for (attrib = 0; attrib < numWrapableAttributes; attrib++)
            newAttribSplines[attrib] = new ForwardCubicSpline*[nLines];
    }
    newInvSplines = new InverseCubicSpline*[nLines];
    newWrappers = new PosableWrapperList *[nLines];

    lesser = numLines;
    if (lesser > nLines)
        lesser = nLines;

    for (i = 0; i < lesser; i++)
    {
        newSplines[i] = splineRepresentations[i];
        if (attribSplineRepresentations)
        {
            for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                newAttribSplines[attrib][i] = attribSplineRepresentations[attrib][i];
        }
        newInvSplines[i] = invsplineRepresentations[i];
        newWrappers[i] = wrappers[i];
    }
    if (nLines < numLines)
    {
        for (;i < numLines; i++)
        {
            delete [] splineRepresentations[i];
            if (attribSplineRepresentations)
            {
                for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                    delete attribSplineRepresentations[attrib][i];
            }
            delete [] invsplineRepresentations[i];
            delete [] wrappers[i];
        }
    }
    else
    {
        for (;i < nLines; i++)
        {
            newSplines[i] = NULL;
            newInvSplines[i] = NULL;
            if (attribSplineRepresentations)
            {
                for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                    newAttribSplines[attrib][i] = NULL;
            }
            newWrappers[i] = NULL;
        }
    }
    wrappers = newWrappers;
    splineRepresentations = newSplines;
    if (attribSplineRepresentations)
        attribSplineRepresentations = newAttribSplines;
    invsplineRepresentations = newInvSplines;
    numLines = nLines;
}

/*---------------------------------------------------------------------------*/
/** This procedure initializes the attribSplineRepresentations array by allocating
 *  all necessary pointers, but no actual splines.  Once initialized, the other
 *  functions will properly handle insertions and deletions of individual points.
 *  This function only works on a NULL attribSplineRepresentaitons pointer--it
 *  does not resize an existing array (so if someone implements adding and deleting
 *  attributes, you'll have to write the resize yourslf, or at least write something
 *  to destroy the array, then call this procedure).
 */
void InterpolationSupport::initAttribSplineRepresentations()
{
    unsigned int attrib, i;

    if (attribSplineRepresentations)
        return;

    attribSplineRepresentations = new ForwardCubicSpline**[numWrapableAttributes];
    for (attrib = 0; attrib < numWrapableAttributes; attrib++)
    {
        attribSplineRepresentations[attrib] = new ForwardCubicSpline*[numLines];
        for (i = 0; i < numLines; i++)
            attribSplineRepresentations[attrib][i] = NULL;
    }
}

/*-----------------------------------Accessors-------------------------------*/
/*---------------------------------------------------------------------------*/
GLboolean InterpolationSupport::indexContentsModifiable(PosableStateWrapper *wrapper)
{
    if (wrapper->getFloatIndex() == wrapper->getIntegerIndex())
        return GL_TRUE;
    return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
PosableWrapperList *InterpolationSupport::findWrapperNode(PosableStateWrapper *wrapper,
                                                          GLboolean mustExist,
                                                          GLboolean getParentNode)
{
    PosableWrapperList *returnList = NULL;
    int lastPoint, lineIndex;
    float index;

    if (!wrapper)
        return NULL;

    lineIndex = wrapper->getIntegerIndex(2);
    lastPoint = getNumPoints(lineIndex) - 1;
    index = wrapper->getFloatIndex();
    if (!getParentNode && (index < 0 || index > lastPoint)) //is it a valid index?
        return NULL;

    if (!wrappers[lineIndex])
    {
        if (!mustExist)
        {
            wrappers[lineIndex] = new PosableWrapperList;
            return wrappers[lineIndex];
        }
    }
    else
        returnList = wrappers[lineIndex]->findWrapper(wrapper);

    if (!mustExist && !returnList)
        return wrappers[lineIndex];
    else if (getParentNode && returnList)
        return wrappers[lineIndex];
    else
        return returnList;
}

/*---------------------------------------------------------------------------*/
CubicSpline *InterpolationSupport::getSplineRepresentation(unsigned int whichLine)
{
    if (whichLine >= numLines || whichLine < 0)
        return NULL;

    if (!splineRepresentations[whichLine])
        splineRepresentations[whichLine] = new CubicSpline(getPoints(whichLine));

    return splineRepresentations[whichLine];
}

/*---------------------------------------------------------------------------*/
InverseCubicSpline *InterpolationSupport::getInverseSplineRepresentation(unsigned int whichLine)
{
    if (whichLine >= numLines || whichLine < 0 || associativeAttribute < 0)
        return NULL;

    if (!invsplineRepresentations[whichLine])
        invsplineRepresentations[whichLine] = new InverseCubicSpline(getAttributeValues(whichLine),
                                                          getNumPoints(whichLine));

    return invsplineRepresentations[whichLine];
}

/*-----------------------------Attributes------------------------------------*/
/*---------------------------------------------------------------------------*/
int InterpolationSupport::getAttribute(PosableStateWrapper *wrapper, SmartValue &val, int attributeIndex)
{
    float index, *values, newValue, highFactor, lowFactor;
    unsigned int numPoints, lineIndex, intIndex;
    const char *interpolationType;
    ForwardCubicSpline *currentSpline;

    index = 0.0;
    
    if (wrapper)
    {
        lineIndex = wrapper->getIntegerIndex(2);
        if (lineIndex >= numLines)
            return TCL_ERROR;
        numPoints = getNumPoints(lineIndex);
        index = wrapper->getFloatIndex(); //use this to give a hint as to where to find next index.

        values = getAttributeValues(lineIndex, attributeIndex);
        if (!values)
            return TCL_ERROR;
        interpolationType = wrapper->getStringIndex(1);
        if (interpolationType && strcmp(interpolationType, "cubic_spline") == 0)
        {
            if (!attribSplineRepresentations)
                initAttribSplineRepresentations();
            if (!attribSplineRepresentations[attributeIndex][lineIndex])
                attribSplineRepresentations[attributeIndex][lineIndex] = new ForwardCubicSpline(values, numPoints);
            currentSpline = attribSplineRepresentations[attributeIndex][lineIndex];
            currentSpline->interpolateAt(index, newValue);
        }
        else
        {
            intIndex = (int)index;
            highFactor = index - intIndex;
            lowFactor = 1.0f - highFactor;
            newValue = lowFactor * values[intIndex] + highFactor * values[intIndex + 1];
        }
        val.set(newValue);
        return TCL_OK;
    }
    return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int InterpolationSupport::setAttribute(PosableStateWrapper *wrapper,
                                       SmartValue &val, int attributeIndex)
{
    int index, lineIndex, numPoints;

    lineIndex = wrapper->getIntegerIndex(2);
    numPoints = getNumPoints(lineIndex);
    if (wrapper->getIndexType() == SmartValue::String && (strcmp(wrapper->getStringIndex(), "end") == 0))
        index = numPoints - 1;
    else
        index = wrapper->getIntegerIndex();

    if (attributeIndex == associativeAttribute && invsplineRepresentations[lineIndex])
        invsplineRepresentations[lineIndex]->pointUpdated(index);

    if (attribSplineRepresentations && attribSplineRepresentations[attributeIndex][lineIndex])
        attribSplineRepresentations[attributeIndex][lineIndex]->pointUpdated(index);
    return TCL_OK;
}

/*---------------------------Change Facilitation-----------------------------*/
/*---------------------------------------------------------------------------*/
/** This function makes sure the changed value is valid, and updates the
 *  internal representation to match.
 */
void InterpolationSupport::wrapperChangedValue(PosableStateWrapper *wrapper,
                                               long &changeMask)
{
    int lastPoint;
    int index, lineIndex, numPoints;
    PosableStateBase *offsetPose;
    
    index = 0;

    if (!wrapper)
    {
        changeMask = ChangeNone;
        return;
    }

    lineIndex = wrapper->getIntegerIndex(2);
    numPoints = getNumPoints(lineIndex);
    if (wrapper->getIndexType() == SmartValue::String && (strcmp(wrapper->getStringIndex(), "end") == 0))
        index = numPoints - 1;
    else
        index = wrapper->getIntegerIndex();
    lastPoint = getNumPoints(lineIndex) - 1;
    if (index < 0 || index > lastPoint
        || index != wrapper->getFloatIndex()) //is it a valid index?
    {
        changeMask = ChangeNone;
        return;
    }
    
    gz4d::Point<double> setValue = wrapper->getPosition(changeMask);
    offsetPose = getPosable();
    gz4d::Point<double> offset = offsetPose->getPosition(changeMask);
    if (!modifyPoint(index, setValue - offset, changeMask, lineIndex))
    {
        changeMask = ChangeNone;
        return;
    }
    if (splineRepresentations[lineIndex] && (changeMask & ChangeTranslateMask))
        splineRepresentations[lineIndex]->pointUpdated(index);
}

/*---------------------------------------------------------------------------*/
void InterpolationSupport::updateWrapper(PosableStateWrapper *wrapper, long &changeMask)
{
    gz4d::Point<double> newValue;
    double index, lowFactor, highFactor;
    unsigned int intIndex, numPoints, lineIndex;
    const char *interpolationType;
    PosableStateBase *offsetPose;

    index = 0.0;
    
    if (wrapper)
    {
        if (changeMask & ChangeIndex)
            changeMask |= ChangeTranslate;
        else
            changeMask &= ChangeTranslate;
        lineIndex = wrapper->getIntegerIndex(2);
        numPoints = getNumPoints(lineIndex);
        if (wrapper->getIndexType() == SmartValue::String && (strcmp(wrapper->getStringIndex(), "end") == 0))
        {
            wrapper->setIndex(int(numPoints - 1));
            return;
        }
        else
            index = wrapper->getFloatIndex();

        if (index < 0 || index > (numPoints - 1))
        {
            changeMask = ChangeNone;
            if (index > 0 && lineIndex >= numLines - 1)
                highLevelEventMediator->Trigger(interpolationBoundsInputType->value, wrapper, (void*)2);
            else
                highLevelEventMediator->Trigger(interpolationBoundsInputType->value, wrapper, (void*)0);
            return;
        }
        interpolationType = wrapper->getStringIndex(1);
        offsetPose = getPosable();
        gz4d::Point<double> offset = offsetPose->getPosition();
        if (interpolationType && strcmp(interpolationType, "cubic_spline") == 0)
        {
            if (!splineRepresentations[lineIndex])
                splineRepresentations[lineIndex] = new CubicSpline(getPoints(lineIndex));
            splineRepresentations[lineIndex]->interpolateAt(index, newValue);
            newValue += offset;
            wrapper->beginLocalSet();
            wrapper->setPosition(newValue, changeMask);
            wrapper->endLocalSet();
        }
        else
        {
            intIndex = (int)index;
            highFactor = index - intIndex;
            lowFactor = 1.0f - highFactor;
            gz4d::Point<double> lowPoint = getPoints(lineIndex)[intIndex];
            gz4d::Point<double> highPoint = getPoints(lineIndex)[intIndex + 1];
            newValue = lowPoint * lowFactor + highPoint * highFactor + offset;
            wrapper->beginLocalSet();
            wrapper->setPosition(newValue, changeMask);
            wrapper->endLocalSet();
        }
        return;
    }
}

/*---------------------------------------------------------------------------*/
/** Child class should redefine this function to make any updates necessary
 *  when a point changes its position.  This implementation makes the change
 *  to the point itself without other side effects.  Returns GL_TRUE if
 *  the index is valid and the newValue was taken, but returns GL_FALSE
 *  if the index is invalid (and none of the points change).  The
 *  changeMask is modified to reflect what values actually changed
 *  if the newValue was taken (or partially taken).
 */
GLboolean InterpolationSupport::modifyPoint(unsigned int index, gz4d::Point<double> const &newValue,
                                            long &changeMask, unsigned int whichLine)
{
    if (index < 0 || index >= getPoints(whichLine).size())
        return GL_FALSE;
    if (changeMask & ChangeTranslateXMask)
        getPoints(whichLine)[index][X] = newValue[X];
    if (changeMask & ChangeTranslateYMask)
        getPoints(whichLine)[index][Y] = newValue[Y];
    if (changeMask & ChangeTranslateZMask)
        getPoints(whichLine)[index][Z] = newValue[Z];
    changeMask &= ChangeTranslateMask;
    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/** returns GL_TRUE if the value could be inserted, GL_FALSE otherwise.
 *  Child classes are expected to implement insertItem such that it
 *  creates all memory necessary to instantiate the specified item.
 */
GLboolean InterpolationSupport::insertValue(PosableStateWrapper *wrapper,
                                            GLboolean beforeCurrent, int dim)
{
    unsigned int lineIndex, numItems, type, index, retval, oldNumLines, i, newIndex, attrib;
    PosableWrapperList *wrapperList;
    PosableStateWrapper *w;

  
    if (wrapper && (dim == 0 || dim == 2))
    {
      oldNumLines = numLines;
        lineIndex = wrapper->getIntegerIndex(2);
        if (dim == 0)
        {
            type = PointType;
            numItems = getNumPoints(lineIndex);
        }
        else
        {
            type = LineType;
            oldNumLines = numItems = numLines;
        }
        if (wrapper->getIndexType() == SmartValue::String && (strcmp(wrapper->getStringIndex(), "end") == 0))
            wrapper->setIndex(int(numItems - 1), dim);
        index = wrapper->getIntegerIndex();

        if (!beforeCurrent)
        {
            if (type == LineType)
                lineIndex++;
            else
                index++;
        }
        if (index < 0) //we can allow multiple points to get created by having a point way out...
            return GL_FALSE;

        retval = insertItem(type, index, lineIndex);

        //update the wrapper and spline knowledge
        if (retval == GL_TRUE)
        {
            if (type == LineType)
            {
                //resize
                if (lineIndex > numItems)
                    setNumLines(lineIndex + 1);
                else
                    setNumLines(numItems + 1);
                //bump up if necessary.
                if (lineIndex < oldNumLines)
                {
                    for (i = oldNumLines; i > lineIndex; i--)
                    {
                        wrapperList = wrappers[i] = wrappers[i - 1];
                        splineRepresentations[i] = splineRepresentations[i - 1];
                        if (attribSplineRepresentations)
                        {
                            for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                                attribSplineRepresentations[attrib][i] = attribSplineRepresentations[attrib][i - 1];
                        }
                        invsplineRepresentations[i] = invsplineRepresentations[i - 1];
                        while (wrapperList)
                        {
                            w = wrapperList->wrapper;
                            w->setIndex(w->getIntegerIndex(2) + 1, 2);
                            wrapperList = wrapperList->next;
                        }
                    }
                    wrappers[lineIndex] = NULL;
                    splineRepresentations[lineIndex] = NULL;
                    if (attribSplineRepresentations)
                    {
                        for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                            attribSplineRepresentations[attrib][lineIndex] = NULL;
                    }
                    invsplineRepresentations[lineIndex] = NULL;
                }
            }
            else if (type == PointType)
            {
                //bump everyone's index up as necessary
                wrapperList = wrappers[lineIndex];
                while (wrapperList)
                {
                    w = wrapperList->wrapper;
                    if (w != wrapper)
                    {
                        newIndex = w->getIntegerIndex();
                        if (newIndex >= index)
                            w->setIndex(int(newIndex + 1));
                    }
                    wrapperList = wrapperList->next;
                }
                if (splineRepresentations[lineIndex])
                    splineRepresentations[lineIndex]->setControlPoints(getPoints(lineIndex));
                if (attribSplineRepresentations)
                {
                    for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                    {
                        if (attribSplineRepresentations[attrib][lineIndex])
                            attribSplineRepresentations[attrib][lineIndex]->setControlPoints(getAttributeValues(lineIndex, attrib), getNumPoints(lineIndex));
                    }
                }
                if (invsplineRepresentations[lineIndex])
                    invsplineRepresentations[lineIndex]->setControlPoints(getAttributeValues(lineIndex), getNumPoints(lineIndex));
            }
        }
        return retval;
    }
    return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/** returns GL_TRUE if the value could be deleted, GL_FALSE otherwise.
 *  Child classes are expected to implement deleteItem such that it
 *  moves everything over such that there are no holes.
 */
GLboolean InterpolationSupport::deleteValue(PosableStateWrapper *wrapper, int dim)
{
    unsigned int lineIndex, numItems, type, index, retval, newIndex, attrib;
    PosableWrapperList *wrapperList;
    PosableStateWrapper *w;

    if (wrapper && (dim == 0 || dim == 2))
    {
        lineIndex = wrapper->getIntegerIndex(2);
        if (dim == 0)
        {
            type = PointType;
            numItems = getNumPoints(lineIndex);
        }
        else
        {
            type = LineType;
            numItems = numLines;
        }

        if (wrapper->getIndexType() == SmartValue::String)
        if (wrapper->getIndexType() == SmartValue::String && (strcmp(wrapper->getStringIndex(), "end") == 0))
            wrapper->setIndex(int(numItems - 1), dim);
        if (type == LineType)
            index = -1;
        else
            index = wrapper->getIntegerIndex();

        retval = deleteItem(type, index, lineIndex);

        //update the wrapper and spline knowledge
        if (retval == GL_TRUE)
        {
            if (type == LineType)
            {
                unsigned int i;

                //clean the current wrapper list
                while (wrappers[lineIndex] && wrappers[lineIndex]->next)
                    wrappers[lineIndex]->removeWrapper(wrappers[lineIndex]->wrapper);
                delete wrappers[lineIndex];

                //clean the current spline info
                if (splineRepresentations[lineIndex])
                    delete splineRepresentations[lineIndex];
                if (attribSplineRepresentations)
                {
                    for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                    {
                        if (attribSplineRepresentations[attrib][lineIndex])
                            delete attribSplineRepresentations[attrib][lineIndex];
                    }
                }
                if (invsplineRepresentations[lineIndex])
                    delete invsplineRepresentations[lineIndex];

                //bump everyone else down
                for (i = lineIndex + 1; i < numLines; i++)
                {
                    wrapperList = wrappers[i - 1] = wrappers[i];
                    splineRepresentations[i - 1] = splineRepresentations[i];
                    if (attribSplineRepresentations)
                    {
                        for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                            attribSplineRepresentations[attrib][i - 1] = attribSplineRepresentations[attrib][i];
                    }
                    invsplineRepresentations[i - 1] = invsplineRepresentations[i];
                    while (wrapperList)
                    {
                        w = wrapperList->wrapper;
                        w->setIndex(w->getIntegerIndex(2) - 1, 2);
                        wrapperList = wrapperList->next;
                    }
                }
                wrappers[numLines - 1] = NULL;
                splineRepresentations[numLines - 1] = NULL;
                if (attribSplineRepresentations)
                {
                    for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                        attribSplineRepresentations[attrib][numLines - 1] = NULL;
                }
                invsplineRepresentations[numLines - 1] = NULL;
                setNumLines(numLines - 1);
            }
            else if (type == PointType)
            {
                //bump everyone's index down as necessary
                wrapperList = wrappers[lineIndex];
                while (wrapperList)
                {
                    w = wrapperList->wrapper;
                    newIndex = w->getIntegerIndex();
                    if (newIndex > index)
                        w->setIndex(int(newIndex - 1));
                    wrapperList = wrapperList->next;
                }
                if (splineRepresentations[lineIndex])
                    splineRepresentations[lineIndex]->setControlPoints(getPoints(lineIndex));
                if (attribSplineRepresentations)
                {
                    for (attrib = 0; attrib < numWrapableAttributes; attrib++)
                    {
                        if (attribSplineRepresentations[attrib][lineIndex])
                            attribSplineRepresentations[attrib][lineIndex]->setControlPoints(getAttributeValues(lineIndex, attrib), getNumPoints(lineIndex));
                    }
                }
                if (invsplineRepresentations[lineIndex])
                    invsplineRepresentations[lineIndex]->setControlPoints(getAttributeValues(lineIndex), getNumPoints(lineIndex));
            }
        }
        return retval;
    }
    return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/** If the attributeIndex is valid, sets things up to service calls to
 *  setWrapperIndexFromAttribute and returns GL_TRUE.  Otherwise returns
 *  GL_FALSE.
 */
GLboolean InterpolationSupport::requestAssociativeAttribute(unsigned int attributeIndex)
{
    if (attributeIndex < 0 || attributeIndex > numWrapableAttributes)
        return GL_FALSE;

    //only allow one indexed attribute at a time.
    if (numAttachedToAttribute > 0 && associativeAttribute != (int)attributeIndex)
        return GL_FALSE;

    associativeAttribute = attributeIndex;
    numAttachedToAttribute++;
    return GL_TRUE;  //maybe later we'll restrict what works and what doesn't
                     //in terms of type and monotonicity.
}

/*---------------------------------------------------------------------------*/
void InterpolationSupport::releaseAssociativeAttribute(unsigned int attributeIndex)
{
    if (associativeAttribute == int(attributeIndex))
    {
        numAttachedToAttribute--;
        if (numAttachedToAttribute < 0)
            numAttachedToAttribute = 0;
    }
}

/*---------------------------------------------------------------------------*/
/** If the associativeAtrribute is valid, sets the passed wrapper's index to
 *  an index that has the passed value for the associativeAttribute and
 *  returns GL_TRUE.  Otherwise returns GL_FALSE.  If the value is outside the
 *  range of the attribute, an interpolationBounds event is triggered.
 */
GLboolean InterpolationSupport::setWrapperIndexFromAttribute(PosableStateWrapper *wrapper,
                                                             float attrVal)
{
    float index, *values, newValue;
    unsigned int numPoints, lineIndex, highIndex, lowIndex, splitDiff;
    const char *interpolationType;
    InverseCubicSpline *currentInvSpline;

    index = 0.0;
    
    if (wrapper)
    {
        lineIndex = wrapper->getIntegerIndex(2);
        numPoints = getNumPoints(lineIndex);
        index = wrapper->getFloatIndex(); //use this to give a hint as to where to find next index.

        values = getAttributeValues(lineIndex);
        if (!values)
            return GL_FALSE;
        interpolationType = wrapper->getStringIndex(1);
        //pick up where we left off, or reset the search
        lowIndex = wrapper->associativeIndexLow;
        highIndex = wrapper->associativeIndexHigh;
        if (lowIndex < 0 || highIndex > numPoints ||
            highIndex <= lowIndex)
        {
            lowIndex = 0;
            highIndex = numPoints - 1;
        }
        if (interpolationType && strcmp(interpolationType, "cubic_spline") == 0)
        {
            if (lineIndex >= numLines)
            {
                highLevelEventMediator->Trigger(interpolationBoundsInputType->value, wrapper, (void*)2);
                return GL_TRUE;
            }
            if (!invsplineRepresentations[lineIndex])
                invsplineRepresentations[lineIndex] = new InverseCubicSpline(values, numPoints);
            currentInvSpline = invsplineRepresentations[lineIndex];
            currentInvSpline->previous_tLowIndex = lowIndex;
            currentInvSpline->previous_tHighIndex = highIndex;
            if (!currentInvSpline->interpolateAt(attrVal, newValue))
            {
                if (attrVal > values[0] && lineIndex >= numLines - 1)
                    highLevelEventMediator->Trigger(interpolationBoundsInputType->value, wrapper, (void*)2);
                else
                    highLevelEventMediator->Trigger(interpolationBoundsInputType->value, wrapper, (void*)0);
                return GL_TRUE;
            }
            //remember where we were (in case another wrapper is attaching similarly)
            wrapper->associativeIndexLow = currentInvSpline->previous_tLowIndex;
            wrapper->associativeIndexHigh = currentInvSpline->previous_tHighIndex;
        }
        else
        {
            //binary search for proper index;
            while (highIndex - lowIndex > 1)
            {
                splitDiff = (highIndex + lowIndex) >> 1;
                if (attrVal < values[splitDiff])
                    highIndex = splitDiff;
                else
                    lowIndex = splitDiff;
            }
            newValue = lowIndex + (attrVal - values[lowIndex]) / (values[highIndex] - values[lowIndex]);
        }
        if (newValue < 0) //don't let it get out of bounds backwards!
            newValue = 0.0;
        wrapper->setIndex(newValue);
        return GL_TRUE;
    }
    return GL_FALSE;
}
