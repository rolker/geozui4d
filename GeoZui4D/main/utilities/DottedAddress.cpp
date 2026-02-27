/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/

/// \file
/// \brief Implentation of DottedAddress class
///
/// \bug convert the child list to a vector or some other STL list

/*---------------------------------------------------------------------------*/
// DottedAddress.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the base class for addresses given in a dotted
 *	hierarchy notation (such as "Object.Group1.Nessie").  This class is
 *	responsible for the following.
 *		* Assume a given name.
 *		* Say who the parent and children are.
 *		* Find a child based on a dotted hierarchy string.
 *		* Return a dotted hierarchy string from which this object can be found.
 *
 *	Created by mdp 3/26/2001
 */
/*---------------------------------------------------------------------------*/

// C stdlib includes
#include <cstring>
#include <cstdio>
#include <cassert>

// C++ stdlib includes
#include <string>
#include <vector>

#include "SDV_types.h"
#include "utilities/DottedAddress.h"
#include "utilities/Addressable.h"
#include "engine/PosableStateBase.h"
#include "engine/PosableStateUser.h"

#define CHILD_SLOT_QUANTUM 5

Addressable rootAddressAnchor;
DottedAddress *rootAddress = new DottedAddress(&rootAddressAnchor, "");

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
DottedAddress::DottedAddress(Addressable *item, std::string newName) : inDestructor(false)
{
	addressedItem = NULL;
	name = newName; // Leave empty on NULL name
	parent = NULL;
	children = NULL;
	numChildren = 0;
	numChildSlots = 0;
	fullAddress = "";

	assumeItem(item);
}

/*---------------------------------------------------------------------------*/
DottedAddress::~DottedAddress()
{
	assert(false==inDestructor);
	inDestructor=true; // Do not let children mess with us when they are deleted.

	if (addressedItem)
		addressedItem->evictFrom(this);
		addressedItem=0;  // FIX: this is just for safety

	if (parent)
		parent->removeChild(this);

	if (children) {
		for (int i = 0; i < numChildren; i++) {
	    		delete children[i];
		    	children[i]=0;  // Just for safety
		}
	}

	if (children) 
	  delete[] children;
}

/*-----------------------------Naming Functions------------------------------*/
/*---------------------------------------------------------------------------*/
void DottedAddress::assumeName(std::string newName)
{
	assert(!newName.empty());
	name = newName;
}

/*---------------------------------------------------------------------------*/
std::string DottedAddress::getName()
{
	return name;
}

/*---------------------------------------------------------------------------*/
std::string DottedAddress::getAddress()
{
	if (name.empty())
		return name;

	// get the parent's full address
	std::string parentAddress;
	if (parent)
		parentAddress = parent->getAddress();
	else
		return name;

	// assume fullAddress is right if we have it.
	if (!fullAddress.empty())
		return fullAddress;

	// create a new address from the parent's
	fullAddress = parentAddress + "." + name;

	return fullAddress;
}

/*---------------------------------------------------------------------------*/
Addressable *DottedAddress::getItem()
{
	return addressedItem;
}

/*---------------------------------------------------------------------------*/
DottedAddress *DottedAddress::matchPattern(std::string pattern, std::string nextNames, std::vector<DottedAddress *> *results)
{
    std::vector<std::string> types;
    std::vector<std::string> matchParts;
    int numParts = 0, numTypes = 0, j;
    bool rightType, drillDown, startsWithWildcard = false, endsWithWildcard = false;
    std::string::size_type split, currentIndex;
    std::string typeName;
    std::string childName, modifiedNextNames;
    DottedAddress *matchingChild = NULL;
    PosableStateBase *posable = NULL;
    PosableStateUser *user = NULL;
    Addressable *thing;

    drillDown = (nextNames.size() > 0);
    //std::cerr << "looking for " << pattern.c_str() << " / " << nextNames.c_str() << " in " << getAddress().c_str() << std::endl;
    if (drillDown && pattern.size() == 0 && nextNames[0] == '.') //"..." case
    {
        modifiedNextNames = nextNames.substr(1);
        while (modifiedNextNames[0] == '.') //and get past the dots in nextNames.
            modifiedNextNames = modifiedNextNames.substr(1);
        //we will be returning either a match or NULL...
        //First, see if literals match at the first level of children under me.
            //do {
                matchingChild = find(modifiedNextNames, true, results);
            //} while (results && matchingChild); //exit out when either is NULL
            if (results && results->size() > 0)
                matchingChild = NULL;
        //nope! Try passing the pattern to the next level down.
        for (int i = 0; i < numChildren && matchingChild == NULL; i++)
        {
            //(If there's no children, the recursion will stop because this loop won't be entered)
            matchingChild = children[i]->matchPattern(pattern, nextNames, results); //pass the ... on!
            if (results)
                matchingChild = NULL; //don't stop looking if we're accumulating results!
        }
        if (results && results->size() > 0)
            matchingChild = (*results)[0];
        return matchingChild;
    }

    //Other special characters
    currentIndex = 0;
    while (pattern.size() > 0)
    {
        if (pattern[0] == '{' || pattern[0] == '|')
        {
            split = pattern.find('|');
            if (split == std::string::npos)
                split = pattern.find('}');
            if (split != std::string::npos)
            {
                //std::cerr << "Type: " << pattern.substr(1, split - 1).c_str() << std::endl;
                types.push_back(pattern.substr(1, split - 1));
                pattern = pattern.substr(split + 1);
            }
            else
            {
                types.push_back(pattern.substr(1));
                pattern = "";
            }
        }
        else
        {
            split = pattern.find('*');
            if (split == std::string::npos)
            {
                //std::cerr << "Part: " << pattern.c_str() << std::endl;
                matchParts.push_back(pattern);
                pattern = "";
            }
            else
            {
                if (currentIndex == split)
                {
                    //either two stars together (ignore) or a star at the front
                    if (currentIndex == 0)
                        startsWithWildcard = true;
                }
                else
                {
                    //std::cerr << "Part: " << pattern.substr(0, split).c_str() << std::endl;
                    matchParts.push_back(pattern.substr(0, split));
                }
                pattern = pattern.substr(split + 1);
                if (pattern.size() == 0)
                    endsWithWildcard = true;
            }
        }
    }

    //OK, we've parsed out the parts, now lets try to match it!
    numParts = matchParts.size();
    numTypes = types.size();
    for (int i = 0; i < numChildren && matchingChild == NULL; i++)
    {
        if (numTypes)
        {
            rightType = false;
            thing = children[i]->getItem();
            DYNAMIC_CAST(posable, PosableStateBase *, thing);
            if (posable)
                user = posable->getUser();
            if (user)
            {
                typeName.erase(); //There's a bug on the VC++ side that doesn't assign strings right sometimes.
                GET_CLASS_NAME(user, typeName);
                //std::cerr << &(typeid(*user).name()[6]) << std::endl;
            }
            else
            {
                GET_CLASS_NAME(thing, typeName);
            }
            for (int j = 0; j < numTypes; j++)
            {
                if (types[j] == typeName)
                {
                    rightType = true;
                    if (numParts <= 0)
                        endsWithWildcard = true;
                    break;
                }
            }
            if (!rightType)
                continue;
        }
        ///Ok, so now we try to match matchParts successively in the child's name...
        childName = children[i]->getName();
        currentIndex = 0;
        for (j = 0; j < numParts; j++)
        {
            currentIndex = childName.find(matchParts[j], currentIndex);
            if (currentIndex == std::string::npos)
                break;
            if (j == 0 && currentIndex != 0 && !startsWithWildcard)
            {
                currentIndex = std::string::npos;
                break; //oops... no good!
            }
            currentIndex += matchParts[j].size(); //skip over the matched part
        }
        if (currentIndex != std::string::npos &&
            (endsWithWildcard || currentIndex == childName.size())) //we have a type-match and a string match!
        {
            if (!drillDown)
            {
                matchingChild = children[i];
                if (results)
                    results->push_back(matchingChild);
            }
            else
            {
                matchingChild = children[i]->find(nextNames, true, results);
            }
            if (results)
                matchingChild = NULL;
        }
    }

    if (results && results->size() > 0)
        matchingChild = (*results)[0];

    return matchingChild; // should be NULL only if we didn't find it.
}

/*---------------------------------------------------------------------------*/
DottedAddress *DottedAddress::find(std::string address, bool usePatterns, std::vector<DottedAddress *> *results)
{
    DottedAddress *retval = NULL;

	assert(!address.empty());

    //see if there's a pattern indicator
    if (address[0] == '#')
    {
        usePatterns = true;
		address = address.substr(1);
    }

	//match root
	if (address[0] == '.' && name.empty())
		address = address.substr(1);

	//first, split the address into the current part and the remainder.
	std::string::size_type split = address.find('.');
	
	std::string currentName, nextNames;
	bool drillDown = false;

	if(split != std::string::npos)
	{
		currentName = address.substr(0,split);
		nextNames = address.substr(split+1);
		drillDown = true;
	}
	else
		currentName = address;

    if (usePatterns)
    {
//        do {
            retval = matchPattern(currentName, nextNames, results);
//        } while (results && retval);
    }
    else //Now go through our children to see if we can find the current name.
    {
	    for (int i = 0; i < numChildren; i++)
	    {
		    if (children[i]->getName() == currentName)
		    {	//found it!
			    if (!drillDown)
                {
                    retval = children[i];
                    if (results) //it's possible to have multiple addresses with the same name, so this would be the only way to access them individually
                        results->push_back(retval);
                }
			    else
				    retval = children[i]->find(nextNames, usePatterns, results);
                if (!results)
                    break; //break out so we can report what we found
		    }
	    }
    }

    if (results && results->size() > 0)
        retval = (*results)[0];

    return retval; //should be NULL only if we didn't find it.
}

/*---------------------------------------------------------------------------*/
Addressable *DottedAddress::findItem(std::string address, bool usePatterns)
{
	DottedAddress *foundAddress = find(address, usePatterns);

	if (foundAddress)
		return foundAddress->getItem();
	else
		return NULL;
}

/*---------------------------------------------------------------------------*/
std::vector<Addressable *> *DottedAddress::findItems(std::string address, bool usePatterns)
{
    std::vector<Addressable *> *retval = NULL;
    std::vector<DottedAddress *> results;

    if (find(address, usePatterns, &results))
    {
        retval = new std::vector<Addressable *>;
        for (std::vector<DottedAddress *>::iterator i = results.begin(); i != results.end(); ++i)
            retval->push_back((*i)->getItem());
    }
    return retval;
}

/*----------------------------Hierarchy Functions----------------------------*/
/*---------------------------------------------------------------------------*/
DottedAddress *DottedAddress::getParent()
{
	return parent;
}

/*---------------------------------------------------------------------------*/
DottedAddress *DottedAddress::getChild(int index)
{
	assert(index>=0);
  	// Is it okay to ask for a child that is out of range??
	// Yes.  This is one way of iterating through the list,
	// and ending when you get a NULL result. (mdp)
	if (index < 0 || index >= numChildren) {
		//cerr << "WARNING: asked for an out of range child in getChild.\n";
		return NULL;
	}
	assert (children[index]); // We don't keep dead children around
	return children[index];
}

/*---------------------------------------------------------------------------*/
void DottedAddress::addChild(DottedAddress *child)
{
	assert(child!=this);  // Try to prevent looping on self
	assert(child); // FIX: should this really return or is in an error?
	if (!child)
		return;

	DottedAddress **newChildren;

	//Do we need to allocate more slots?
	int newNumChildren = numChildren + 1;
	if (newNumChildren > numChildSlots)
	{
		numChildSlots += CHILD_SLOT_QUANTUM;
		newChildren = new DottedAddress *[numChildSlots];
		for (int i = 0; i < numChildren; i++) {
			newChildren[i] = children[i];
		}
		delete []children;
		children = newChildren;
	}

	// Now we add the child and set its parent to be this.
	children[numChildren] = child;
	if (!child->parent) //allow for secondary names by not assigning if exists.
		child->parent = this;
	numChildren = newNumChildren;
}

/*---------------------------------------------------------------------------*/
// FIX: should this remove the parent from the child if it is this node?!?!
void DottedAddress::removeChild(DottedAddress *child)
{
	assert(child);  // FIX: is this an ok assertion?
	if (inDestructor) {
		return;  // we are in the process of nuking children, so ignore their requests to divorce us
	}

	//find child and replace it with the last child.
	for (int i = 0; i < numChildren; i++)
	{
		if (children[i] == child)
		{
			numChildren--;
			assert(numChildren>=0);
			// copy in last child
			if (numChildren>0)
			  children[i] = children[numChildren];
			children[numChildren] = NULL;
			if (!addressedItem && numChildren <= 0) {
                std::cout << "FIX: deleting this!!!! SCARY\n";
			  delete this; // FIX: IS THIS OK?
			}
			return;
		}
	}
}

/*---------------------------------------------------------------------------*/
void DottedAddress::reparent(DottedAddress *newParent)
{
	assert(newParent);
	if (parent)
	{
		parent->removeChild(this);
		parent = NULL;
	}
	newParent->addChild(this);
	invalidateFullAddress();
	fullAddress = "";
}

/*---------------------------------------------------------------------------*/
void DottedAddress::invalidateFullAddress()
{
	fullAddress.erase();

	for (int i = 0; i < numChildren; i++)
	{
		if (children[i])
			children[i]->invalidateFullAddress();
	}
}

/*--------------------------------Management functions-----------------------*/
/*---------------------------------------------------------------------------*/
void DottedAddress::assumeItem(Addressable *item)
{
  // assert(item); This assert causes a unittest to fail.
	if (addressedItem == item)
		return;

	if (addressedItem)
		addressedItem->evictFrom(this);
	addressedItem = item;
	item->moveTo(this);
}

/*---------------------------------------------------------------------------*/
void DottedAddress::substituteItem(Addressable *item)
{
	assert(item);
	numChildren++;
	assumeItem(item);
	numChildren--;
}

/*---------------------------------------------------------------------------*/
void DottedAddress::removeItem(Addressable *item)
{
	assert(item);

	if (inDestructor) {
		return;
	}

	if (addressedItem == item)
	{
		addressedItem = NULL;
		item->evictFrom(this);
		if (!numChildren)
			delete this;
	}
}

//////////////////////////////////////////////////////////////////////
// Write out a graph viz dot file of the children.
// FIX: might be good to have a stream version so we can crank these
// graphs into the web interface.
//
// These are really for debugging and understanding system operation.  -kurt

bool DottedAddress::writeGraphViz(std::string filename) {
	assert (!filename.empty());
	std::string s(filename);
	s+=std::string(".dot");
    std::cout << "Opened " << s << std::endl;
    std::ofstream out(s.c_str(),std::ios::out);
	if (!out.is_open()) {
		std::cerr << "ERROR: writeGraphViz unable to open file ("<<filename<<")\n";
		return false;
	}
	out << "digraph " << filename << " {" << "\n";
	writeGraphViz(out);
	out << "} // digraph " << filename << "\n";
	return true;
}

void DottedAddress::writeGraphViz(std::ofstream &out) {
	for (int childNum=0;childNum<getNumChildren();childNum++) {
		out << "\t" << getName() << " -> " << getChild(childNum)->getName() << "; // "<< childNum << "\n";
		getChild(childNum)->writeGraphViz(out);
	}
}

void DottedAddress::printTextGraph(const int level) {
	for (int childNum=0;childNum<getNumChildren();childNum++) {
		for (int i=0;i<level;i++) std::cout << "\t";
		std::cout << getName() << " -> " << getChild(childNum)->getName() << "; // "<< childNum << "\n";
		getChild(childNum)->printTextGraph(level+1);
	}
}
