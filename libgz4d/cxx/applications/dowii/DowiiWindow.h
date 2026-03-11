// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef DOWIIWINDOW_H
#define DOWIIWINDOW_H

#include <gz4d/GlutWindow.h>
#include <boost/filesystem.hpp>
#include <gz4d/Screen.h>
#include <gz4d/View.h>
#include <gz4d/Transform.h>
#include <gz4d/X3D.h>

#include "Dowii.h"

class DowiiWindow: public gz4d::GlutWindow
{
    public:
        DowiiWindow(std::string t,boost::filesystem::path fn, gz4d::Dowii *d);

        void OnIdle();
        void OnDisplay();
        void OnReshape(int w, int h);
        void OnMouse(int button, int state, int x, int y);
        void OnMotion(int x, int y);

    protected:
        gz4d::Screen screen;
        gz4d::View view;
        gz4d::Transform transform;
        gz4d::Dowii *dowii;

    private:
        gz4d::X3D x3d;
};

#endif
