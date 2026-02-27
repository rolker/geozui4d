/* Copyright (c) 2000 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef GRID_H
#define GRID_H

#include "TextureMap.h"

#define MAXSUBGRIDS 	10

struct GridElement
{
  int hdivs;
  int vdivs;
  int linewidth;
  Point4ub linecolor;

  GridElement();

  void Print() const;
};

class Grid: public TextureMap
{
public:
  Grid();
  virtual ~Grid();

  void Init();			// load default or named file
  int Load();
  int Save(std::string const &fn = "");

  bool Enable(const Point3d offset);	// turn this grid on
  bool Enable();	// turn this grid on
  void Disable();		// turn this grid off

  void Build();			// build the OpenGL grid texture
  void Destroy();		// delete the internal grid data

  void CreateDefault();		// make the default grid texture

  void Print() const;
  void Draw(bool asQuad = false);
  void Display(Tcl_Interp *interp, float min , float max, char * target);
  void InitDims(){};

private:
  // color of grid background, if alpha < 1, grid holes will be transparent
  Point4ub backcolor;		// color of grid background

  // main grid parameters
  GridElement maingrid;

  // subgrid parameters
  int nsubgrids;
  GridElement *subgrid[MAXSUBGRIDS];
  
  void clear(const Point4ub &clearcolor);
  void drawrow(float y, int linewidth, const Point4ub &color);
  void drawcol(float x, int linewidth, const Point4ub &color);

  void BuildImage(int w, int h);// create the grid texture map image
  void BuildMipMaps();		// build the mipmaps to be used for texture
};

#endif
