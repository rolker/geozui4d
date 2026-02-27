/* Copyright (c) 2002 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///Note: The tiles build left to right, top to bottom

#include "engine/QuadTreeCollection.h"
#include "engine/QuadTreeObject.h"
#include "engine/QuadTreeTile.h"
#include "engine/Window3D.h"
#include "engine/PosableStateBase.h"
#include "engine/ViewManager.h"
#include "engine/Scene3D.h"
#include "engine/gzCommandInterface.h"
#include <boost/filesystem.hpp>
#include <queue>
#include <gz4d/vector/ios.h>

REGISTER_OBJECT(QuadTreeCollection, "");

/*--------------------------------------------------------*/
QuadTreeCollection::QuadTreeCollection(GLboolean allocatePosable, int pickingTypes)
:ObjectCollection(allocatePosable, pickingTypes)
{
    leafDepth = 0;
    leafPixelSize = -1.0;
    cellCountX = cellCountY = 0;
}

/*--------------------------------------------------------*/
int QuadTreeCollection::DrawHighRes(int eye, Window3D *win)
{
    DrawLowRes(0.0, win);//for now, just let the low res renderer take care of it...
    return 1;
}

/*--------------------------------------------------------*/
void QuadTreeCollection::DrawLowRes(float degradation, Window3D *win)
{
    QuadTreeObject *root = quadTreeRoot->getObject();

    if (!root) //Nothing there!
        return;

    //quadTreeRoot->setStatus(QuadTreeTile::NOT_DRAWING);

    //1. All tiles know pixel size.
    //Steps 2-4 done in initializing renderInfo...
    QuadTreeTile::RenderInfo renderInfo(win->getViewInfo());

    //New step: calculate a multiplier based on angle of viewing
    renderInfo.pixelMultiplier = QuadTreeTile::basePixelMultiplier
                                 * sqrt(sin(DEGTORAD * 
                                       win->getPoseInfo()->getOrientationAxis()[Pitch]));
    if (renderInfo.pixelMultiplier < 0.01)
        renderInfo.pixelMultiplier = 0.01;
    
    //5. Set the deepestLevel to initially be the deepest we can go
    int deepestLevel = leafDepth;

    //6. Hold onto the deepest-level's pixel size
    double minTilePixelSize = fabs(leafPixelSize);

    //7. Determine the pixel size at the front clipping plane
    gz4d::Vector<double, 3> crossScreen(renderInfo.R - renderInfo.L);
    double frontPixelSize = sqrt(crossScreen.dot(crossScreen)) / (double)win->getWidth();
    renderInfo.frontPixelSize = frontPixelSize;

    //8. Determine the deepest level we could need to render at
    while ((minTilePixelSize * renderInfo.pixelMultiplier) < frontPixelSize)
    {
        minTilePixelSize *= 2.0;
        deepestLevel--;
    }

    //9. Tell the tile to render
    quadTreeRoot->render(*this, renderInfo, deepestLevel);
}

/*--------------------------------------------------------*/
double QuadTreeCollection::getTileSize(int atLevel, int minMaxAvg)
{
    double sizeX, sizeY;
    int multiplier;

    if (atLevel == LeafLevel)
        atLevel = leafDepth;
    
    multiplier = (1 << (leafDepth - atLevel));

    sizeX = fabs(leafStepX * multiplier);
    sizeY = fabs(leafStepY * multiplier);

    return calcValue(sizeX, sizeY, minMaxAvg);
}

/*--------------------------------------------------------*/
double QuadTreeCollection::getCellsInTile(int minMaxAvg)
{
    double countX, countY;

    countX = cellCountX;
    countY = cellCountY;

    return calcValue(countX, countY, minMaxAvg);
}

/*--------------------------------------------------------*/
double QuadTreeCollection::calcValue(double x, double y, int minMaxAvg)
{
    switch (minMaxAvg)
    {
        case MaxXY:
            if (x > y)
                return x;
            return y;
            break;
        case AvgXY:
            return (x + y) / 2.0;
            break;
    }
    //default case MinXY:
    if (x < y)
        return x;
    return y;
}

/*--------------------------------------------------------*/
int QuadTreeCollection::RemoveObject(Object* object)
{
    ObjectCollection::RemoveObject(object);
    //&&&And remove from the QuadTree, I presume
	return TCL_OK;
}

/*--------------------------------------------------------*/
int QuadTreeCollection::ReloadObject(Object* object)
{
    ObjectCollection::ReloadObject(object);
    //&&&Anything else needed?
	return TCL_OK;
}

/*--------------------------------------------------------*/
Object * QuadTreeCollection::LoadObject(std::string const &fn)
{
    Object *retval;
    retval = ObjectCollection::LoadObject(fn);
    //&&&And add to the quadtree, I assume
    return retval;
}

/*--------------------------------------------------------------------------*/
/**	This function returns a height summary of the specified area surrounding
*	the passed location, based on the results returned from each child object
*	of this collection.  Defaults to the maximum height (Object::height_max)
*	but can also do Object::height_min and Object::height_average.  Returns
*	-1.0 if a height is not found near this location or if the range doesn't
*	cross any cells in the GUTM.  The area is always a box with the length
*	of twice the rangeMeters parameter on each side.  Right now, the function
*	is very crude, in that it works only with cell values and does not do any
*	interpolation.  Note: height_average weights each child's contribution
*	equally, and may result in undesirable behavior.
*	Function created by mdp 1/3/2006 to support vessel shadow functionality.
*/
double QuadTreeCollection::GetHeightNear(double x, double y, float rangeMeters, int heightType)
{
    //&&&Might be important to limit to just the leaf-level, or to the currently-showing level
    return ObjectCollection::GetHeightNear(x, y, rangeMeters, heightType);
}

/*--------------------------------------------------------------------------*/
double QuadTreeCollection::GetHeightAt(double x, double y, Point3d coord, GLboolean secondary)
{
    //&&&Might be important to limit to just the leaf-level, or to the currently-showing level
    return ObjectCollection::GetHeightAt(x, y, coord, secondary);
}

/*--------------------------------------------------------------------------*/
/*
void QuadTreeCollection::WriteToScene(string scenepath)
{
    ObjectCollection::WriteToScene(scenepath);
}
*/

/*--------------------------------------------------------------------------*/
/** Loads (by looping) textures, objects, collections, scripts and animators from an XML document.
 * @param[in] xml gz4d::Xml instance from which to load.
 * @param[in] cur the current item that is being evaluated.
 * @param[in] path of the directory from which the XML file is loaded.
 * \return Object* - The entire collection of objects
 * \par Purpose: 
 * To process all valid portions of an XML formatted file (i.e. .gzx).
 * \note LoadXMLObject() is used for objects and AddObject() is used for collections of objects.  
 * \sa LoadXMLObject(), AddObject()
 * \author Roland Arsenault (rja)
 * \date 2002
 */
Object * QuadTreeCollection::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    gz4d::Point<double> position; // georefrence
    bool usePosition = false, useFilePattern = false;

    firstX = endX = 0;
    firstY = endY = 0;
    stepX = stepY = 1;
    leafStepX = leafStepY = 0.0;
    indexRows = indexCols = -1;
    rowMajorOrder = true;
    createObjects = false;

    // read values from XML
    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.file_pattern"))
    {
        filePattern = *attribute;
        useFilePattern = true;
    }
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.object_type"))
    {
        objectType = *attribute;
        createObjects = true;
    }
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.local_origin"))
    {
        std::stringstream ss(*attribute);
        ss >> position;
        usePosition = true;
    }

    leafStepX = xml.get("<xmlattr>.leaf_step_x",leafStepX);
    leafStepY = xml.get("<xmlattr>.leaf_step_y",leafStepY);
    firstX = xml.get("<xmlattr>.first_x",firstX);
    endX = xml.get("<xmlattr>.end_x",endX);
    stepX = xml.get("<xmlattr>.step_x",stepX);
    firstY = xml.get("<xmlattr>.first_y",firstY);
    endY = xml.get("<xmlattr>.end_y",endY);
    stepY = xml.get("<xmlattr>.step_y",stepY);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.index_order"))
        rowMajorOrder = (*attribute == "row");

    indexCols = xml.get("<xmlattr>.index_cols",indexCols);
    indexRows = xml.get("<xmlattr>.index_rows",indexRows);
    minScale = xml.get("<xmlattr>.min_scale",minScale);
    maxScale = xml.get("<xmlattr>.max_scale",maxScale);

    if (usePosition)
        SetPosition(position);

    //set up loop values if index_cols specified
    if (indexCols > 0 && endX <= 0)
        endX = firstX + stepX * indexCols;
    //set up loop values if index_rows specified
    if (indexRows > 0 && endY <= 0)
        endY = firstY + stepY * indexRows;

    if (useFilePattern)
    {
        if (!LoadXMLTiles(xml, path, ncfile, usePosition))
            return NULL;
    }

    Object::LoadXML(xml, path, ncfile);
    LoadXMLTiles(xml, path, ncfile);

    return this;
}

/*--------------------------------------------------------------------------*/
/** This loads multiple files based on the filePattern and other information
 *  already read in from the header description.  If the instance variable
 *  createObjects is true, then objects are created instead of loaded, and
 *  filePattern holds the attribute that has the filePattern (so we know
 *  which attribute to replace in the new object).
 * \author Matt Plumlee (mdp)
 * \date 10/10/2008
 */
bool QuadTreeCollection::LoadXMLTiles(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile, bool alreadyPositioned)
{
    Object *ob = NULL;
    QuadTreeObject *prototypeObject = NULL, *copyObject = NULL;
    std::shared_ptr<QuadTreeObject> copyOb;
    std::shared_ptr<QuadTreeTile> newTile;
    bpt::ptree prototypeElement, patternElement, layerElement;
    int i, tileX = 0, tileY = 0, otherX, otherY;
    std::vector<std::vector<std::shared_ptr<QuadTreeTile> > > tiles(indexCols > 0? indexCols : 0);
    bool multipattern = false, potentialEdge = false;
    int eventualIndexCols = -1;

    currentX = currentY = currentI = currentL = 0;
    if (indexCols > 0 && indexRows > 0)
    {
        for (i = 0; i < indexCols; i++)
            tiles[i].resize(indexRows);
        currentI = firstY * indexCols + firstX;
    }

    currentPath = path;
    
    for(bpt::ptree::const_iterator n = xml.begin(); n != xml.end(); ++n)
    {
        //first load the prototype object
        if (n->first == "Object")
        {
            if (!prototypeElement.empty() && (indexRows <= 0 || indexCols <= 0))
            {
                std::cerr << "Multiple Object elements not supported in QuadTreeCollections without positive index_rows and index_cols" << std::endl;
                return false;
            }
            prototypeElement = n->second;
            //load the object as a normal ObjectCollection would.
            ob = LoadXMLObject(n->second, path, ncfile);
            //see if it is valid.
            DYNAMIC_CAST(prototypeObject, QuadTreeObject *, ob);
            if (!prototypeObject)
            {
                std::cerr << "Could not convert Object to QuadTreeObject--must be a QuadTree-enabled Object!" << std::endl;
                return false;
            }
            if (!alreadyPositioned)
            {
                SetPosition(prototypeObject->GetPosition());
                alreadyPositioned = true;
            }
            //set up for patterning off of this one
            prototypeObject->actAsTemplate();
            templateObject = std::shared_ptr<QuadTreeObject>(prototypeObject);
            newTile = QuadTreeTile::create(templateObject);
            //add to the quadtree
            tiles[tileX][tileY] = newTile;
            bumpIndex(tileX, tileY);
            cellCountX = prototypeObject->getCellsWide();
            cellCountY = prototypeObject->getCellsTall();
            currentI++;
            leafPixelSize = prototypeObject->getPixelSize();
        }
        //now get the pattern for loading the remaining objects.
        else if (n->first == "Pattern")
        {
            if (prototypeElement.empty())
            {
                std::cerr << "Must have an Object element before a Pattern element in QuadTreeCollection " << GetLabel() << std::endl;
                return false;
            }
            if (!patternElement.empty())
            {
                std::cerr << "Multiple pattern elements not yet supported in QuadTreeCollections...";
                std::cerr << std::endl << "Trying to guess what you want..." << std::endl;
                multipattern = true;
            }
            patternElement = n->second;
            //Loop through based on the pattern specs already loaded,
            //stopping if we find a file we can't use.
            for (currentY = firstY; currentY != endY; currentY += stepY)
            {
                if (currentY + stepY == endY)
                    potentialEdge = true;
                else
                    potentialEdge = false;
                for (currentX = firstX; currentX != endX; currentX += stepX)
                {
                    //skip over the very first one, since that's the prototype
                    if (!multipattern && (currentX == firstX && currentY == firstY))
                        continue;

                    if (currentX + stepX == endX)
                        potentialEdge = true;
                    copyObject = createPatternedObject(patternElement, (currentX - firstX) * leafStepX, (currentY - firstY) * leafStepY, potentialEdge);
                    //copyObject->readContents();
                    //AddObject(copyObject);
                    
                    //add to the quadtree
                    copyOb = std::shared_ptr<QuadTreeObject>(copyObject);
                    newTile = QuadTreeTile::create(copyOb);
                    tiles[tileX][tileY] = newTile;
                    bumpIndex(tileX, tileY);
                    currentI += stepX;
                }
                if ((endX - firstX) < indexCols)
                {
                    currentI += indexCols - (endX - firstX);
                    eventualIndexCols = tileX;
                    tileX = 0;
                    tileY++;
                }
                if (stepY < 0 && indexCols > 0)
                    currentI += (2 * stepY * indexCols);
            }
        }
        else if (n->first == "LayerPattern")
        {
            if (patternElement.empty())
            {
                std::cerr << "Pattern element must come before LayerPattern Element.";
                std::cerr << std::endl << "Bailing out!" << std::endl;
                return false;
            }
            if (!layerElement.empty())
            {
                std::cerr << "Multiple LayerPattern elements not supported in QuadTreeCollections.";
                std::cerr << std::endl << "Bailing out!" << std::endl;
                return false;
            }
            layerElement = n->second;
        }
    }

    //At this point, tileX and tileY become size of the quadtree, rather than the file grid
    actualCols = indexCols;
    if (eventualIndexCols > -1)
        actualCols = eventualIndexCols;
    else if (indexCols == 0)
        actualCols = indexCols = (endX - firstX / stepX);
    if (stepY < 0)
        actualRows = (endY - firstY / stepY);
    else
        actualRows = tileY;

    //attach child tiles to each other
    int direction;
    
    for (tileX = 0; tileX < actualCols; tileX++)
    {
        for (tileY = 0; tileY < actualRows; tileY++)
        {
            for (direction = QuadTreeTile::FIRST_SIBLING;
                 direction <= QuadTreeTile::LAST_SIBLING;
                 direction++)
            {
                otherX = tileX;
                otherY = tileY;
                if (direction & QuadTreeTile::IS_EAST_OR_WEST)
                    otherX += (direction & QuadTreeTile::IS_SOUTH_OR_WEST)? -1 : 1;
                else
                    otherY += (direction & QuadTreeTile::IS_SOUTH_OR_WEST)? 1 : -1;
                if (otherX >= 0 && otherX < actualCols &&
                    otherY >= 0 && otherY < actualRows)
                {
                    tiles[tileX][tileY]->setSibling(
                        (QuadTreeTile::SiblingDirection)direction, tiles[otherX][otherY]);
                }             
            }
        }
    }

    //create hierarchy over the tiles
    createHierarchyUnder(quadTreeRoot, tiles, layerElement);

    //create/load hierarchy of mipmap-like images
    if (layerElement.empty())
        quadTreeRoot->getObject()->combineChildrenOfTile();

    //set our position
    pose->AccumulateNotifications();
    SetPosition(quadTreeRoot->getObject()->GetPosition());
    pose->ResumeSingularNotification(true); //purge notifications
    return true;
}

/*--------------------------------------------------------------------------*/
//Create a copy of the template
QuadTreeObject *QuadTreeCollection::createPatternedObject(bpt::ptree const &elem,
                               float xOffset, float yOffset,
                               bool potentialEdge, float scaleFactor,
                               bool actAsRoot)
{
    QuadTreeObject *copyObject = NULL;

    //create a shallow copy
    //Note: we're building left to right, top to bottom.
    copyObject = templateObject->offsetCopy(xOffset, yOffset, scaleFactor);
    if (potentialEdge)
        copyObject->actAsPotentialEdge();
    if (actAsRoot)
        copyObject->actAsRoot();

    //now fill in the details
    if (!elem.empty())
    {
        std::string name, patternValue;
        bpt::ptree attribs = elem.get_child("<xmlattr>");
        for(bpt::ptree::const_iterator a = attribs.begin(); a != attribs.end(); ++a)
        {
            name = a->first;
            patternValue = a->second.data();
            copyObject->setXMLattribute(name, patternSubstitution(patternValue), currentPath.string());
        }
        copyObject->readContents();
    }
    else
    {
        char name[100];
        sprintf(name, "%d-span_%d_%d", 1 << currentL, currentX, currentY);
        copyObject->SetLabel(name);
    }
    AddObject(copyObject);

    return copyObject;
}

/*--------------------------------------------------------------------------*/
// Recursive method to create a hierarchy of QuadTreeTiles under the passed
// parent.  If the parent pointer is not set, a new QuadTreeTile instance is
// created to be the parent.
void QuadTreeCollection::createHierarchyUnder(
    std::shared_ptr<QuadTreeTile> &parent,
    std::vector<std::vector<std::shared_ptr<QuadTreeTile> > > tiles,
    bpt::ptree  const &layerElement,
    int startX, int startY, int halfWidth
    )
{
    int i, x, y;
    QuadTreeObject *copyObject;
    std::shared_ptr<QuadTreeObject> copyOb;
    std::shared_ptr<QuadTreeTile> midParent;
    bool potentialEdge;

    //Top-level call?  Then determine the proper starting stride
    if (halfWidth < 0 || !parent)
    {
        int maxSize;
        maxSize = actualCols;
        if (maxSize < actualRows)
            maxSize = actualRows;

        //find the first power of two that encompases the maxSize
        currentL = 0;
        for (halfWidth = 1; halfWidth < maxSize; halfWidth <<= 1)
            currentL++;//currentL will be one power above halfWidth when done...
        //now half it
        halfWidth >>= 1;

        if (!parent)
        {
            currentX = currentY = currentI = 0;
            copyObject = createPatternedObject(layerElement, startX, startY, true, 2*halfWidth, true);
            copyOb = std::shared_ptr<QuadTreeObject>(copyObject);
            parent = QuadTreeTile::create(copyOb);
            leafDepth = 0;
        }
    }
    currentL--; //currentLayer
    for (i = QuadTreeTile::FIRST_CHILD; i <= QuadTreeTile::LAST_CHILD; i++)
    {
        x = startX + ((i & QuadTreeTile::IS_EAST)? (halfWidth) : 0);
        y = startY + ((i & QuadTreeTile::IS_SOUTH)? (halfWidth) : 0);

        if (x < actualCols && y < actualRows)
        {
            if (x == 0 && y == 0)
                leafDepth++; //keep track of the depth on initial descent.

            if (halfWidth <= 1) //base case of the recursion
            {
                parent->setChild((QuadTreeTile::ChildDirection)i, tiles[x][y]);
            }
            else //recurse
            {
                currentX = x; currentY = y;
                currentI = ((y * ((indexCols + halfWidth - 1) / halfWidth)) + x) / halfWidth;
                if (x + halfWidth >= endX || y + halfWidth >= endY)
                    potentialEdge = true;
                else
                    potentialEdge = false;
                copyObject = createPatternedObject(layerElement, x * leafStepX, y * leafStepY, potentialEdge, halfWidth);
                copyOb = std::shared_ptr<QuadTreeObject>(copyObject);
                midParent = QuadTreeTile::create(copyOb);
                parent->setChild((QuadTreeTile::ChildDirection)i, midParent);
                createHierarchyUnder(midParent, tiles, layerElement, x, y, halfWidth >> 1);
            }
        }
        //else, no children at this point
    }

    //hook parent up to any siblings
    std::shared_ptr<QuadTreeTile> neighbor;
    int sibDir, childDir;
    bool outside[QuadTreeTile::NUM_CHILDREN][QuadTreeTile::NUM_DIRECTIONS] =
    { {true, false, false, true},
    {true, true, false, false},
    {false, false, true, true},
    {false, true, true, false}};

    for (i = QuadTreeTile::FIRST_CHILD; i <= QuadTreeTile::LAST_CHILD; i++)
    {
        for (sibDir = QuadTreeTile::FIRST_SIBLING; sibDir <= QuadTreeTile::LAST_SIBLING; sibDir++)
        {
            childDir = (sibDir & QuadTreeTile::IS_EAST_OR_WEST)? QuadTreeTile::IS_EAST : QuadTreeTile::IS_SOUTH;
            if (outside[i][sibDir])
            {
                neighbor = parent->children[i];
                if (neighbor) //we may not have a child in that direction
                    neighbor = neighbor->siblings[sibDir].lock();
                if (neighbor) //that child may not have a sibling, but if it does, we'll use it.
                    neighbor = neighbor->parent.lock();
            }
            else
                neighbor = parent->children[i ^ childDir];

            if (neighbor)
                parent->setSibling((QuadTreeTile::SiblingDirection)sibDir, neighbor);
        }

    }
    currentL++;
}

/*--------------------------------------------------------------------------*/
// Creates a string by substituting the passed values for the substrings
// "#x", "#y", "#i", respectively.
std::string QuadTreeCollection::patternSubstitution(std::string pattern)
{
    size_t start, spot, len;
    std::string retval;

    len = pattern.size();
    start = 0;
    while (start < len)
    {
        spot = pattern.find('#', start);
        if (spot == std::string::npos)
            break;
        //found a '#' at the spot
        //First, copy what's between the start and the spot
        retval.append(pattern.substr(start, spot - start));

        //now see what to substitute--make spot the spot past the # now
        switch (pattern[++spot])
        {
            case '#':
                retval += "#";
                spot++; //advance spot past the '#'
                break;
            case 'i':
                retval += boost::lexical_cast<std::string>(currentI);
                spot++; //advance spot past the 'i'
                break;
            case 'l':
                retval += boost::lexical_cast<std::string>(currentL);
                spot++; //advance spot past the 'x'
                break;
            case 'w':
                retval += boost::lexical_cast<std::string>(1 << currentL);
                spot++; //advance spot past the 'x'
                break;
            case 'x':
                retval += boost::lexical_cast<std::string>(currentX);
                spot++; //advance spot past the 'x'
                break;
            case 'y':
                retval += boost::lexical_cast<std::string>(currentY);
                spot++; //advance spot past the 'y'
                break;
            default:
                //just allow the # to be the start of a copy for the rest.
                retval += "#";
                break;
        }
        start = spot;
    }
    retval.append(pattern.substr(start));

    return retval;
}

/*--------------------------------------------------------------------------*/
// Takes care of incrementing the 2D index of the leaf that will be loaded
// next.  Best done in a separate method because it could be in either
// rowMajorOrder or column major order.  The handy thing about this method
// is that you can deal with things in row-major order, and it will
// automatically put the indices in column-major order if rowMajorOrder is
// false.
void QuadTreeCollection::bumpIndex(int &x, int &y)
{
    if (rowMajorOrder)
    {
        x++;
        if (indexCols > 0 && x >= indexCols)
        {
            y++;
            x = 0;
        }
    }
    else
    {
        y++;
        if (indexRows > 0 && y >= indexRows)
        {
            x++;
            y = 0;
        }
    }
}

/*--------------------------------------------------------------------------*/
std::weak_ptr<QuadTreeTile> QuadTreeCollection::getTileAt(double x, double y, int level)
{
    if (!quadTreeRoot)
        return quadTreeRoot;

    return quadTreeRoot->getTileAt(x, y, level);
}

/*--------------------------------------------------------------------------*/
BBox QuadTreeCollection::GetBoundingBox()
{
    //&&&Should this be restricted to leaves, or might it mean different things
    //to different objects?
    return ObjectCollection::GetBoundingBox();
}

/*--------------------------------------------------------------------------*/
void QuadTreeCollection::SaveXML(FILE *out, std::string const &path)
{
    //&&&If we implement this, it needs a total rewrite
    /*
	if(out)
	{
		if(parent)
		{
			fprintf(out,"<Collection ");
			SaveXMLattributes(out);
			fprintf(out,">\n");
		}
		SaveXMLChildren(out,path);
		if(parent)
		{
			fprintf(out,"</Collection>\n");
		}
	}
    */
}

/*--------------------------------------------------------------------------*/
void QuadTreeCollection::SaveXMLChildren(FILE *out, std::string const &path)
{
    //&&&If we implement this, it needs a total rewrite
    /*
	Node * cursor = nodeList;
	if(out)
	{
		while(cursor)
		{
			cursor->object->SaveXML(out,path);
			cursor=cursor->next;
		}
	}
    */
}

/*--------------------------------------------------------------------------*/
int QuadTreeCollection::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    if (argc >= 1)
    {
        int ret;
        std::string command(argv[0]);
        if (command == "fuse")
        {
            Addressable *address = NULL;
            Object *ob;
            ret = gzCommandInterface::findObject(argv[1], address, ob);
            if (ret != TCL_OK)
                return ret;
            Fuse(ob);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
    }
    return ObjectCollection::perform(argc,argv,interp);
}

/*--------------------------------------------------------------------------*/
int QuadTreeCollection::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "fuse ", NULL);
    return ObjectCollection::list(interp);
}

/*--------------------------------------------------------------------------*/
void QuadTreeCollection::Fuse(Object *other)
{
    QuadTreeCollection *otherQTC;
    float compromiseScale;
    int myDepth, otherDepth, i;
    double mySize, otherSize;
    double minX, maxX, minY, maxY, currentX, currentY;
    BBox bbox, obbox;
    std::shared_ptr<QuadTreeTile> myTile, oTile;

    //is it another QuadTreeCollection?
    DYNAMIC_CAST(otherQTC, QuadTreeCollection *, other);
    if (!otherQTC) //if not, then just use the default...
    {
        ObjectCollection::Fuse(other);
        return;
    }

    compromiseScale = sqrt(otherQTC->minScale * maxScale); //geometric mean

    //determine closest level where otherSize <= mySize
    myDepth = getLeafLevel();
    otherDepth = otherQTC->getLeafLevel();
    mySize = getTileSize(myDepth, MaxXY);
    otherSize = otherQTC->getTileSize(otherDepth, MaxXY);
    while (otherSize <= mySize)
        otherSize = otherQTC->getTileSize(--otherDepth);
    otherSize = otherQTC->getTileSize(++otherDepth);

    //find the spatial bounds of overlap
    bbox = GetBoundingBox();
    obbox = otherQTC->GetBoundingBox();
    minX = (bbox.w > obbox.w)? bbox.w : obbox.w;
    maxX = (bbox.e < obbox.e)? bbox.e : obbox.e;
    minY = (bbox.s > obbox.s)? bbox.s : obbox.s;
    maxY = (bbox.n < obbox.n)? bbox.n : obbox.n;

    if (minX >= maxX || minY >= maxY)
        return; //no overlap; no need to fuse

    //set each other's tiles that overlap with mine so their
    //minScale is the compromise, and set each of mine that
    //overlap so their maxScale is the compromise.
    while (myDepth >= 0 && otherDepth >= 0)
    {
//        myTile = getTileAt(minX, minY, myDepth).lock();
//        oTile = getTileAt(minX, minY, otherDepth).lock();
        mySize = getTileSize(myDepth, MaxXY);
        otherSize = otherQTC->getTileSize(otherDepth, MaxXY);
        //work out where the overlap is
        for (currentY = maxY; currentY > minY - otherSize; currentY -= otherSize)
        {
            for (currentX = minX; currentX < maxX + otherSize; currentX += otherSize)
            {
                myTile = getTileAt(currentX, currentY, myDepth).lock();
                oTile = otherQTC->getTileAt(currentX, currentY, otherDepth).lock();
                if (!myTile)
                    continue; //edge conditions may mean certain levels don't cover here.
                myTile->maxScale = compromiseScale;
                if (oTile)
                    oTile->minScale = compromiseScale;
                else
                    myTile->fusionEdgeTile = true;
            }
        }
        //work out where the edges to the overlap are
        for (i = 0, currentY = maxY + 1.0; i < 2; i++, currentY = minY - 1.0)
        {
            for (currentX = minX; currentX < maxX + otherSize; currentX += otherSize)
            {
                myTile = getTileAt(currentX, currentY, myDepth).lock();
                oTile = otherQTC->getTileAt(currentX, currentY, otherDepth).lock();
                if (myTile)
                {
                    myTile->fusionEdgeTile = true;
                    myTile->maxScale = compromiseScale;
                }
                if (oTile)
                {
                    oTile->fusionEdgeTile = true;
                    oTile->minScale = compromiseScale;
                }
            }
        }
        for (i = 0, currentX = minX - 1.0; i < 2; i++, currentX = maxX + 1.0)
        {
            for (currentY = maxY; currentY > minY - otherSize; currentY -= otherSize)
            {
                myTile = getTileAt(currentX, currentY, myDepth).lock();
                oTile = otherQTC->getTileAt(currentX, currentY, otherDepth).lock();
                if (myTile)
                {
                    myTile->fusionEdgeTile = true;
                    myTile->maxScale = compromiseScale;
                }
                if (oTile)
                {
                    oTile->fusionEdgeTile = true;
                    oTile->minScale = compromiseScale;
                }
            }
        }
        myDepth--; otherDepth--;
        if (otherDepth < 0)
            otherDepth = 0;
    }
}
