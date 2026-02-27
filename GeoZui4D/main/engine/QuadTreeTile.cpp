/* Copyright (c) 2008 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
// QuadTreeTile.cpp: Class for multiscale tiled grids of objects that may
//  have a sort of mipmapped representation when zoomed out and/or
//  viewed at an angle.
//
//  Created by mdp 9/12/2008
//////////////////////////////////////////////////////////////////////

#include "engine/QuadTreeTile.h"
#include "engine/QuadTreeObject.h"
#include "engine/QuadTreeCollection.h"
#include "engine/PosableStateBase.h"
#include "engine/Object.h"
#include "engine/ViewManager.h"

std::weak_ptr<QuadTreeTile> QuadTreeTile::nullTile;
const double QuadTreeTile::basePixelMultiplier = 1.0; //sqrt(2.0);

/*---------------------------------------------------------------------------*/
std::shared_ptr<QuadTreeTile> QuadTreeTile::create(std::shared_ptr<QuadTreeObject> object)
{
    std::shared_ptr<QuadTreeTile> instance(new QuadTreeTile(object));
    instance->self = instance;
    if (object)
        object->setTile(instance);
    return instance;
}

/*---------------------------------------------------------------------------*/
QuadTreeTile::QuadTreeTile(std::shared_ptr<QuadTreeObject> object)
{
    visibleObject = object;
    drawingStatus = 0;
    for (int i = 0; i < 4; i++)
    {
        siblings[i] = std::weak_ptr<QuadTreeTile>();
        children[i] = std::shared_ptr<QuadTreeTile>();
    }
    minScale = maxScale = -1.0;
    fusionEdgeTile = false;
    needsPop = false;
}

/*---------------------------------------------------------------------------*/
QuadTreeTile::~QuadTreeTile()
{
}

/*---------------------------------------------------------------------------*/
void QuadTreeTile::setChild(ChildDirection which, std::shared_ptr<QuadTreeObject> object)
{
    setChild(which, create(object));
}

/*---------------------------------------------------------------------------*/
void QuadTreeTile::setChild(ChildDirection which, std::shared_ptr<QuadTreeTile> child)
{
    if (which <= LAST_CHILD && which >= FIRST_CHILD)
    {
        children[which] = child;
        children[which]->reparentTo(std::shared_ptr<QuadTreeTile>(self));
        reorientFamilyTiesFor(which);
    }
}

/*---------------------------------------------------------------------------*/
void QuadTreeTile::reparentTo(std::shared_ptr<QuadTreeTile> newParent)
{
    parent = newParent;
}

/*---------------------------------------------------------------------------*/
void QuadTreeTile::setSibling(SiblingDirection which, std::shared_ptr<QuadTreeTile> tile)
{
    siblings[which] = tile;
}

/*---------------------------------------------------------------------------*/
void QuadTreeTile::setStatus(int newStatus)
{
    drawingStatus = newStatus;
    for (int i = 0; i < 4; i++)
    {
        if (children[i])
            children[i]->setStatus(drawingStatus);
    }
}

/*---------------------------------------------------------------------------*/
void QuadTreeTile::reorientFamilyTiesFor(ChildDirection which)
{
    /* //I'm having difficulty with unbound QuadTreeTile shared_ptr's
       //so the sibling hookup will have to go on outside...
    std::shared_ptr<QuadTreeTile> childTile = children[which], otherTile;
    SiblingDirection toChild, toOther;

    //link east/west children to each other
    otherTile = children[which ^ EAST];
    toOther = (which & EAST)? WEST : EAST;
    childTile->setSibling(toOther, otherTile);
    if (otherTile)
    {
        toChild = (which & EAST)? EAST : WEST;
        otherTile->setSibling(toChild, childTile);
    }

    //link north/south children to each other
    otherTile = children[which ^ SOUTH];
    toOther = (which & SOUTH)? NORTH : SOUTH;
    childTile->setSibling(toOther, otherTile);
    if (otherTile)
    {
        toChild = (which & SOUTH)? SOUTH : NORTH;
        otherTile->setSibling(toChild, childTile);
    }
    */
}

/*--------------------------------------------------------------------------*/
std::weak_ptr<QuadTreeTile> QuadTreeTile::getTileAt(double x, double y, int level)
{
    BBox bbox;
    int child;
    std::weak_ptr<QuadTreeTile> result;

    if (!visibleObject)
        return nullTile;

    bbox = visibleObject->GetBoundingBox();
    if (bbox.w > x || bbox.e < x || bbox.n < y || bbox.s > y)
        return nullTile; //outside of this tile.

    //if we got here, then there's something in this tile... need
    //to find the right level now.
    if (level == 0)
        return self;

    //We don't know our own size, so ask our children...
    for (child = FIRST_CHILD; child <= LAST_CHILD; child++)
    {
        if (!children[child])
            continue;
        result = children[child]->getTileAt(x, y, level - 1);
        if (!result.expired())
            return result; //stop as soon as we find something
    }

    return result;
}

/*--------------------------------------------------------------------------*/
QuadTreeTile::RenderInfo::RenderInfo(ViewManager *v)
: view(v),
  eyePoint(v->getEyePoint()),
  screenCenter(v->getScreenCenterPoint()),
  topLeft(v->getCorner(ViewManager::FRONT_TOP_LEFT)),
  topRight(v->getCorner(ViewManager::FRONT_TOP_RIGHT)),
  bottomLeft(v->getCorner(ViewManager::FRONT_BOTTOM_LEFT)),
  bottomRight(v->getCorner(ViewManager::FRONT_BOTTOM_RIGHT)),
  backCorner(v->getCorner(ViewManager::BACK_TOP_LEFT)),
  S(screenCenter - eyePoint),
  L(((topLeft + bottomLeft) / 2) - eyePoint),
  R(((topRight + bottomRight) / 2) - eyePoint),
  T(((topLeft + topRight) / 2) - eyePoint),
  B(((bottomLeft + bottomRight) / 2) - eyePoint),
  F(backCorner - eyePoint),
  isRoot(true)
{
    //Calculate key vectors
    //S = screenCenter - eyePoint;
    //L = ((topLeft + bottomLeft) / 2) - eyePoint;
    //R = ((topRight + bottomRight) / 2) - eyePoint;
    //F = backCorner - eyePoint;

    //Calculate often-needed dot-products
    S_S = S.dot(S);
    L_L = L.dot(L);
    R_R = R.dot(R);
    T_T = T.dot(T);
    B_B = B.dot(B);
    F_S = F.dot(S);

    //Calculate often-needed quotients
    LL_SS = L_L / S_S,
    RR_SS = R_R / S_S;
    TT_SS = T_T / S_S;
    BB_SS = B_B / S_S;
}

/*--------------------------------------------------------------------------*/
bool QuadTreeTile::render(QuadTreeCollection &qtc, RenderInfo &renderInfo, int deepestLevel)
{
    int i;
    bool isRoot = renderInfo.isRoot;
    double depthPixelSize, C_S, C_L, C_R, C_T, C_B, CS_SS;
    gz4d::Vector<double, 3> *C;

    double scale = renderInfo.view->getPosable()->getScale();

    //1. Initialize array indicating which children have drawn themselves
    bool childHasDrawn[NUM_CHILDREN] = {false};

    //2. Calculate the vector from the eyepoint to each tile corner
    BBox bbox = visibleObject->GetBoundingBox();
    gz4d::Vector<double, 3> corners[4] = {
        gz4d::Vector<double, 3>(bbox.w, bbox.n, bbox.t) - renderInfo.eyePoint,
        gz4d::Vector<double, 3>(bbox.e, bbox.n, bbox.t) - renderInfo.eyePoint,
        gz4d::Vector<double, 3>(bbox.w, bbox.s, bbox.t) - renderInfo.eyePoint,
        gz4d::Vector<double, 3>(bbox.e, bbox.s, bbox.t) - renderInfo.eyePoint
    };

    //3-6. Initialize some variables
    bool inFrustum[NUM_CHILDREN] = {false};
    int numInside = 0, numAbove = 0, numBelow = 0, numLeft = 0, numRight = 0;
    double minSize = -1.0;
    bool crossesFrontPlane = false;
    bool inX, inY;

    //7. Determine who's inside, and what the relative pixel size is
    for (i = 0; i < NUM_CHILDREN; i++)
    {
        C = &(corners[i]);
        C_S = C->dot(renderInfo.S); //substep 1
        C_L = C->dot(renderInfo.L);
        C_R = C->dot(renderInfo.R);
        C_T = C->dot(renderInfo.T);
        C_B = C->dot(renderInfo.B);
        CS_SS = C_S / renderInfo.S_S; //substep 2
        depthPixelSize = renderInfo.frontPixelSize * CS_SS; //substep 3
        if (minSize > depthPixelSize || minSize < 0.0) //substep 4
            minSize = depthPixelSize;
        if (CS_SS < 1.0) //substep 5
            crossesFrontPlane = true;
        //substeps 6-9
        inX = inY = false;
        if (C_L > C_S * renderInfo.LL_SS)
            numLeft++;
        else if (C_R > C_S * renderInfo.RR_SS)
            numRight++;
        else
            inX = true;
        if (C_T > C_S * renderInfo.TT_SS)
            numAbove++;
        else if (C_B > C_S * renderInfo.BB_SS)
            numBelow++;
        else
            inY = true;
        if (!inX || !inY || C_S > renderInfo.F_S || crossesFrontPlane)
            continue;
        inFrustum[i] = true; //substep 10
        numInside++; //substep 11
    }

    if (numInside == 0) //8. Are tile corners all off to one side?
    {
        if (numLeft >= 4 || numRight >= 4 ||
            numAbove >= 4 || numBelow >= 4)
            return false; //nothing to draw!
    }

    //Make it so isRoot is only true for top level call
    if (isRoot)
        renderInfo.isRoot = false;

    //9. Otherwise, at least one corner is in the viewing frustum...
    //(We're not using an "else" here because we want to get here
    //if numInside == 0, but the frustum is inside the tile.
    double pixelSize = visibleObject->getPixelSize() * renderInfo.pixelMultiplier;
    //substep 1
    if (numInside < 4 && crossesFrontPlane && minSize > renderInfo.frontPixelSize)
        minSize = renderInfo.frontPixelSize;
    //substep 2
    if ((minSize > pixelSize * 2.0) && !isRoot) //don't draw if detail is too high
        return false;
    //substep 3
    if (deepestLevel <= 0 || minSize >= pixelSize)
    {
        if (!beginFusionCheck(scale, pixelSize))
            return true; //we don't want to draw anything here... fool parent into thinking we did.
        qtc.DrawObjectLowRes(visibleObject.get(), 0.0, renderInfo.view->getParentWin());
        endFusionCheck();
        return visibleObject->renderedOnMostRecentTry();
    }
    //substep 4
    for (i = 0; i < NUM_CHILDREN; i++)
    {
        if (children[i])
            childHasDrawn[i] = children[i]->render(qtc, renderInfo, deepestLevel-1);
    }
    
    //10. Draw what the children haven't
    if (!beginFusionCheck(scale, pixelSize))
        return true; //we don't want to draw anything here... fool parent into thinking we did.
    visibleObject->resetChildrenDrawn(childHasDrawn);
    qtc.DrawObjectLowRes(visibleObject.get(), 0.0, renderInfo.view->getParentWin());
    visibleObject->resetChildrenDrawn();
    endFusionCheck();

    return visibleObject->renderedOnMostRecentTry();
}

/*--------------------------------------------------------------------------*/
bool QuadTreeTile::beginFusionCheck(double scale, double pixelSize)
{
    if ((minScale >= 0 && scale < minScale) || (maxScale >= 0 && scale > maxScale))
    {
        if (fusionEdgeTile) //draw "under" our normal depth
        {
            PosableStateBase *pose = visibleObject->getPosable();
            //"push" a transformation on the object
            oldZValue = pose->getPosition()[Z];
            pose->AccumulateNotifications();
            pose->setPosition(0.0, 0.0, oldZValue - pixelSize, ChangeTranslateZ);
            needsPop = true;
        }
        else
            return false; //we don't want to draw anything here
    }
    return true;
}

/*--------------------------------------------------------------------------*/
void QuadTreeTile::endFusionCheck()
{
    if (needsPop)
    {
        PosableStateBase *pose = visibleObject->getPosable();
        pose->setPosition(0.0, 0.0, oldZValue, ChangeTranslateZ);
        pose->ResumeSingularNotification(true); //purge notifications
    }
    needsPop = false;
}