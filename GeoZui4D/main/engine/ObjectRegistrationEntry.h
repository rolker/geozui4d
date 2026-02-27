/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// ObjectRegistrationEntry.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class Object;

#ifndef OBJECTREGISTRATIONENTRY_H
#define OBJECTREGISTRATIONENTRY_H
/** 
 *	The ObjectRegistryEntry structure supports the registration of objects at
 *	startup time.  This file also contains the definitions for the
 *	REGISTER_OBJECT and DERIVE_OBJECT macros.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 1/9/2001
 */
struct ObjectRegistrationEntry
{
	std::string className;
	std::string extension;
	Object *(*create)();
	std::string contextMenu;
	std::string aliasFor; //added by mdp 2/11/2004 to allow for alias names
};

#define DEFAULT_OBJECT_MENU_ITEMS "{Attributes} {attribute panel show} {} {} {Hide} hide {show} show {Wireframe} {wireframe 1} {Solid} {wireframe 0} {Allow Edit} {edit 1} {DisallowEdit} {edit 0} {Goto} goto {hud on} {hud 1} {hud off} {hud 0} "
//#define DEFAULT_OBJECT_MENU_ITEMS "{Hide} hide"

/*---------------------------------------------------------------------------*/
/**	The REGISTER_OBJECT macro is intended to be used in the .cpp files for
 *	each object implementation.  
 *  \note The classname must match the class's name
 *	exactly, and the extension is the expected extension of any file storing
 *	this type of object.  Abstract classes should not use this macro.
 *  \author Matt Plumlee (mdp)
 *  \date 1/9/2001
 */
#define REGISTER_OBJECT(classname, extension) \
Object *classname::instantiate() \
{	return new classname(); } \
\
const ObjectRegistrationEntry *classname##RegistrationEntry = Object::registerObject(#classname, extension, classname::instantiate, classname::contextMenu); \
\
const ObjectRegistrationEntry *classname::getRegistrationEntry() \
{	return classname##RegistrationEntry; }\
\
const char *classname::contextMenu = "{" DEFAULT_OBJECT_MENU_ITEMS "}";

/*---------------------------------------------------------------------------*/
/**	The REGISTER_OBJECT_WITH_MENU macro is intended to be used in the .cpp
 *	files for each object implementation.  
 *  \note The classname must match the class's
 *	name exactly, and the extension is the expected extension of any file
 *	storing this type of object.  This macro expects the user to assign a
 *	menu string to it.  This string is later used to create the context
 *	sensitive menu.  Abstract classes should not use this macro.
 *  \author Matt Plumlee (mdp)
 *  \date 1/9/2001
 */
#define REGISTER_OBJECT_WITH_MENU(classname, extension) \
Object *classname::instantiate() \
{	return new classname; } \
\
const ObjectRegistrationEntry *classname##RegistrationEntry = Object::registerObject(#classname, extension, classname::instantiate, classname::contextMenu); \
\
const ObjectRegistrationEntry *classname::getRegistrationEntry() \
{	return classname##RegistrationEntry; } \
\
const char *classname::contextMenu /*user string here*/

/*---------------------------------------------------------------------------*/
/**	The LINK_OBJECT macro is reqiured in the main.cpp file for those classes
 *	that otherwise don't get linked in.
 */
#define LINK_OBJECT(classname) \
extern const ObjectRegistrationEntry *classname##RegistrationEntry;\
const ObjectRegistrationEntry *classname##Instantiator = classname##RegistrationEntry;

/*---------------------------------------------------------------------------*/
/**	The KNOW_OBJECT macro is used to allow reference to
 *	ObjectRegistrationEntries from other files.
 */
#define KNOW_OBJECT(classname) \
extern const ObjectRegistrationEntry *classname##RegistrationEntry;

/*---------------------------------------------------------------------------*/
/**	The DERIVE_OBJECT macro is intended to be used in the .h files for each
 *	object implementation.  The classname must match the class's name exactly.
 */
#define DERIVE_OBJECT(classname) \
static Object *instantiate();\
static const char *contextMenu;\
virtual const ObjectRegistrationEntry *getRegistrationEntry();

/*---------------------------------------------------------------------------*/
/**	The REGISTER_ALIAS macro is intended to be used in the .cpp files for
 *	creating XML aliases--virtual classes that are so similar codewise that
 *	there's no good reason to create C++ classes to differentiate them.
 */
#define REGISTER_ALIAS(alias, classname, extension) \
	const ObjectRegistrationEntry *alias##RegistrationEntry = Object::registerAlias(#alias, #classname, extension);

#endif
