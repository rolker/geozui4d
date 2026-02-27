/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ZoomInteractor.h
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

class ZoomInteractor; //this line connects the above comments with this class.

#ifndef ZOOMINTERACTOR_H
#define ZOOMINTERACTOR_H

#include "engine/ViewManager.h"
#include "engine/Interactor.h"

class RelationshipQueryResult;
class ToglWindow;

/**
 *  This class implements a zooming interactor that zooms in and out at a rate
 *  proportional to the distance to the selected object.  This class operates
 *  on ViewManagers to effect the viewpoint changes.
 *  This class implements the following responsibilities:
 *  \li Provide mechanisms for customizing the speed of the zooms.
 *  \note Separated out from Sv_Widgets
 *  \author Matt Plumlee (mdp)
 *  \date 7/24/2000 
 */
class ZoomInteractor : public Interactor
{
   public:
        //Initialization
        enum InteractionType
        {
            DontChangeInteractionType      = 0x00,
            SwitchDirectionOnShift         = 0x01,
            SwitchDirectionOnThreshhold    = 0x02,
            SwitchDirectionOnShiftOrThresh = 0x03,
            SwitchDirectionOnReverseThresh = 0x04,
            SwitchDirectionOnRevShiftThresh= 0x05,
            SwitchDirectionOnSomeThreshhold= 0x06,
        };

        ZoomInteractor(ViewManager *targetView, InteractionType it = DontChangeInteractionType);
        int RegisterWithTcl(ToglWindow *throughWin);
        ~ZoomInteractor(void);

        //Interaction functions
        enum ZoomDirection {NotZooming = -2, ZoomingIn = -1, TranslatingOnly, ZoomingOut = 1};

        int HandleInput(unsigned int inputType, InputInfo *info);

        GLboolean startZoom(InputInfo *info);
        void continueZoom(void);
        void uponThreshhold(int y);
        int uponShift(int inputType);

        void SetScale(GLfloat sval);

    void Zoom(double amount);
        
        static double zoomInRate, zoomOutRate, translateRate;
        
   protected:
        GLdouble        startingPos[3];        // Where did the mouse start?
        gz4d::Point<double>  desiredTranslation; // The amount we are translating for the full interaction.
        GLboolean       shiftPressed;          // Is the Shift button currently down?
        GLboolean       usingWheel;            // Are we using the wheel rather than mouseMotion?

        ZoomDirection   zoomDirection;
        static InteractionType interactionType;

        ViewManager *currentView;
        RelationshipQueryResult *locksOnWindow; //who do we need to control when changing scale/position?
        long changeMask;                //what is the current operation in locking terms?

};
#endif
