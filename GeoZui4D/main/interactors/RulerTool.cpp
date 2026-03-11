// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// RulerTool.cpp
// 
// Interactor for making measurements and placing depth markers.
//
//
// Created by mdp 3/3/2000
// 
///////////////////////////////////////////////////////////////////////////////

#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "objects/RulerPost.h"
#include "interactors/RulerTool.h"
#include "widgets/Sv_Standards.h"
#include "engine/ViewManager.h"

/*---------------------------------------------------------------------------*/
RulerTool::RulerTool()
:Tool3D("ruler")
{   hotKey = 'r';
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int RulerTool::HandleInput(unsigned int inputType, InputInfo *info)
{
    gz4d::Point<double> worldPos;
    Window3D *window;
    Scene3D *scene;

    preprocessInput(inputType, info);

    if (inputType & EnumeratedAction)
        return PASS_EVENT_THROUGH;

    if (inputType & MouseButton1 && inputType & MouseButtonDown) //changed to the first mouse button my mdp 7/25/2000
    {   
        window = Window3D::cast(info->window);
        if (window && window->getSelectedLocation(info, worldPos))
        {
            scene = window->getScene3D();
            window->addObject(new RulerPost((ObjectCollection *)scene->GetObjectPtr(0), worldPos[X], worldPos[Y], worldPos[Z]));
        }
        return REMOVE_FROM_STREAM;
    }
    return PASS_EVENT_THROUGH;
}

