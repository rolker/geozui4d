/* Copyright (c) 2002 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
// QuadTreeCollection.h: Class that contains QuadTreeObjects only.
//
//////////////////////////////////////////////////////////////////////

#ifndef QUADTREECOLLECTION_H
#define QUADTREECOLLECTION_H

class QuadTreeCollection;
#include "engine/ObjectCollection.h"
#include <memory>
//#include <gz4d/util/Xml.h>

class QuadTreeObject;
class QuadTreeTile;

class QuadTreeCollection : public ObjectCollection
{
public:

    DERIVE_OBJECT(QuadTreeCollection);

    QuadTreeCollection(GLboolean allocatePosable = GL_TRUE, int pickingTypes = PICK_CLICK1 | PICK_PRESS3);

    virtual void DrawLowRes(float degradation, Window3D *win);
    virtual int DrawHighRes(int eye, Window3D *win); //mdp 3/7/2000 -- return 1 if done, 0 if not done.

    virtual void Fuse(Object *other);

    virtual int RemoveObject(Object* object);
    virtual int ReloadObject(Object* object);
    virtual Object * LoadObject(const std::string& fn);
    //virtual void WriteToScene(std::string scenepath);
  
    virtual Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    virtual bool LoadXMLTiles(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile, bool alreadyPositioned = false);
    virtual void SaveXML(FILE *out, std::string const &path);
    virtual void SaveXMLChildren(FILE *out, std::string const &path);
    
    //Tile info
    enum {RootLevel = 0, LeafLevel = -1};
    enum {MinXY, MaxXY, AvgXY};
    virtual int getLeafLevel() {return leafDepth;}
    virtual double getTileSize(int atLevel = LeafLevel, int minMaxAvg = MinXY);
    virtual double getCellsInTile(int minMaxAvg = MinXY);
    std::weak_ptr<QuadTreeTile> getTileAt(double x, double y, int level);

    //Tellable interface
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);
    
    //Bounding box stuff
    virtual BBox GetBoundingBox();

    virtual double GetHeightAt(double x, double y, Point3d coord = NULL, GLboolean secondary = GL_FALSE);
    virtual double GetHeightNear(double x, double y, float rangeMeters = 1.0, int heightType = height_max);

protected:
    std::shared_ptr<QuadTreeObject> templateObject;
    std::shared_ptr<QuadTreeTile> quadTreeRoot;

    //Load-time variables and methods
    float leafStepX, leafStepY; //What the leaf-level stepsize is
    int firstX, endX, stepX; //file-pattern specifications in X
    int firstY, endY, stepY; //file-pattern specifications in Y
    int indexRows, indexCols; //file-pattern specifications to calculate index
    int actualRows, actualCols; //actual number of rows/cols on bottom level
    bool rowMajorOrder; //file-pattern specification to calculate index
    std::string filePattern; //Pattern of files to load/field with such pattern
    std::string labelPattern; //Pattern of name to give items
    std::string objectType; //What type of object to create in lieu of loading
    bool createObjects;
    int leafDepth; //how far the deepest leaf is from the root (which is at depth 0)
    double leafPixelSize; //how big is a "pixel" for the deepest child?
    double cellCountX, cellCountY; //How many units wide is the child?
    float minScale, maxScale; //Whats the optimal viewing range in terms of scale?

    void bumpIndex(int &x, int &y);
    int currentX, currentY, currentI, currentL;
    bfs::path currentPath;
    std::string patternSubstitution(std::string pattern);
    QuadTreeObject *createPatternedObject(bpt::ptree const &elem, float xOffset, float yOffset,
                                      bool potentialEdge = true, float scaleFactor = 1.0,
                                      bool actAsRoot = false);
    void createHierarchyUnder(
        std::shared_ptr<QuadTreeTile> &parent,
        std::vector<std::vector<std::shared_ptr<QuadTreeTile> > > tiles,
        bpt::ptree const &layerElement,
        int startX = 0, int startY = 0, int halfWidth = -1
        );

    double calcValue(double x, double y, int minMaxAvg);

};

#endif
