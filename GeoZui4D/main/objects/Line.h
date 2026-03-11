// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////

#ifndef LINE_H
#define LINE_H

#include "objects/SegmentUser.h"

class Segment;
class TimeLine;
class TimeSequence;


struct LineNodeType
{
    gz4d::Point<double> position;
    std::shared_ptr<gz4d::Color> color;
    float *width;
    float *height;
    Segment * segment;
    LineNodeType * next;
};

class Line: public SegmentUser
{
    friend class TimeLine;

public:
    Line();
    ~Line();

    DERIVE_OBJECT(Line);
    void DrawLowRes(float degradation, Window3D *win);

    void DrawLowResSection(float degradation, Window3D *win, LineNodeType *startNode, LineNodeType *endNode);

    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);

    void setColor(gz4d::Color const &c) {*startColor=c; *endColor = c; lineDLNeedsRebuild = true;}
    gz4d::Color const &getColor(){return *startColor;};
    void setHighlightColor(gz4d::Color const &c) {highlightColor = c; haveHighlight = true;}
    gz4d::Color const &getHighlightColor(){return highlightColor;};

    void setSize(float s) {startWidth = startHeight = endWidth = endHeight = s;}

    void drawTube(bool d = true) {drawTubeFlag = d;}
    void remove(LineNodeType *node);

    bool deletingInfo, drawForPicking;
    virtual void getHtmlInfo(std::stringstream &html, short level);
    void setTimeLine(TimeLine * tl) {time = tl;};
private:
    LineNodeType * newLineNode();

    LineNodeType * lineNodes;
    GLuint lineDisplayList, tubeDisplayList;
    bool lineDLNeedsRebuild, tubeDLNeedsRebuild;
    gz4d::Color highlightColor;
    bool haveHighlight;

    LineNodeType * lastStartNode;
    LineNodeType * lastStopNode;

    bool drawTubeFlag; 

    TimeLine * time;
};

#endif

