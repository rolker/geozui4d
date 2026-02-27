// Tell emacs this is -*- c++ -*-
/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// DottedAddress.h
/*---------------------------------------------------------------------------*/

/// \file
/// \brief DottedAddress class for addressable names

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class DottedAddress;

#ifndef DOTTED_ADDRESS_H
#define DOTTED_ADDRESS_H

#ifndef NULL
#define NULL 0
#endif

class Addressable;

/** 
 * \brief DottedAddress class.  A node in a Addressable tree.
 *
 *	This is the base class for addresses given in a dotted
 *	hierarchy notation (such as "Object.Group1.Nessie").  This class is
 *	responsible for the following.
 *	\li Assume a given name.
 *	\li Say who the parent and children are.
 *	\li Find a child based on a dotted hierarchy string.
 *	\li Return a dotted hierarchy string from which this object can be found.
 *
 *	\author Matt Plumlee (mdp)
 *  \note 3/26/2001
 */
/*---------------------------------------------------------------------------*/
class DottedAddress
{
public:
    /*---Constructors/Destructors---*/
    DottedAddress(Addressable *item, std::string newName = "");
    virtual ~DottedAddress();

    /*---Naming functions---*/
    void assumeName(std::string newName);
    /// return a string of the object name (or "" for none)
    std::string getName();
    /// return the path to this object as an address string
    std::string getAddress();
    /// return the object being stored
    Addressable *getItem();
    DottedAddress *find(std::string address, bool usePatterns = false, std::vector<DottedAddress *> *results = NULL);
    Addressable *findItem(std::string address, bool usePatterns = false);
    std::vector<Addressable *> *findItems(std::string address, bool usePatterns = false);
    int getNumChildren() {return numChildren;}
    void invalidateFullAddress();

    /*---Hierarchy functions---*/
    DottedAddress *getParent();
    DottedAddress *getChild(int index = 0);
    /// \brief Add a child node to this one.  
    /// If the child has no parent, then give it this one as parent.
    void addChild(DottedAddress *child);
    /// Remove the a child from the list of children.  Does NOT change the child's parent.
    void removeChild(DottedAddress *child);
    void reparent(DottedAddress *parent);

    /*---Management functions---*/
    void assumeItem(Addressable *item);
    /// FIX: Roland, what is the difference between asumeItem and substituteItem?
    void substituteItem(Addressable *item);
    /// Remove the Addressable from this node.  If the node has not more children, then delete the node
    void removeItem(Addressable *item);

    /// \brief Write out a GraphViz dot file
    /// This is just to help view what the doted namespace looks like.
    /// \return true on success
    bool writeGraphViz(std::string filename);
    /// Recursive decent printer for nodes.  Assumes wrapped by a graph foo {}
    void writeGraphViz(std::ofstream &out);
    /// Print a textual representation of children.
    void printTextGraph(const int level=0);  

private:
    DottedAddress *matchPattern(std::string pattern, std::string nextNames, std::vector<DottedAddress *> *results);

    std::string name;	///< The name of this node.
    std::string fullAddress;	///< Contains the dotted address string to this node.
    DottedAddress *parent;	///< The node up the tree.
    DottedAddress **children;	///< Array of all the children.  FIX: change this to a vector
    int numChildren, numChildSlots;	///< Array management variables.
    Addressable *addressedItem;	///< The data associated with the node
    bool inDestructor; ///< set to true in the destructor to ignore whining children
};

extern DottedAddress *rootAddress;

#endif
