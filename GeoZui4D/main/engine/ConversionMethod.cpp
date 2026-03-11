// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// ConversionMethod.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  This class wraps the projection mechanisms of libproj with addressing.
 *
 *  This class defines the interface for converting information in
 *  ConversionGrids to GUTMs using information from Projections.
 *
 * Created by mdp 12/14/2001
 *---------------------------------------------------------------------------*/
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "widgets/Sv_Standards.h"
#include "utilities/Addressable.h"
#include "utilities/DottedAddress.h"
#include "engine/ConversionMethod.h"
#include "engine/ConversionGrid.h"
#include "objects/gutm.h"
#include <iostream>

using namespace std;

Addressable *ConversionMethodNeighborhoodAnchor;
DottedAddress *ConversionMethodNeighborhood = NULL;

extern MainWindow *mainWindow;

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
ConversionMethod::ConversionMethod(string name,
                               bool readonly)
: Addressable(name)
{
    readOnly = readonly;
    numRows = numCols = 0;
    spacing = 0.0;
    registerMethod();
}

/*---------------------------------------------------------------------------*/
ConversionMethod::~ConversionMethod()
{
}

/*---------------------------------------------------------------------------*/
void ConversionMethod::registerMethod()
{
    if (!ConversionMethodNeighborhood)
    {
        ConversionMethodNeighborhoodAnchor = new Addressable;
        ConversionMethodNeighborhood = new DottedAddress(ConversionMethodNeighborhoodAnchor, "Conversion");
    }
    ConversionMethodNeighborhood->addChild(getAddress());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void ConversionMethod::setNumColumns(int nCols)
{
    numCols = nCols;
}

/*---------------------------------------------------------------------------*/
void ConversionMethod::setNumRows(int nRows)
{
    numRows = nRows;
}

/*---------------------------------------------------------------------------*/
void ConversionMethod::setSpacing(float interval)
{
    spacing = interval;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int ConversionMethod::convert(Gutm *gutm, ConversionGrid *convGrid,
                              gz4d::geo::Projection const &proj)
{
    int retval;
    float val1, val2, midpoint;
    Scene3D *scene3d = mainWindow->getScene3D();
    gz4d::geo::Projection baseProjection = scene3d->getProjection();
    Point3d gr;

    //find projection if don't have one.
    if (!proj)
    {
        if (baseProjection)
            projection = baseProjection;
        else
        {
            if (gutm->centerGiven)
                midpoint = (float)gutm->GetPosition(X);
            else
                midpoint = (float)((convGrid->xcoords[0] + convGrid->xcoords[convGrid->numCols - 1]) / 2.0);
            projection = gz4d::geo::UTMProjection(gz4d::geo::UTMZone(midpoint));
        }
    }
    else
        projection = proj;

    //inform GeoZui4D of new base projection
    if (!baseProjection)
        scene3d->setProjection(projection);

    //set up class variables
    determineConvertedBounds(convGrid, projection);

    //grab gutm info
    targetGrid = gutm->dem;
    if (numCols <= 0)
        numCols = gutm->maxx;
    if (numRows <= 0)
        numRows = gutm->maxy;
    if (spacing <= 0)
        spacing = gutm->cellsize;

    //determine missing info
    if (spacing <= 0)
    {
        val1 = (float)((maxs[0] - mins[0]) / numCols);
        val2 = (float)((maxs[1] - mins[1]) / numRows);
        if (val1 > val2)
            spacing = val1;
        else
            spacing = val2;
    }
    else if (targetGrid == NULL)
    {
        numCols = (int)ceil((maxs[0] - mins[0]) / spacing);
        numRows = (int)ceil((maxs[1] - mins[1]) / spacing);
        gutm->maxx = numCols;
        gutm->maxy = numRows;
        retval = gutm->AllocateGrid();
        targetGrid = gutm->dem;
    }

    //grab conversion grid info
    gridObject = convGrid;
    sourceGrid = convGrid->grid;
    sourceRows = convGrid->numRows;
    sourceCols = convGrid->numCols;
    sourceSpacingX = convGrid->xSpacing;
    sourceSpacingY = convGrid->ySpacing;
    sourceCoordsX = convGrid->xcoords;
    sourceCoordsY = convGrid->ycoords;
    if (gutm->centerGiven)
        zOffset = (float)gutm->GetPosition(Z);
    else
        zOffset = -convGrid->minHeight + 1;
    //  zOffset = convGrid->minHeight - 1;

    //run the conversion
    retval = conversionMethod();

    //feed results back to the GUTM (in case it wasn't read from file)
    gutm->dem = targetGrid;
    gutm->maxx = numCols;
    gutm->maxy = numRows;
    gutm->cellsize = spacing;
    gutm->bbox.b = zOffset;
    gutm->bbox.t = zOffset + convGrid->maxHeight;
    gr[0] = mins[0];
    gr[1] = mins[1];
    gr[2] = -zOffset;
    gutm->SetPosition(gr);

    return retval;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** This procedures finds the bounding square of the ConversionGrid in
 *  projected coordinates according to the Projection.  It does this
 *  by picking two corners and following them along until it finds the
 *  respective local maxima or minima.  The results are stored in the
 *  class variables minX, maxX, minY, and maxY.
 */
void ConversionMethod::determineConvertedBounds(ConversionGrid *convGrid,
                                                gz4d::geo::Projection const &proj)
{
    float *xcoords = convGrid->xcoords, *ycoords = convGrid->ycoords;
    int numRows = convGrid->numRows, numCols = convGrid->numCols;
    int i;

    mins = proj(ycoords[0],xcoords[0]);
    maxs = proj(ycoords[numRows - 1],xcoords[numCols - 1]);

    //update minX by riding edge until find local minimum.
    for (i = 1; i < numRows; i++)
    {
        gz4d::Point<double> newVal = proj(ycoords[i],xcoords[0]);
        if (newVal[0] <= mins[0])
            mins[0] = newVal[0];
        else
            break;
    }
    //update maxX by riding edge until find local maximum.
    for (i = numRows - 2; i >= 0; i--)
    {
        gz4d::Point<double> newVal = proj(ycoords[i],xcoords[numCols - 1]);
        if (newVal[0] >= maxs[0])
            maxs[0] = newVal[0];
        else
            break;
    }
    //update minY by riding edge until find minimum.
    for (i = 1; i < numCols; i++)
    {
        gz4d::Point<double> newVal = proj(ycoords[0],xcoords[i]);
        if (newVal[1] <= mins[1])
            mins[1] = newVal[1];
    }
    //update maxY by riding edge until find maximum.
    for (i = numCols - 2; i >= 0; i--)
    {
        gz4d::Point<double> newVal = proj(ycoords[numRows - 1],xcoords[i]);
        if (newVal[1] >= maxs[1])
            maxs[1] = newVal[1];
    }
}
