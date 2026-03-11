// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// gutm.h: interface for the gridded utm class.
//
//////////////////////////////////////////////////////////////////////

#ifndef GUTM_H
#define GUTM_H


#include "engine/Object.h"
//#include <GL/glut.h>
#include "objects/nodeTypes/SDV_DataHandler.h"


#define HYBRID_HIGHRES 2

class ViewManager;
class DynamicTides;
class RubberBandObject;

class Gutm : public Object
{
  friend class ConversionMethod;
public:
  virtual int Load(); //redefined to divert Bag processing (mdp 11/4/2005)
  int readFrom(FILE *file);
  int writeTo(FILE *file);
  virtual int processField(int option, char **Field);
  void DrawLowRes(float degradation, Window3D *win);
  int DrawHighRes(int eye, Window3D *win);
  void Fuse(Object *other);
  void RestoreFusionHoles();
  void ResetHighRes();
  Gutm *createCopy(int startx = 0, int starty = 0, int endx = -1, int endy = -1);

  GLboolean LoadBag(const char *path = NULL);
  
    virtual void Idle(double time);

  void CalculateNormals(float verticalScale, int progressStart = 80);
  virtual void useNewHeightExaggeration(GLfloat hieghtEx);

  //Preferred viewing parameters--&&& this is a quick hack until there's enough demand
  //to implement preferred components in the posables themselves.
  virtual gz4d::Point<double> getPreferredOffset();

  virtual void ShowInfo(GLuint *names = NULL, int numNames = 0);
  virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
  virtual int list(Tcl_Interp *interp);

    SDV_DataHandler * GetHandler(int x, int y);
    void SetHandler(SDV_DataHandler * h,int x, int y) {rtdata[y][x] = h;};

  double GetHeightAt(double x, double y, Point3d coord = NULL, GLboolean secondary = GL_FALSE);
  double GetHeightNear(double x, double y, float rangeMeters = 1.0, int heightType = height_max);
  Gutm(int rt = 0);
  ~Gutm();

  void Initialize(int r, int c, double csize, bool altData = false);
  
  DERIVE_OBJECT(Gutm);
  double GetValue(double x,double y);
  float GetValue(int x, int y);

  GLshort GetNormX(int x, int y);
  GLshort GetNormY(int x, int y);
  GLshort GetNormZ(int x, int y);
  
  void SetValue(int x, int y, double z, bool allowZero = false);
  int HasValue(double x,double y);
    int HasValueInRange(double x1,double y1, double x2, double y2);
    void ApplyNormal(int x, int y,float he);
    
    void SetAltValue(int x, int y, float v);

    void SetNorth(Gutm * g){north = g;};
    void SetEast(Gutm * g){east = g;};
    Gutm * GetNorth(){return north;};
    Gutm * GetEast(){return east;};

    int loadFromNetCDF(gz4d::netcdf::File ncfile,std::string var,std::string normsvar,int timestep=-1);
    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);

  static int useHighResZone;
  static GLboolean normalsOnTheFly;

    void print();

    void setRubberband(RubberBandObject *r);
    GLboolean getRubberbandIndexes(int &startx, int &starty, int &endx, int &endy);
    void rubberbandCopy();
    void rubberbandHole();
    void rubberbandRestore();

  static GLfloat surfaceColor[4];

protected:

  void DrawShadedStrip(int row, int start, int end, int step);
  void DrawEverything(int eye, Window3D *win);
  void DrawUsingHighResZone(float degradation, Window3D *win);
  void renderingBounds(float degradation);
  GLboolean generateBoundingLines(int boundsType);
  void calcBoundingLine(int startIdx, int stopIdx, float &invSlope, float &invIntercept);
  void renderWithinBounds(int boundsType, int step, int earliestY = 0, int latestY = -1);
  int AllocateGrid(bool zero = true);
  virtual void initializeAs(const char* alias);

  int drawrow;
  int shape;
  bool hasAlternateData;
  int binary;

  float shadeExaggeration; // applies height exaggeration to the normals, without needing to exagerate the scene

    GLfloat * dem_data; // array dor dem data
  GLfloat * * dem; // pointers to each row of the dem data
  GLfloat * * alternateData; // to store other info.
  GLfloat alternateDataOffset; // how much is the alternate data offset by?
  int *startline, *endline; // skip the start and end values

  // used for rt data input
  SDV_DataHandler * * * rtdata;
  int rtflag;
  
  // valid times for time varying cells
  double * startValidTime;
  double * endValidTime;
  
  // for tild gutms
  Gutm * east;
  Gutm * north;

  GLboolean isBag;

    void calculateCenter();
    gz4d::Point<double> centerOfGutm;

  int maxy; // number of rows
  int maxx; // number of columns
    
  float cellsize;

  float heightscale;
  GLboolean depthsAsFeet;

  int highResSize; // size of high-res window in low-res data

  unsigned short priority; // used for fusing, allows lower res gutm to drill in hier res

    vNorms * norms_data;
    vNorms * * norms_array;
    
  ViewManager *currentView;

  //of time and tides
  DynamicTides *dTides, *lastDynamicTides;
  float *tideTimeOffsets; //added by mdp 4/8/2005 to speed dynamic tide application
  float *tideMultipliers; //added by mdp 4/8/2005 to speed dynamic tide application
  void createTideOffsetTable();
  double *plannedPathTime; //added by mdp 4/11/2005 to speed application of planned path w/ tides
  NavigationPath *lastNavPath;
  int navPathUpdateCounter;
  void updatePlannedPathTimes();

  RubberBandObject *rubberBand;

  //---Bounds variables---
  enum {pBound, qBound, rBound, sBound};
  enum {OuterBounds, FrontBounds, BackBounds};
  Point2f allBounds[3][4], *bounds;
  int minyBound, maxyBound;
  float leftInvSlope, rightInvSlope, leftInvIntercept, rightInvIntercept;
  int leftBoundLowIndex, leftBoundHighIndex, leftDirection;
  int rightBoundLowIndex, rightBoundHighIndex, rightDirection;
  GLboolean dropNear;
  GLboolean drewBox;
  float frontLengthFactor; //how far out should high-res zone go relative to default

  enum GutmParams {COLS_ROWS = FirstObjectKeyword, CELLSIZE, SHAPE,
                   FULLGRID, PARTIAL, USE_ALTERNATE_DATA, OFFSET, BINARY, SHADE_EXAGGERATION, DEPTHS_AS_FEET};

};


#endif 
