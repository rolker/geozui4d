/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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

