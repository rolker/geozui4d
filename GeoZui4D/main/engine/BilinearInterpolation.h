// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// BilinearInterpolation.h
///////////////////////////////////////////////////////////////////////////////

#ifndef BILINEARINTERPOLATION_H
#define BILINEARINTERPOLATION_H

#include "engine/ConversionMethod.h"
/**
 *	This class implements bilinear interpolation for converting
 *	ConverstionGrids to GUTMs.
 *
 *  \author Matt Plumlee (mdp)
 *  \date 12/17/2001
 */
class BilinearInterpolation : public ConversionMethod
{
	public:
		BilinearInterpolation();
		virtual ~BilinearInterpolation();

		//Settings
		//virtual void addArgument(char *arg) {}; //expected to be handled by child classes w/out storing arg.

		//Conversion
		virtual int conversionMethod();
};

extern BilinearInterpolation *bilinearInterpolation;

#endif
