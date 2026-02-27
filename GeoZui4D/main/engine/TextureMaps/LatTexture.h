/* Copyright (c) 2000 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef LATTEXTURE_H
#define LATTEXTURE_H

#include "TextureMap.h"

#define MAXLATTICES 	10

// 
// Texture particles are distributed on the surface of the image
// according to horizontal and vertical distribution functions.
//
// Each particle is drawn as an oriented line segment, a dot, or
// a squiggle
//
// distribution function: gaussian: mean, sd; uniform: center, extent
//
// line: rotation, length, width, color
// dot: radius, color
// gabor: rotation, size, contrast, decay rate
//


class LatTexture: public TextureMap
{
public:
  LatTexture();
  virtual ~LatTexture();

  void Init();			// load default or named file
  int Load();
  int Save(std::string const &fn = "");  // save texture to file

  bool Enable(const Point3d offset);	// turn this texture on
  bool Enable();	// turn this texture on
  void Disable();		// turn this texture off

  void Build();			// build the OpenGL texture texture
  void refreshTexture();
  void Destroy();		// delete the internal texture data

  void CreateDefault();		// make the default texture texture

  void Print() const;
  void Draw(bool asQuad = false);
  void Display(Tcl_Interp *interp, float min , float max, char * target);
  void InitDims(){};

private:
	enum FilterType{Nofilter, Gaussian, Box};
	enum Jitter{Constant, Normal, Uniform};
	enum Style{Line, Dot, Gabor};
	
	struct LatticeElement
	{
	int nrows;			// rows and cols in the lattice
	int ncols;
	Point4ub color;		// background color behind lattice
	
	float probability;		// probablility of drawing each lattice node
	
	int hjitter;			// horizontal jitter params
	float hoffset, hrange;
	int vjitter;			// vertical jitter params
	float voffset, vrange;
	int rjitter;			// rotational jitter params
	float roffset, rrange;
	
	int style;			// lattice node drawing style
	float angle;			// angle at which node is drawn
	float length;			// length of node icon
	float thickness;		// width of node icon
	
	LatticeElement();
	
	void Print() const;
	};
  // color of texture background, if alpha < 1, texture holes will be transparent
  Point4ub backcolor;		// color of texture background

  int filtertype;		// how to filter image after creation
  int filterwidth;

  // lattice parameters
  int nlattices;
  LatticeElement *lattice[MAXLATTICES];

  void BuildImage(int w, int h);// create the texture image
  void BuildMipMaps();		// build the mipmaps to be used for texture

  // jittering and filtering methods
  double jitter(int type, double offset, double range);
  void filter();

  // drawing methods
  void drawline(float x, float y, float rjitter, LatticeElement *e);
  void drawdot(float x, float y, LatticeElement *e);
  void drawgabor(float x, float y, float rjitter, LatticeElement *e);
};

#endif
