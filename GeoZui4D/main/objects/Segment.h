// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef SEGMENT_H
#define SEGMENT_H

#include "engine/SimpleObject.h"
#include "SDV_types.h"
#include <memory>
#include <gz4d/Color.h>
#include <gz4d/Point.h>

class Segment : public SimpleObject
{
public:
    typedef enum{Open, Disc, Sphere} SegmentCapType;

    Segment();
    ~Segment();

    void Draw(float degradation, Window3D *win);
    void DrawPoints(Window3D *win, float customStartColor[3] = NULL, float customEndColor[3] = NULL);
    void DrawStartPoint(Window3D *win, float customColor[3] = NULL);
    void DrawEndPoint(Window3D *win, float customColor[3] = NULL);

    void setPositions(gz4d::Point<float> const &s, gz4d::Point<float> const &e);

    void setCapTypes(SegmentCapType s,SegmentCapType e);

    void setColor(std::shared_ptr<gz4d::Color> &s, std::shared_ptr<gz4d::Color> &e);
    void setColor(std::shared_ptr<gz4d::Color>& c) {setColor(c,c);};

    void setScales(float *sw, float *sh, float *ew, float *eh);
    void setScales(float *w, float *h){setScales(w,h,w,h);};

    void setRadius(float *s, float *e){setScales(s,s,e,e);};
    void setRadius(float *r){setRadius(r,r);};

private:
    void drawCap(float deg, SegmentCapType type,float * ws, float * hs);
    void calculateTransforms();

    static Point3f * circle;
    static Point3f * cap;

    gz4d::Point<float> start,end;
    SegmentCapType startCap, endCap;

    float length;
    float heading;
    float elevation;

    std::shared_ptr<gz4d::Color> startColor;
    std::shared_ptr<gz4d::Color> endColor;

    float *startWidthScale, *startHeightScale, *endWidthScale, *endHeightScale;

};


#endif
