/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// CompositeObject.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  This class provides a way to represent aggregate reference frames built
 *  from multiple PosableStateBases.  This is an abstract class.
 *  This class leaves the following responsibilities up to derived classes.
 *      * Managing PosableStateBases by passing change event along to them
 *      * Handling pose-change notifications.
 *  This class actually implements the following.
 *      * Keeping track of PosableStateBases for management
 *      * Removing a PosableStateBase from change notifications.
 *
 *  Created by mdp 6/21/2001
 */
/*---------------------------------------------------------------------------*/

#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "CompositeObject.h"
#include "widgets/Sv_Standards.h"
#include "engine/ObjectPoseState.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "utilities/DottedAddress.h"
#include "engine/gzCommandInterface.h"
#include <stdio.h>
#include <filesystem>

extern MainWindow *mainWindow;

#define OPERAND_LIST_QUANTUM 10

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
CompositeObject::CompositeObject(void *creator, int pickingTypes,
                                 GLboolean allocatePosable)
: PosableStateRelationship(creator),
  Object(GL_FALSE, pickingTypes)
{
    relationshipManager->registerRelationship(this); //make sure to do this before managing anyone

    poseOfOperands = NULL;
    numOperandsAllocated = numOperands = 0;
    relationshipFromOperands = NoRelationship;
    relationshipToResult = NoRelationship;

    if (allocatePosable)
        pose = new ObjectPoseState(this);
    numManaged = 1; //we're always managing the result.
}

/*---------------------------------------------------------------------------*/
CompositeObject::~CompositeObject()
{
    relinquish();
    relationshipManager->unregisterRelationship(this); //make sure to do this after unmanaging
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** The manage procedure informs this object to add the passed PosabeStateBase
 *  to its list of input objects.
 *  The return value is the "index" of the managed pose for future reference.
 *  Operand indecies start at 1.  The "index" may change when posables are
 *  relinquished.
 */
long CompositeObject::manage(PosableStateBase *poseState)
{
    if (poseState == pose)
        return 0;

    prepareListForMoreOperands();

    poseOfOperands[numOperands++] = poseState;
    relationshipManager->attach(this, poseState);
    numManaged++;

    return numOperands;
}

/*---------------------------------------------------------------------------*/
void CompositeObject::prepareListForMoreOperands()
{
    long i, newNumAllocated;
    PosableStateBase **tempList;

    if (numOperandsAllocated > numOperands)
        return;

    newNumAllocated = numOperandsAllocated + OPERAND_LIST_QUANTUM;
    tempList = new PosableStateBase *[newNumAllocated];

    //copy old members over
    for (i = 0; i < numOperandsAllocated; i++)
        tempList[i] = poseOfOperands[i];

    //NULL out the new spots
    for (; i < newNumAllocated; i ++)
        tempList[i] = NULL;

    //Make the switch
    delete poseOfOperands;
    poseOfOperands = tempList;
    numOperandsAllocated = newNumAllocated;
}

/*---------------------------------------------------------------------------*/
/** The relinquish procedure removes this CompositeObject from the
 *  notification path of the passed PosableStateBase, or of all
 *  PosableStateBases if NULL.  This procuedure may change the
 *  "index" of a posable.
 */
void CompositeObject::relinquish(PosableStateBase *poseState)
{
    int i;

    if (!poseState || poseState == pose)
        return;  //never detach from our pose.

    for (i = numOperands - 1; i >= 0; i--)
    {
        if (poseState == poseOfOperands[i])
        {
            relationshipManager->detach(this, poseOfOperands[i]);
            if (i < --numOperands)
                poseOfOperands[i] = poseOfOperands[numOperands];
            poseOfOperands[numOperands] = NULL;
            numManaged--;
        }
    }
}

/*---------------------------------------------------------------------------*/
/** The replace procedure replaces one pose with another, with no side
 *  effects.
 */
void CompositeObject::replace(PosableStateBase *oldPose, PosableStateBase *newPose)
{
    int i;

    if (!oldPose || oldPose == pose)
        return;  //never replace our pose through this function.

    for (i = numOperands - 1; i >= 0; i--)
    {
        if (oldPose == poseOfOperands[i])
        {
            poseOfOperands[i] = newPose;
            return;
        }
    }
}

/*---------------------------------------------------------------------------*/
/** The numManaging function returns how many PosableStateBases this instance
 *  is currently managing.
 */
long CompositeObject::numManaging()
{
    return numManaged;
}

/*---------------------------------------------------------------------------*/
/** getType returns the contributions to the relationship between operands
 *  and result from the passed party (fromWhom).  The associatedPose is
 *  ignored (a child class might define different relationships from
 *  different operands, but here they're assumed to all be the same).
 */
long CompositeObject::getType(long fromWhom, PosableStateBase *associatedPose)
{
    long retval = 0;

    if (fromWhom & FromOwner)
        retval |= relationshipFromOperands;
    if (fromWhom & FromTarget)
        retval |= relationshipToResult;

    return retval;
}

/*---------------------------------------------------------------------------*/
/** getPoseOfOwner returns the operand at the passed index.  If the index
 *  is -1, an arbitrary operand is passed.  If and invalid index is passed,
 *  NULL is returned.
 */
PosableStateBase *CompositeObject::getPoseOfOwner(int index)
{
    if (index == -1)
        index = 1;
    if (index < 1 || index > numOperands)
        return NULL;
    return poseOfOperands[index - 1];
}

/*---------------------------------------------------------------------------*/
/** getPoseOfTarget returns the result at the passed index.  If the index
 *  is -1, an arbitrary result is passed.  If and invalid index is passed,
 *  NULL is returned.
 */
PosableStateBase *CompositeObject::getPoseOfTarget(int index)
{
    if (index == 0 || index == -1)
        return pose;
    else
        return NULL;
}

/*-----------------------------Persistence-----------------------------------*/
/*---------------------------------------------------------------------------*/
/** This readFrom procedure assumes that the proper number of operands has
 *  been allocated, and that (numOperandsAllocated - numOperands) records
 *  should be read from the file.
 */
int CompositeObject::processField(int option, char **fields)
{
    int i = 1;
    
    switch(option)
    {
        case N_RECORDS:
            numOperandsAllocated = ConvertInt(i);
            poseOfOperands = new PosableStateBase *[numOperandsAllocated];
            break;
        default:
            return Object::processField(option, fields);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/** This readFrom procedure assumes that the proper number of operands has
 *  been allocated, and that (numOperandsAllocated - numOperands) records
 *  should be read from the file.
 */
int CompositeObject::readFrom(FILE *file)
{
    char **fields = NULL;
    PosableStateBase *newPose;

    if (Object::readFrom(file))
        return 1;

    while (numOperands < numOperandsAllocated)
    {
        fields = NextLine(GL_FALSE, GL_FALSE);
        if (fields == NULL)
        {
            fprintf(stderr, "Warning: missing operands\n");
            break;
        }
        if (fields[0][0] == '.')
        {
            DYNAMIC_CAST(newPose, PosableStateBase*, rootAddress->findItem(fields[0]));
        }
        else
        {
            DYNAMIC_CAST(newPose, PosableStateBase*, PosableNeighborhood->findItem(fields[0]));
        }
        if (!newPose)
        {
            fprintf(stderr, "Warning: could not find posable %s\n", fields[0]);
            continue;
        }
        manage(newPose);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
int CompositeObject::writeTo(FILE *file)
{
    int i;
    DottedAddress *address;
    std::string dottedName;

    Object::writeTo(file);
    for (i = 0; i < numOperands; i++)
    {
        address = poseOfOperands[i]->getAddress();
        if (address)
            dottedName = address->getAddress();
        if (!address || dottedName.empty())
        {
            fprintf(stderr, "Warning: nameless operand (%d) not written to file\n", i + 1);
            continue;
        }
        fprintf(file, "%s\n", dottedName.c_str());
    }
    return 0;
}

/*---------------------------Rendering/Info----------------------------------*/
/*---------------------------------------------------------------------------*/
void CompositeObject::DrawLowRes(float degradation, Window3D *win)
{
}

/*---------------------------------------------------------------------------*/
int CompositeObject::DrawHighRes(int eye, Window3D *win)
{
    return 1;
}

/*---------------------------------------------------------------------------*/
void CompositeObject::ShowInfo(unsigned int *names, int numNames)
{
    char infoLine[512];
    Scene3D *scene;
    int i;
    DottedAddress *address;
    std::string name;

    scene = mainWindow->getScene3D();

    sprintf(infoLine, "Type:\t%s\n", getRegistrationEntry()->className.c_str());
    scene->appendInfo(infoLine);
    sprintf(infoLine, "Name:\t%s\n", GetLabel().c_str());
    scene->appendInfo(infoLine);
    sprintf(infoLine, "File:\t%s\n", (GetFilename().empty())? "<none>" : GetFilename().c_str());
    scene->appendInfo(infoLine);
    if (numOperands > 0)
    {
        scene->appendInfo("Operands:\n");
        for (i = 0; i < numOperands; i++)
        {
            address = poseOfOperands[i]->getAddress();
            if (!address)
                continue;
            name = address->getName();
            if (name.empty())
                continue;
            scene->appendInfo("  ");
            scene->appendInfo(name);
        }
    }
    scene->appendInfo("--------------------\n");
}

/*---------------------------------------------------------------------------*/
int CompositeObject::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    const char *message = argv[0];
    char retval[2048] = "";
    PosableStateBase *oldPose = NULL, *newPose = NULL;
    Addressable *address = NULL;
    int status, i, num, first = GL_TRUE;
    DottedAddress *location;

    if (argc >= 1)
    {
        if (!strcmp(message, "replace"))
        {
            if (argc < 3)
            {
                Tcl_AppendResult(interp, "usage: replace <oldPose> <newPose>", NULL);
                return TCL_ERROR;
            }
            status = gzCommandInterface::findPosable(argv[1], address, oldPose);
            if (status != TCL_OK)
                return status;
            status = gzCommandInterface::findPosable(argv[2], address, newPose);
            if (status != TCL_OK)
                return status;
            replace(oldPose, newPose);
            return TCL_OK;
        }
        if (!strcmp(message, "manage") && argc >= 2)
        {
            if (argc < 2)
            {
                Tcl_AppendResult(interp, "usage: manage <newPose>", NULL);
                return TCL_ERROR;
            }
            status = gzCommandInterface::findPosable(argv[1], address, newPose);
            if (status != TCL_OK)
                return status;
            manage(newPose);
            return TCL_OK;
        }
        if (!strcmp(message, "relinquish") && argc >= 2)
        {
            if (argc < 2)
            {
                Tcl_AppendResult(interp, "usage: relinquish <oldPose>", NULL);
                return TCL_ERROR;
            }
            status = gzCommandInterface::findPosable(argv[1], address, oldPose);
            if (status != TCL_OK)
                return status;
            relinquish(oldPose);
            return TCL_OK;
        }
        if (!strcmp(message, "numManaging"))
        {
            sprintf(retval, "%ld", numManaging());
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "listManaged"))
        {
            num = numManaging();
            for (i = 0; i < num; i++)
            {
                newPose = poseOfOperands[i];
                if (newPose)
                {
                    location = newPose->getAddress();
                    if (first)
                        first = GL_FALSE;
                    else
                        strcat(retval, " ");
                    if (location)
                        strcat(retval, location->getAddress().c_str());
                    else
                        strcat(retval, "{}");
                }
            }
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            return TCL_OK;
        }
    }
    return Object::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int CompositeObject::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "manage relinquish replace numManaging listManaged ", NULL);
    return Object::list(interp);
}
