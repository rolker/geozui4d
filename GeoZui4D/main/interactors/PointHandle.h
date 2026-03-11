// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// PointHandle.h
///////////////////////////////////////////////////////////////////////////////

class PointHandle; //this line connects the above comments with this class.

#ifndef POINTHANDLE_H
#define POINTHANDLE_H

#include "engine/Interactor.h"
#include "interactors/Pickable.h"

class PosableStateBase;
class PosableStateWrapper;
class Object;
class InputTentacle;
class Window3D;

/**
 *	This is the interface for classes that implement various ways of
 *	interacting with point information, especially picking and modifying
 *	a point's location (but rotation, scale, others are possibilities as
 *	well).
 *	\par This class implements the following responsibilities:
 *	\li Default rendering for point selection
 *	\li	Attachment to point information through a PosableStateBase reference
 *		or a PosableStateWrapper.
 *
 *	\author Matt Plumlee mdp 
 *  \date 8/2/2002
 *  \image html pointHandle.jpg
 */
///////////////////////////////////////////////////////////////////////////////

class PointHandle : public Interactor, public Pickable
{
    public:
        PointHandle(int type = PICK_PRESS1, GLboolean constructWrapper = GL_TRUE);
        ~PointHandle();
    
        virtual void drawIn(Window3D *inWindow, gz4d::Point<double> const &pointPos, int selectLevel = 1, bool local = false, float size = 5.0f);
        virtual void drawSupportTools(Window3D *inWindow, gz4d::Point<double> const &pointPos);
        virtual void drawPickableTools(Window3D *inWindow, gz4d::Point<double> const &pointPos);
        virtual void toolAction(int pickType, InputInfo *inputInfo, GLuint *names, int hits);
        virtual void cancelAction();
        virtual void reset();
        void dragObject(Object *ob, Window3D *currentWindow);
        GLboolean usingWrapper(PosableStateWrapper *w) {return (wrapper == w);}
    
        InputTentacle *getKeyGrabber() {return keyGrabber;}
        PosableStateWrapper *getPointWrapper() {return wrapper;}
    
        virtual void publishName(std::string name);
    
        int HandleInput(unsigned int inputType, InputInfo *info);
        virtual void continueResponse();
    
        virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);
        virtual void renderPickableObject(Window3D *inWindow);
    
        void setDrawMode(int dMode) {drawMode = dMode;}
        void beginLayout(Object *obForLayout);
        void endLayout();
        
        void setTargetPoint(PosableStateBase * p){targetPoint = p;}
    
    protected:
        PosableStateBase *targetPoint;
        PosableStateWrapper *wrapper;
        GLboolean picking, dragging, layoutMode, internalPosable;
        gz4d::Point<double> lastSelectedPoint;
        bool lastSelectedPointValid;
        InputTentacle *keyGrabber;
        GLboolean cntrlPressed, shiftPressed;
        int keyPressed;
        Window3D *currentWin;
        int drawMode, drawModeDefault;
};

#endif
