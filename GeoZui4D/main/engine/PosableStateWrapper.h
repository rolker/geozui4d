/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// PosableStateWrapper.h
/*---------------------------------------------------------------------------*/

class PosableStateWrapper;

#ifndef POSABLE_STATE_WRAPPER_H
#define POSABLE_STATE_WRAPPER_H

#include "engine/PosableStateBase.h"
#include "utilities/SmartValue.h"

struct PosableWrapperList;
class PosableWrapperSupport;

/** 
 *  This is the interface for classes that act as proxies for accessing
 *  position, orientation, and scale properties of data that is not already
 *  modeled in a PosableStateBase.  
 *  \par Implementations of this interface are responsible for the following.
 *   Default behaviors for most of these are provided.
 *  \li Allowing for modification and retrieval of pose information
 *  \li Applying proper transformation for display (although this is
 *        unlikely to be used).
 *  \li Respond to queries about preferred poses, focus, and context
 *  \li Provide hooks that allow other objects to respond to changes
 *        in this PosableStateBase's pose
 *  \li Allow for highlighting this PosableStateBase.
 *  \par This class actually implements the following responsibilities:
 *  \li Keep track of the index into the source data.
 *  \li Provide a pose changed notification to user.
 *  \li Retain a counter for being in flux (continuously changing)
 *  \note that this class can be used to add wrapper support to existing
 *  posable types by deriving a new class from this and the posable type,
 *  but declaring the derivation of this class as virtual.
 *
 *  \author Matt Plumlee (mdp)
 *  \date 2/12/2002
 *  \image html wrapperDef.jpg
 *  \image html wrapperUse.jpg
 *  \image html posableStateWrappers.jpg
 *  \image html posableStateWrapUse.jpg
 *  \image html posableStateWrapUse2.jpg
 */
/*---------------------------------------------------------------------------*/
class PosableStateWrapper : virtual public PosableStateBase
{
  public:
    /*---Constructors/Destructors---*/
      PosableStateWrapper(PosableStateUser *initialUser = NULL, std::string name = "");
    PosableStateWrapper(PosableStateBase *toBeReplaced);
    virtual ~PosableStateWrapper();

    /*---Accessors---*/
    int getNumDimensions() {return numDimensions;}
    SmartValue::IndexType getIndexType(int dim = 0);
    int getIntegerIndex(int dim = 0);
    float getFloatIndex(int dim = 0);
    const char *getStringIndex(int dim = 0);
    const void *getPointerIndex(int dim = 0);
    void getIndecies(const void **values, int *dimensions = NULL, SmartValue::IndexType *types = NULL);
    GLboolean isAttached() {return support? GL_TRUE : GL_FALSE;}

    /*---attribute stuff---*/
    virtual int getNumAttributes();
    virtual int getAttributeIndex(std::string attributeName);
    virtual std::string getAttributeName(int attributeIndex);
    virtual int getAttribute(SmartValue &val, int attributeIndex);
    virtual int setAttribute(SmartValue &val, int attributeIndex);
    virtual GLboolean setIndexingAttribute(int attributeIndex);
    virtual GLboolean indexByAttributeValue(float value);
    virtual void resetTimeBoundsOnAttribute(double offset = 0.0, double scale = 1.0);
    int getIndexingAttribute() {return indexingAttribute + numDimensions;}
    int associativeIndexLow, associativeIndexHigh; //for optimizations in associative indexing

    /*---Scripting Interface---*/
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);

    /*---Bookkeeping---*/
    PosableWrapperList *getList() {return sourceList;}
    void setList(PosableWrapperList *list) {sourceList = list;}

    /*---Mutators---*/
    void attachTo(PosableWrapperSupport *newSupport);
    void detach();
    void setNumDimensions(int newCount);
    int setIndecies(const void **values, int *dimensions, SmartValue::IndexType *types = NULL);//multi-dimensional set
    int setIndex(int val, int dim = 0);
    int setIndex(float val, int dim = 0);
    int setIndex(const char *val, int dim = 0);
    int setIndex(const void *val, int dim = 0);
    int incrementIndex(float amount, int dim = 0);
    int insertValue(GLboolean beforeCurrent = GL_TRUE, int dim = 0);
    int deleteValue(int dim = 0);

    void beginLocalSet() {settingLocally = GL_TRUE;}
    void endLocalSet() {settingLocally = GL_FALSE;}

    void delayIndexNotification() {if (++holdIndexNotify < 0) holdIndexNotify = 1;}
    void resumeIndexNotification(GLboolean notifyNow = GL_TRUE) {if (!--holdIndexNotify && notifyNow) NotifyIndexChange();}

    /*---Interaction facilitation---*/
    virtual void update(long changeMask);
    static unsigned int wrapperPoseChangedEvent() { return wrapperPoseChangedInputType->value;}

    /*---GL Transformations---*/
    //virtual void applyTransforms();

  protected:
    PosableWrapperList *sourceList;
    PosableWrapperSupport *support;
    SmartValue *indecies;
    int numDimensions;
    int indexingAttribute; //stored as index straight into support, rather than exported index
    GLboolean originator, settingLocally;
    int holdIndexNotify;
    //GLboolean infoUpToDate;

    virtual void replace(PosableStateBase *toBeReplaced);

    /*---Interaction facilitation---*/
    static const EventType *wrapperPoseChangedInputType;
    virtual void NotifyPose(long changeMask);
    virtual void NotifyIndexChange() {update(ChangeIndex);} //call this when index changes
};

#endif
