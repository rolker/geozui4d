// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef X3D_H
#define X3D_H

#include "engine/Object.h"
#include <gz4d/X3D.h>

class X3D: public Object
{
public:
    X3D();
    ~X3D();

    DERIVE_OBJECT(X3D);
    void DrawLowRes(float degradation, Window3D *win);
    int Load();
    int perform(int argc, const char **argv, Tcl_Interp *interp);
    void getHtmlInfo(std::stringstream &html, short level);
private:
    gz4d::X3D x3d;
    float pointSize;
};

#endif
