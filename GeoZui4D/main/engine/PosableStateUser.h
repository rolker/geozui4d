/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// PosableStateUser.h
/*---------------------------------------------------------------------------*/

class PosableStateUser;

#ifndef POSABLESTATEUSER_H
#define POSABLESTATEUSER_H

#include <sstream>
#include "utilities/TellableInterface.h"
#include "SDV_types.h"


class PosableStateBase;
class SmartValue;

/**
 * Base for a class that can use a PosableStateBase.
 *  This is the interface for classes that use objects derived form
 *  PosableStateBase, and require notification of changes from these objects.
 *  scale properties.  Implementations of this interface are responsible for
 *  the following.
 *  \li Handling pose-change notifications.
 *
 *  \par This class actually implements the following responsibilities:
 *  \li Provide storage and access mechanism for pose
 *  \li Provide a pose changed event
 *
 *  \author Matt Plumlee (mdp)
 *  \date 1/2/2001
 *  \image html posableStateUser.jpg
 *  \image html posableStateUse.jpg
 */
/*---------------------------------------------------------------------------*/
class PosableStateUser : public TellableInterface
{
  public:
    PosableStateUser(PosableStateBase *initialPosable = 0);
    virtual ~PosableStateUser();

    PosableStateBase *getPosable();
    virtual void setPosable(PosableStateBase *newPosable);

    virtual DottedAddress *getAddress();

    virtual void PoseReplaced(PosableStateBase *oldPose, PosableStateBase *newPose) {}

    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);

    /*---Interaction facilitation---*/
    virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged) = 0;
    virtual void PoseContinuouslyChanging(PosableStateBase *whichPose, GLboolean isChanging = GL_TRUE) {};

    /*---Attribute Accessors and Mutators---*/
    virtual int getNumAttributes();
    virtual int getAttributeIndex(std::string attributeName);
    virtual std::string getAttributeName(int attributeIndex);
    virtual int getAttribute(SmartValue &val, int attributeIndex);
    virtual int setAttribute(SmartValue &val, int attributeIndex);

    virtual void getHtmlInfo(std::stringstream &html, short level);


  protected:
    PosableStateBase *pose; 
    int numAttributes;
    const char **attributeNames;
};

#endif
