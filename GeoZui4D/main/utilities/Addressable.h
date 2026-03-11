// Tell emacs -*- c++ -*-
// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/

/// \file
/// \brief Lightweight container for an addressable object: class Addressable

/*---------------------------------------------------------------------------*/
// Addressable.h
/*---------------------------------------------------------------------------*/

class Addressable;

#ifndef ADDRESSABLE_H
#define ADDRESSABLE_H

#ifndef NULL
#define NULL 0
#endif

#include<sstream>
#include "utilities/TellableInterface.h"


class DottedAddress;

/** 
 *  \brief Addressable class.  Uses DottedAddress
 *
 *	This is the base class for entities that are addressable in a
 *	dotted hierarchy notation (such as "Object.Group1.Nessie").
 *	Does not actually put the addressable object into the tree.
 *	That is your job.
 *
 *  \par This class is responsible for the following.
 *	\li Notify associated DottedAddress when this object is deleted
 *		  or changes address.
 *	\li Accept notification of involuntary address change.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 3/26/2001
 *  \image html addressHierarchy.jpg
 */
/*---------------------------------------------------------------------------*/
class Addressable : public TellableInterface
{
	public:
		/*---Constructors/Destructors---*/
		Addressable(DottedAddress *startingAddress = NULL);
                Addressable(std::string name);
		virtual ~Addressable();

		/// \name Address changes
		/// @{
		/// If given the correct address, nuke the DottedAddress.	
		/// Sets the address to be 0.
		void evictFrom(DottedAddress *oldAddress);
		///
		///
		///
		void moveTo(DottedAddress *newAddress);
		/// Change the name of the addressable to a new name
		/// @param name new name to go by
                virtual void publishName(std::string name);
		/// @}

		/// \name Address
		/// @{
		/// Get the DOTtedAddress object that contains the actual address
		/// \return pointer to a DottenAddress or 0 if none
		DottedAddress *getAddress();
		/// @}
		
                virtual void getHtmlInfo(std::stringstream &html, short level);
	private:
		DottedAddress *address; ///< Address in dotted notation.
};

#endif
