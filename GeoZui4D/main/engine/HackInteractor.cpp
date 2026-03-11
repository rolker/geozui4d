// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// HackInteractor.h
/**
 * This class has been created as a stop-gap measure until the full scripting
 * interface is available for handling eventStreams and eventHandlers.
 *  *   Call Tcl scripts based on input events
 *
 * Created by mdp 3/11/2002
 */
///////////////////////////////////////////////////////////////////////////////

#include "MainWindow.h"
#include "engine/PosableStateWrapper.h"
#include "HackInteractor.h"
#include "engine/InputMediator.h"
#include "engine/InterpolationSupport.h"
#include "utilities/DottedAddress.h"
#include "engine/Object.h"
//#include "engine/Joystick.h"
#include "engine/PosableStateRelationships/ProximityHighlighter.h"
#include "objects/TextureScreen.h"

extern MainWindow *mainWindow;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** This function handles the stop-gap script calls.
 *
 */
int HackInteractor::HandleInput(unsigned int inputType, InputInfo *info)
{
    std::string name, otherName;
    char command[1024];
    char check[] = "if {[info procs userEvent] == \"userEvent\"}";

//  Joystick::JoystickInfoType * js;

    if (inputType == InterpolationSupport::interpolationBoundsEvent())
    {
        PosableStateWrapper *wrapper;
        wrapper = (PosableStateWrapper *) info->val.p;
        if (!wrapper || !wrapper->getAddress())
            return PASS_EVENT_THROUGH;
        name = wrapper->getAddress()->getAddress();
        sprintf(command, "%s {userEvent interpolationBounds %s}", check, name.c_str());
        mainWindow->Tcl_Eval(command);
        return SINK_EVENT;
    }
    if (inputType == Object::objectSelectedEvent())
    {
        Object *object;
        object = (Object *) info->val.p;
        if (!object || !object->getAddress())
            return PASS_EVENT_THROUGH;
        name = object->getAddress()->getAddress();
        sprintf(command, "%s {userEvent objectSelected %s %d}", check, name.c_str(), info->modifier.i);
        mainWindow->Tcl_Eval(command);
        return SINK_EVENT;
    }
    if (inputType == ProximityHighlighter::InProximityEvent() ||
        inputType == ProximityHighlighter::InCloseProximityEvent() ||
        inputType == ProximityHighlighter::OutOfProximityEvent() ||
        inputType == ProximityHighlighter::OutOfCloseProximityEvent())
    {
        PosableStateBase *p1, *p2;
        p1 = (PosableStateBase *)info->val.p;
        p2 = (PosableStateBase *)info->modifier.p;
        if (!p1 || !p2 || !p1->getAddress() || !p2->getAddress())
            return PASS_EVENT_THROUGH;
        name = p1->getAddress()->getAddress();
        otherName = p2->getAddress()->getAddress();
        if (inputType == ProximityHighlighter::InProximityEvent())
            sprintf(command, "%s {userEvent inProximity %s %s}", check, name.c_str(), otherName.c_str());
        else if (inputType == ProximityHighlighter::InCloseProximityEvent())
            sprintf(command, "%s {userEvent inCloseProximity %s %s}", check, name.c_str(), otherName.c_str());
        else if (inputType == ProximityHighlighter::OutOfProximityEvent())
            sprintf(command, "%s {userEvent outOfProximity %s %s}", check, name.c_str(), otherName.c_str());
        else if (inputType == ProximityHighlighter::OutOfCloseProximityEvent())
            sprintf(command, "%s {userEvent outOfCloseProximity %s %s}", check, name.c_str(), otherName.c_str());
        mainWindow->Tcl_Eval(command);
        return SINK_EVENT;
    }
    if (inputType == TextureScreen::textureScreenEvent())
    {
        Object *object;
        const char *eventType;
        object = (Object *) info->val.p;
        eventType = (const char *) info->modifier.p;
        if (!object || !object->getAddress() || !eventType)
            return PASS_EVENT_THROUGH;
        name = object->getAddress()->getAddress();
        sprintf(command, "%s {userEvent textureScreen %s %s}", check, name.c_str(), eventType);
        mainWindow->Tcl_Eval(command);
        return SINK_EVENT;
    }
/*
    if ((inputType & Joystick::JoystickAction))
    {
        int i;
        
        js = (Joystick::JoystickInfoType *)(info->val.p);


        cerr << "B";

        for(i=0; i<js->numButtons; i++)
        {
            cerr << " " << js->Buttons[i];
        }

        cerr << "\tH";

        for(i=0; i<js->numHats; i++)
        {
            cerr << " " << js->Hats[i];
        }

        cerr << "\tA";

        for(i=0; i<js->numAxes; i++)
        {
            cerr << " " << js->Axes[i];
        }

        cerr << "\n";
        return SINK_EVENT;
    }
*/
    return PASS_EVENT_THROUGH;
}

