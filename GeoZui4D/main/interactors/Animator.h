/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Animator.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class Animator;

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "engine/Interactor.h"
#include "engine/Persistence.h"
#include "SDV_types.h"
//#include <gz4d/util/Xml.h>
#include <gz4d/netcdf.h>

#include <boost/property_tree/ptree.hpp>

namespace bpt = boost::property_tree;


namespace gz4d
{
    class Xml;
};

class PosableStateBase;
/** 
 *	This is the interface for classes that animate one PosableStateBase to
 *	another, with some potential input from various input streams.
 *	Implementations of this interface are responsible for the following.
 *	\li Managing operand and resultant PosableStateBases
 *	\li Process input events
 *	\li Recognize a "cancel" event
 *
 *	\author Matt Plumlee (mdp)
 *  \date 1/17/2002
 */
class Animator: public Interactor, public Persistence
{
  public:
      Animator(PosableStateBase *resultant, std::string name,
			 int maxOperands = -1);
	Animator(PosableStateBase *resultant, int maxOperands = -1);
	void init(PosableStateBase *resultant, int maxOperands);
	virtual ~Animator();

	/*---Mutator facilitation---*/
	virtual void replaceResultant(PosableStateBase *newPose);
	virtual long addOperand(PosableStateBase *pose);
	virtual void removeOperand(PosableStateBase *pose = NULL);
	virtual void replaceOperand(PosableStateBase *oldPose, PosableStateBase *newPose);
	virtual void replaceOperand(PosableStateBase *newPose, int index = -1);
	virtual void setCancelable(GLboolean val = GL_TRUE) {cancelable = val;}
	void setChangeMask(long mask) {changeMask = mask;}
	virtual void activate(GLboolean val = GL_TRUE);

	/*---Accessor functions---*/
	virtual PosableStateBase *getResultant() {return resultantPose;}
	virtual PosableStateBase *getOprerand(int index = -1);
	long numOperands() {return nOperands;}

	/*---Interaction functions---*/
	virtual int HandleInput(unsigned int inputType, InputInfo *info);
	//virtual void RemovalNotify(InputMediator *fromList);
	virtual int animate() = 0;

	/*---Scripting funcitons---*/
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

	/*---File Related functions---*/
    virtual Animator * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    GLboolean setChangeMask(std::string effect);

	/*---Registration stuff---*/
	struct RegistrationEntry
	{
		const char *className;
		Animator *(*create)();
	};
	static RegistrationEntry **getRegistry() {return animatorRegistry;}
	static int getRegistrySize() {return numEntries;}
	static const RegistrationEntry *registerAnimator(
						const char *classname,
						Animator *(*instantiating_fn)());
	static RegistrationEntry *getRegistryEntry(const char *classname);
        static Animator *createFromClassname(std::string classname);

  protected:
	PosableStateBase *resultantPose, **operandPoses;
	long nOperands, nOperandsAllocated, maxOperandsAllowed;
	GLboolean cancelable, activated;
	long changeMask;

	virtual int IsCancelAction(int inputType, InputInfo *info);
	virtual const RegistrationEntry *getRegistrationEntry() = 0;

  private:
	static RegistrationEntry **animatorRegistry;
	static int numEntries, maxEntries;

};

extern DottedAddress *AnimatorNeighborhood;

/*---------------------------------------------------------------------------*/
/**	The DERIVE_ANIMATOR macro is intended to be used in the .h files for each
 *	Animator implementation.  The classname must match the class's name exactly.
 */
#define DERIVE_ANIMATOR(classname) \
static Animator *instantiate();\
virtual const RegistrationEntry *getRegistrationEntry();

/*---------------------------------------------------------------------------*/
/**	The REGISTER_ANIMATOR macro is intended to be used in the .cpp files for
 *	each Animator implementation.  The classname must match the class's name
 *	exactly.  Abstract classes should not use this macro.
 */
#define REGISTER_ANIMATOR(classname) \
Animator *classname::instantiate() \
{	return new classname; } \
\
const Animator::RegistrationEntry *classname##RegistrationEntry = Animator::registerAnimator(#classname, classname::instantiate); \
\
const Animator::RegistrationEntry *classname::getRegistrationEntry() \
{	return classname##RegistrationEntry; } \

/*---------------------------------------------------------------------------*/
/**	The LINK_ANIMATOR macro is reqiured in the main.cpp file for those classes
 *	that otherwise don't get linked in.
 */
#define LINK_ANIMATOR(classname) \
extern const Animator::RegistrationEntry *classname##RegistrationEntry;\
const Animator::RegistrationEntry *classname##Instantiator = classname##RegistrationEntry;

#endif
