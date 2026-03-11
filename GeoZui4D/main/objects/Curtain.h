// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/////////////////////////////////////////////////////////////////////
//
// Curtain.h: interface for the Curtain display class
//
/////////////////////////////////////////////////////////////////////

#ifndef CURTAIN_H
#define CURTAIN_H

#include "engine/Object.h"

// Set depending on the color mapping type in the Curtain definition
// file (.ctn). CLUT = INDEX, RGB = COLOR3, RGBA = COLOR4
enum CurtainDataType {INDEX, COLOR3, COLOR4};

//
// Data structure to hold the data for a single column of the curtain
//
struct ColumnData
{
  Point3f normal;	// unit surface normal of the column

  int nSteps;		// the number of steps (data values) in the column

  // the column data itself
  unsigned char *data;	// used only in INDEX (CLUT) mode
  Point4ub *colors;	// used only in COLOR3 (RGB) or COLOR4 (RGBA) modes

  ColumnData(int nvals = 0);
  ~ColumnData();

  void SetSize(int nvals, int datatype);

  void Print();
};

//
// Data structure for a texture map that is assigned to a panel.
//
struct PanelTexture
{
  GLuint textureID;		// the OpenGL texture ID for this texture

  int width, height;		// the texture image width and height
  Point4ub *image;		// the image being used for this texture

  PanelTexture(int w = 0, int h = 0);
  ~PanelTexture();

  void SetSize(int w, int h);

  void Print();
};

//
// A panel is a piece of the polygonal surface which forms the curtain
// geometry. It is associated with one texture map. The entire curtain
// may consist of many panels, each with its own texture map.  It
// consists of a number of rectangular elements (or segments) each
// positioned with a column as its vertical central axis. Each segment
// can be thought of as a column of grid cells, centered about a data
// sample.
//
struct Panel
{
  int startColumn;	// starting data column number for a segment
  int nSegs;		// number of vertical rectangular segments in the panel
  int maxHeight;	// max number of data samples in a panel column

  Point3f *segments;	// array of surface vertices that form the segments

  PanelTexture *texture;	// texture map for the panel

  Panel(int n = 0);
  ~Panel();

  void SetSize(int n);

  void Print();
};

//
// Internal data structure to hold the color lookup table being used
// to color the curtain
//
struct ColorTable
{
  int nColors;			// the number of colors in the table
  Point4ub *colors;		// the table itself

  ColorTable(int nentries = 0);
  ~ColorTable();

  void SetSize(int nentries);

  void Print();
};

class Curtain : public Object
{
 public:
 
  Curtain();
  ~Curtain();
  
  DERIVE_OBJECT(Curtain);
  
  // File related
  virtual int readFrom(FILE *file);
  virtual int processField(int option, char **Field);
  virtual int writeTo(FILE *file);

  // print self
  void Print();

  // Rendering
  virtual void DrawLowRes(float degradation, Window3D *win);
  virtual int DrawHighRes(int eye, Window3D *win);
  virtual void ResetHighRes();

  virtual void useNewHeightExaggeration(GLfloat heightEx);

  // Querying
  virtual double GetHeightAt(double x, double y, 
				   Point3d coord = NULL, GLboolean secondary = GL_FALSE);

  virtual void UpdateTexture(TextureMap *ptr);

 protected:
  int dataType;		// curtain data may be either INDEX, COLOR3, or COLOR4
  int useLighting;	// true if using lighting and shading for display
  int lastStart;	// last panel we started on (for when we're not showing every panel in every frame)

  int xyonlyformat;	// true if XY_ONLY is specified.

  double zStep; 	// the vertical distance between intervals
  double zHeight;	// vertical height of curtain at the longest column

  int nColumns;  	// the number of columns of data in the curtain
  ColumnData *curtain;	// the data for the curtain
  std::vector<gz4d::Point<double> > location;	// the positions for the tops of each column

  int nPanels;		// the number of panels forming the curtain geometry
  Panel *panels;	// the panels of the geometry for the curtain

  double pathLength;	// total length of path of curtain in horizontal plane
  double hStep;		// average horizontal distance between columns

  int minSamples;	// bounds on number of samples per data column across
  int maxSamples;	//   the entire curtain

  ColorTable clut;	// the color table to be used by the curtain

  void CalculateNormals();	// calculate the per column surface normals

  // each panel consistes of a number of segments (each centered on a column)
  // this builds the segments for a particular panel from the data columns
  void BuildSegments(Panel &panel, int startcolumn);

  void BuildPanelSurfaces();		// construct the panels
  void BuildPanelTexture(Panel &panel);	// texture the panels

  void DrawPanel(const Panel &panel, int skipcount = 1) const;

  enum CurtainParams {CLUT = FirstObjectKeyword, RGB, RGBA, 
					  USE_LIGHTING, XY_ONLY, NCOLS, Z_STEP, NTH};

};

#endif 
