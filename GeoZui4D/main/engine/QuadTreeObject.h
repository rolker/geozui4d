// Tell emacs -*- c++ -*-
// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/

/// \file
/// \brief Interface for Objects that can be the visibleObjects of a QuadTreeTile

/*---------------------------------------------------------------------------*/
// QuadTreeObject.h
/*---------------------------------------------------------------------------*/

class QuadTreeObject;

#ifndef QUADTREETARGET_H
#define QUADTREETARGET_H

#include "engine/Object.h"
#include <memory>

class QuadTreeTile;

/** 
 *  \brief QuadTreeObject class.  Uses QuadTreeTile
 *
 *	This is the base class for Objects that can be
 *  used in an Object QuadTree as the target of a QuadTreeTile.
 *
 *  \par This class is responsible for the following.
 *	\li Keep track of which QuadTreeTile it belongs to
 *	\li Be able to make a "shallow" copy of itself offset by
 *      specified x- and y-offsets
 *  \li Be able to read new "deep" contents from a given file
 *      without obtaining new properties from the file (for
 *      example, skipping over the header in a GUTM to get
 *      to the depth values only).
 *  \li Be able to generate a half-resolution copy of oneself
 *      from the children of the associated QuadTreeTile.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 9/19/2008
 */
/*---------------------------------------------------------------------------*/
class QuadTreeObject : public Object
{
    public:
        /*---Constructors/Destructors---*/
        QuadTreeObject(GLboolean allocatePosable = GL_TRUE, int pickingTypes = PICK_CLICK1 | PICK_PRESS3);
        virtual ~QuadTreeObject() {};

        /// \name Tile access
        /// @{
        /// Change the tile we're associated with
        /// @param tile new tile to be associated with
        virtual void setTile(std::weak_ptr<QuadTreeTile> tile) {myTile = tile;}

        /// Get the tile we're associated with
        /// \return weak_ptr to the associated tile
        virtual std::weak_ptr<QuadTreeTile> getTile() {return myTile;}
		/// @}

		/// \name Copy/Prototype facility
		/// @{
		/// Create a "shallow" copy of ourselves, offset by
        /// the xOffset and yOffset parameters
		/// \return pointer to new version of the child object
        /// without "deep" contents like an image or depth field.
		virtual QuadTreeObject *offsetCopy(float xOffset, float yOffset, float scaleFactor = 1.0) = 0;

        /// Set an XML attribute (useful for changing individual
        /// items for each pattern copy of a prototype)
        /// The path argument is there primarily for the "filename"
        /// xml attribute to know what it is relative to.
        /// \return true if attribute name and value are valid
        /// and false otherwise.
        virtual bool setXMLattribute(std::string const &name, std::string const &value,
                                     std::string const &path);

        /// Read new "deep" contents from any files specified
        /// in prior calls to setXMLattribute, skipping over any
        /// information not specifically intended for this
        /// kind of read.
        /// \return true if succeeded, false if failed.
        virtual bool readContents() = 0;
		/// @}

        /// Create a half-resolution (mip-map) copy of the
        /// QuadTreeTargets associated with the children of
        //  the QuadTreeTile this object is associated with.
        virtual bool combineChildrenOfTile() = 0;

        /// Set up to draw only portions of self the next time
        /// we draw.  This is done by indicating which children
        /// have already drawn (we draw only where false).
        virtual void resetChildrenDrawn(bool *cd = NULL) {childrenDrawn = cd;}
		
        /// Set a flag that says this is a template object
        /// (May be important to some objects)
        virtual void actAsTemplate();

        /// Set a flag that says this is a patterned object
        /// (May be important to some objects)
        virtual void actAsPattern() {quadtreeStatus |= Quadtree_Pattern;}

        /// Set a flag that says this is the root object
        /// (May be important to some objects)
        virtual void actAsRoot() {quadtreeStatus |= Quadtree_Root;}

        /// Set a flag that says this object may be on an edge
        /// (May be important to some objects)
        virtual void actAsPotentialEdge() {quadtreeStatus |= Quadtree_Potential_Edge;}

        /// Return how wide or tall the object is in terms of
        /// cells (as determined by child class)
        virtual double getCellsWide() = 0;
        virtual double getCellsTall() = 0;

        /// Return an estimate of the size of the smallest
        /// resolvable feature in the object.
        virtual double getPixelSize() = 0;

        /// Return whether or not the object was able to fully
        /// render on the last go-around
        virtual bool renderedOnMostRecentTry() {return drewLastTime;}

	protected:
        /// Method to help child classes copy common Object parts.
        /// @param other New QuadTreeTarget we're copying into
        /// @param xOffset Amount to add to x-position for new QuadTreeObject
        void copyObjectComponents(QuadTreeObject *other, float xOffset, float yOffset);

        std::weak_ptr<QuadTreeTile> myTile;
        enum {Quadtree_Not,
              Quadtree_Template = 0x1,
              Quadtree_Pattern = 0x2,
              Quadtree_Root = 0x4,
              Quadtree_Potential_Edge = 0x8};
        char quadtreeStatus;
        bool *childrenDrawn, drewLastTime;
};

#endif
