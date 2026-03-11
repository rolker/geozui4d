// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// RubberbandInteractor.h

///////////////////////////////////////////////////////////////////////////////

#ifndef RUBBERBANDINTERACTOR_H
#define RUBBERBANDINTERACTOR_H

#include "engine/Interactor.h"
#include "objects/RubberBandObject.h"

class Gutm;
/** 
 * Interactor for handling input for rubberbands (for GUTMs only at this point).
 *
 *
 * \author Matt Plumlee (mdp)
 * \date 10/18/2004
 */
class RubberbandInteractor : public Interactor
{   public:
        RubberbandInteractor();
        int HandleInput(unsigned int inputType, InputInfo *info);
        virtual void RemovalNotify(InputMediator *fromList);

        void attachToGutm(Gutm *g);

        RubberBandObject rubberband; //for more flexibility, we might consider making this a pointer later on

    protected:
        Gutm *targetGutm;

    private:
};

extern RubberbandInteractor *gutmRubberband;
#endif

