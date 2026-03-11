// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
// QuadTreeTile.h: Class for multiscale tiled grids of objects that may
//  have a sort of mipmapped representation when zoomed out and/or
//  viewed at an angle.
//  USAGE NOTE: Any QuadTreeTile that has a child set MUST be created
//  using create(), which makes a std::shared_ptr so that it doesn't
//  go away!
//
//  Created by mdp 9/12/2008
//////////////////////////////////////////////////////////////////////

#ifndef QUADTREETILE_H
#define QUADTREETILE_H

class QuadTreeObject;
class QuadTreeCollection;

class ViewManager;
#include <memory>
#include <gz4d/Vector.h>

class QuadTreeTile
{
    friend class QuadTreeCollection;
public:
    //co-defined bitmasks so EAST is 01, SOUTH is 10 for ChildDirection.  Don't use NORTH or WEST as bit masks.
    enum SiblingDirection {NORTH, FIRST_SIBLING = NORTH, EAST, IS_EAST_OR_WEST = EAST, SOUTH, IS_SOUTH_OR_WEST = SOUTH, WEST, LAST_SIBLING = WEST, NUM_DIRECTIONS};
    enum ChildDirection {NORTHWEST, FIRST_CHILD = NORTHWEST, NORTHEAST, IS_EAST = NORTHEAST, SOUTHWEST, IS_SOUTH = SOUTHWEST, SOUTHEAST, LAST_CHILD = SOUTHEAST, NUM_CHILDREN};
    enum DrawingStatus {NOT_DRAWING, DRAWING};

    static std::shared_ptr<QuadTreeTile> create(std::shared_ptr<QuadTreeObject>);
    ~QuadTreeTile();

    void setChild(ChildDirection which, std::shared_ptr<QuadTreeObject>);
    void setChild(ChildDirection which, std::shared_ptr<QuadTreeTile>);
    void reparentTo(std::shared_ptr<QuadTreeTile>);
    std::weak_ptr<QuadTreeTile> getChild(ChildDirection which) {return children[which];}
    std::weak_ptr<QuadTreeTile> getSibling(SiblingDirection which) {return siblings[which];}
    std::weak_ptr<QuadTreeTile> *getSiblings() {return siblings;}
    std::weak_ptr<QuadTreeTile> getParent() {return parent;}

    void setStatus(int newStatus); //default is propogate regardless
    int getStatus() {return drawingStatus;}

    QuadTreeObject *getObject() {return &(*visibleObject);}
    std::weak_ptr<QuadTreeTile> getTileAt(double x, double y, int level);

    struct RenderInfo {
        RenderInfo(ViewManager *v);

        ViewManager *view;
        gz4d::Vector<double, 3> eyePoint, screenCenter,
            topLeft, topRight, bottomLeft, bottomRight,
            backCorner;
        gz4d::Vector<double, 3> S, L, R, T, B, F;
        double S_S, L_L, R_R, T_T, B_B, F_S, LL_SS, RR_SS, TT_SS, BB_SS;
        double frontPixelSize;
        double pixelMultiplier;
        bool isRoot;
    };
    bool render(QuadTreeCollection &qtc, RenderInfo &renderInfo, int deepestLevel);

    static const double basePixelMultiplier;

protected:
    QuadTreeTile(std::shared_ptr<QuadTreeObject>);
    void setSibling(SiblingDirection which, std::shared_ptr<QuadTreeTile>);
    void reorientFamilyTiesFor(ChildDirection which);
    bool beginFusionCheck(double scale, double pixelSize);
    void endFusionCheck();
    float minScale, maxScale;
    bool fusionEdgeTile, needsPop;
    double oldZValue;

    int drawingStatus;

    static std::weak_ptr<QuadTreeTile> nullTile;
    std::weak_ptr<QuadTreeTile> self;
    std::weak_ptr<QuadTreeTile> parent;
    std::weak_ptr<QuadTreeTile> siblings[4]; //pointers to siblings
    std::shared_ptr<QuadTreeTile> children[4]; //pointers to children
    std::shared_ptr<QuadTreeObject> visibleObject; //the thing that will be drawn
};

#endif
