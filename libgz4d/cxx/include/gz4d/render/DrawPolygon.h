// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_RENDER_DRAWPOLYGON_H
#define GZ4D_RENDER_DRAWPOLYGON_H

#include <gz4d/gz4d.h>
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/Color.h>
#include <gz4d/glo/Color.h>
#include <gz4d/render/DrawArrays.h>


namespace gz4d
{
    template <typename T> void DrawPolygon(Polygon<T> const &p)
    {
        glo::PushAttrib sattrib(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        glClearStencil(0x0);

        {
            glo::PushAttrib cattrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            glClear(GL_STENCIL_BUFFER_BIT);
            glColorMask(false,false,false,false);
            glDepthMask(false);
            glStencilFunc(GL_ALWAYS,1,1);
            glStencilOp(GL_INVERT,GL_INVERT,GL_INVERT);

            DrawArrays(p.getExteriorRing(),GL_TRIANGLE_FAN);
            for(std::vector<std::vector<gz4d::Point<double> > >::const_iterator ir = p.getInteriorRings().begin(); ir != p.getInteriorRings().end(); ++ir)
                DrawArrays(*ir,GL_TRIANGLE_FAN);
        }
        glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc (GL_NOTEQUAL, 0, 0x1);
        DrawArrays(p.getExteriorRing(),GL_TRIANGLE_FAN);
        for(std::vector<std::vector<gz4d::Point<double> > >::const_iterator ir = p.getInteriorRings().begin(); ir != p.getInteriorRings().end(); ++ir)
            DrawArrays(*ir,GL_TRIANGLE_FAN);
    }

    template <typename T> void DrawPolygonOutline(Polygon<T> const &p)
    {
        DrawArrays(p.getExteriorRing(),GL_LINE_STRIP);
        for(std::vector<std::vector<gz4d::Point<double> > >::const_iterator ir = p.getInteriorRings().begin(); ir != p.getInteriorRings().end(); ++ir)
            DrawArrays(*ir,GL_LINE_STRIP);
    }

    template <typename T> void DrawPolygonOutline(Polygon<T> const &p, Color const &exteriorColor, Color const &interiorColor)
    {
        glo::Color(exteriorColor);
        DrawArrays(p.getExteriorRing(),GL_LINE_STRIP);
        glo::Color(interiorColor);
        for(std::vector<std::vector<gz4d::Point<double> > >::const_iterator ir = p.getInteriorRings().begin(); ir != p.getInteriorRings().end(); ++ir)
            DrawArrays(*ir,GL_LINE_STRIP);
    }


    template <typename IT> void DrawPolygons(IT const &b, IT const &e)
    {
        for(IT i = b; i != e; ++i)
            DrawPolygon(*i);
    }

    template <typename CT> void DrawPolygons(CT const &pc)
    {
        DrawPolygons(pc.begin(),pc.end());
    }
}

#endif
