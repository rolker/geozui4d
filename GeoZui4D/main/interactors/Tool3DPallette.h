// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// Tool3DPallette.h
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL3DPALLETTE_H
#define TOOL3DPALLETTE_H

#include "engine/Interactor.h"

class Tool3D;
class InputMediator;
class ToglWindow;
/** 
 * This class handles selection of Tool3D's.
 *
 * \author Matt Plumlee (mdp)
 * \date 3/3/2000
 */
class Tool3DPallette : public Interactor
{   public:
		Tool3DPallette(InputMediator *targetOfTool);
		virtual int RegisterWithTcl(ToglWindow *throughWin);
		~Tool3DPallette();

		int HandleInput(unsigned int inputType, InputInfo *info);
		int addTool(Tool3D *);
		void enable(int flag=0) {enabled = flag;}

	protected:
		Tool3D **hotKeyArray;
		InputMediator *toolTarget;
		ToglWindow *windowForRegistering;
		int enabled;

	private:
};

extern Tool3DPallette globalToolbox;

#endif

