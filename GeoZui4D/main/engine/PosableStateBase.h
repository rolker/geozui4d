/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// PosableStateBase.h
/*---------------------------------------------------------------------------*/

class PosableStateBase;

#ifndef POSABLE_STATE_BASE_H
#define POSABLE_STATE_BASE_H

#include<sstream>


#include "engine/PosableState.h"
#include "utilities/Addressable.h"
#include "engine/Interactor.h"

#include "SDV_types.h"
#include <gz4d/Point.h>

class PosableStateUser;
class SmartValue;
class ViewManager;

/** 
 *	This is the interface for classes that may have position, orientation, and
 *	scale properties.  
 *  \note Implementations of this interface are responsible for
 *	the following.  Default behaviors for most of these are provided.
 *	\li Allowing for modification and retrieval of pose information
 *	\li Applying proper transformation for display
 *	\li Respond to queries about preferred poses, focus, and context
 *	\li Provide hooks that allow other objects to respond to changes
 *		  in this PosableStateBase's pose
 *	\li Allow for highlighting this PosableStateBase.
 *	\par This class actually implements the following responsibilities:
 *	\li Provide a pose changed notification to user.
 *	\li Retain a counter for being in flux (continuously changing)
 *
 *	\author Matt Plumlee (mdp)
 *  \date 12/13/2000
 *  \image html posableDef.jpg
 *  \image html posableUse.jpg
 *  \image html posableChange.jpg
 */
/*---------------------------------------------------------------------------*/
class PosableStateBase : public Addressable
{
  public:
	/*---Constructors/Destructors---*/
      PosableStateBase(PosableStateUser *initialUser = NULL, std::string name = "");
	PosableStateBase(PosableStateBase *toBeReplaced);
        void init();
        
        virtual void publishName(std::string newName);
	virtual ~PosableStateBase();
	static unsigned int PosableDeletedEvent() { return posableDeletedInputType->value;}

	/*---Accessors---*/
	//Use the changeMask arguments to get at min, max, and preferred data
        virtual void getPoseInfo(gz4d::Point<double> &position, OrientationInfo &rot, double &scale, int changeMask = ChangeAll);
	virtual void getPoseInfo(PosableStateInfo &info, int changeMask = ChangeAll);

        virtual gz4d::Point<double> getPosition(int changeMask = ChangeTranslate);

	virtual double getOrientationMagnitude(int changeMask = ChangeRotate);
        virtual gz4d::Point<double> getOrientationAxis(int changeMask = ChangeRotate);
	virtual const OrientationInfo *getOrientation(int changeMask = ChangeRotate);

	virtual double getScale(int changeMask = ChangeScale);

	virtual int supportMask(int changeMask = ChangeAll) = 0;

	/*---Preference Accessors---*/
        virtual void getFocusInfoUsing(gz4d::Point<double> &position, OrientationInfo &rot, double &scale);
        virtual void getContextInfoUsing(gz4d::Point<double> &position, OrientationInfo &rot, double &scale);

	/*---Scripting Interface---*/
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);
        virtual void getHtmlInfo(std::stringstream &html, short level);

	/*---Attribute Accessors and Mutators---*/
	virtual int getNumAttributes();
        virtual int getAttributeIndex(std::string attributeName);
        virtual std::string getAttributeName(int attributeIndex);
	virtual int getAttribute(SmartValue &val, int attributeIndex);
	virtual int setAttribute(SmartValue &val, int attributeIndex);

	/*---Mutators---*/
        virtual void setPosition(gz4d::Point<double> const &p, int changeMask = ChangeTranslate);
	virtual void setPosition(double valx = 0.0, double valy = 0.0, double valz = 0.0, int changeMask = ChangeTranslate);
	virtual void setPosition(const double *val, int changeMask = ChangeTranslate);
	virtual void setX(double valx);
	virtual void setY(double valy);
	virtual void setZ(double valz);
	virtual void setOrientation(const OrientationInfo &rot, int changeMask = ChangeRotate);
	virtual void setOrientation(double pitchAngle, double yawAngle, int changeMask = ChangeRotate);
	virtual void setScale(double sval, int changeMask = ChangeScale);

	virtual void moveBy(double valx = 0.0, double valy = 0.0, double valz = 0.0, int changeMask = ChangeTranslate);
        virtual void moveBy(gz4d::Point<double> const &val, int changeMask = ChangeTranslate);
	virtual void rotateBy(double angx = 0, double angy = 0, double angz = 0, int changeMask = ChangeRotate);
	virtual void rotateBy(const OrientationInfo &rot, int changeMask = ChangeRotate);
	virtual void scaleBy(double sval = 1.0, int changeMask = ChangeScale);

//	virtual PosableStateBase &operator =(const PosableStateBase &posable);

	/*---GL Transformations---*/
	/** Applies the transformation to the current view matrix.
	If a view is passed in, the translation from the view's position
	to this pose's position is applied.
	\param view The view from which we should translate
	*/
	virtual void applyTransforms(ViewManager * view = NULL) = 0;

	/*---Interaction facilitation---*/
	virtual void NotifyPose(long changeMask);
	virtual void AccumulateNotifications();
	virtual void FlushNotifications();
    virtual void PurgeNotifications();
	virtual void ResumeSingularNotification(bool purge = false);
	/*---Bookkeeping---*/
	long getSerialNumber() const;
	PosableStateUser *getUser() {return user;}

/*---Generalized Interface---*/
	virtual PosableStateInfo getPoseInfo(long changeMask = ChangeAll);
	virtual void setPoseInfo(const PosableStateInfo &state, long changeMask = ChangeAll);
	virtual void setPoseInfoFromTo(long fromMask, long toMask = ChangePose);
	virtual void offsetPoseInfoBy(const PosableStateInfo &state, long changeMask = ChangeAll);
	virtual void unsetPoseInfo(long changeMask = ChangeAll);

	//Focus+context positioning information
	virtual void getFocusInfoUsing(PosableStateInfo &stateInfo);
	virtual void getContextInfoUsing(PosableStateInfo &stateInfo);

	//Continuous change indicators (high-res rendering hint)
	virtual void setContinuousChange(unsigned char changing);
	virtual unsigned char isInContinuousChange();

/*---End of Generalized Interface---*/

	/*---Static PosableStateInfo Operations---*/
	static void incrementPoseInfo(PosableStateInfo &accum, const PosableStateInfo &addend, long changeMask = ChangeAll);
	static void decrementPoseInfo(PosableStateInfo &accum, const PosableStateInfo &addend, long changeMask = ChangeAll);
	static void multiplyPoseInfo(PosableStateInfo &accum, float factor, long changeMask = ChangeAll);
	static void vectorToAnglePoseInfo(PosableStateInfo &result, const PosableStateInfo &vector);
        void setMinimumPixelsPerMeter(double m){minPixelsPerMeter = m;};
        double getMinimumPixelsPerMeter(){return minPixelsPerMeter;};

	/*---Other Operations---*/
        void approachToPosition(OrientationInfo &resultOrientation, double &resultDistance, gz4d::Point<double> const &vect);
        void approachFromPosition(OrientationInfo &resultOrientation, double &resultDistance, gz4d::Point<double> const &vect);

  protected:
	int continuouslyChanging;
	long accumulatedChange;
	GLboolean accumulatingChanges;
	PosableStateUser *user;
	long serialNumber;
	static const EventType *posableDeletedInputType;
	static int supressRecordingRequest;
	static GLboolean askingPointerForSet;

        double minPixelsPerMeter; ///< Allows an object to specify a minimum size to display, even if the object needs to scale up to do so.

	int stabilityThreshold;

	virtual PosableStateInfo *pointerForMask(long changeMask);
	virtual void replace(PosableStateBase *toBeReplaced);
};

extern DottedAddress *PosableNeighborhood;

#endif
