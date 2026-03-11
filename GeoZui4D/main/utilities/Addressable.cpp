// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// Addressable.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the base class for entities that are addressable in a dotted
 *	hierarchy notation (such as "Object.Group1.Nessie").  This class is
 *	responsible for the following.
 *		* Notify associated DottedAddress when this object is deleted
 *		  or changes address.
 *		* Accept notification of involuntary address change.
 *
 *	Created by mdp 3/26/2001
 */
/*---------------------------------------------------------------------------*/

#include <string.h>
#include "utilities/Addressable.h"
#include "utilities/DottedAddress.h"

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
/// Create an address by moving to an exsiting address
Addressable::Addressable(DottedAddress *startingAddress)
{
	address = NULL;
	moveTo(startingAddress);
}

/*---------------------------------------------------------------------------*/
/// Create an address from a string
Addressable::Addressable(std::string name)
{
	address = NULL;
	publishName(name);
}

/*---------------------------------------------------------------------------*/
/// destructor
Addressable::~Addressable()
{
	if (address)
		address->removeItem(this);
}

/*------------------------------Address Changes------------------------------*/
/*---------------------------------------------------------------------------*/
void Addressable::evictFrom(DottedAddress *oldAddress)
{
	if (address == oldAddress)
	{
		if (address == NULL)
			return;
		address = NULL;
		oldAddress->removeItem(this);
	}
}

/*---------------------------------------------------------------------------*/
void Addressable::moveTo(DottedAddress *newAddress)
{
	if (address == newAddress)
		return;

	if (address)
		address->removeItem(this);
	address = newAddress;
	if (address)
		address->assumeItem(this);
}

/*---------------------------------------------------------------------------*/
void Addressable::publishName(std::string name)
{
	if (name.empty())
		return;

	evictFrom(address);
	address = new DottedAddress(this, name);
}

/*----------------------------------Address----------------------------------*/
/*---------------------------------------------------------------------------*/
DottedAddress *Addressable::getAddress()
{
	return address;
}

void Addressable::getHtmlInfo(std::stringstream &html, short level)
{

	html << "<h" << level << ">Addressable:</h" << level << ">";
	if (address && !address->getAddress().empty())
		html << address->getAddress();
	else
		html << "Homeless...";
	html<< "<br>";
}
