// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/**
*/
class Clut;

#ifndef CLUT_H
#define CLUT_H

#include "engine/TextureMaps/TextureMap.h"
#include "engine/Interactor.h"

struct clutnode
{
  clutnode * next;
  clutnode * previous;
  GLdouble height;
  Point4ub color;
};

class Clut: public TextureMap
{
public:
  Clut();
  virtual ~Clut();

  void Init();			// load default or named file
  int Load();
  int Save(std::string const &fn = std::string());

  bool Enable(const Point3d offset);
  bool Enable();
  void Disable();

  void Build();
  void refreshTexture();
  void Destroy();

  void CreateDefault();

  void Print() const;
  void Draw(bool asQuad = false);

  void Display(Tcl_Interp *interp, float min , float max, char * target);

  void SetMinMax();
  float GetMaxheight();
  float GetMinheight();
  float GetHeightRange();
  void ModifyExtents(float scale, float shift);
  void CreateRange(float max, float min, float stepsize);
  void Clear();
  void ClearRange(float max, float min);

  void AddNode(clutnode * newnode);
  void ReAddNode(clutnode * node);
  clutnode * Delete(clutnode * element);

  clutnode * GetNodePtr(float i);
  clutnode * GetNodePtrAtIndex(int i = -1); //added by mdp 5/30/2008 for GeoCoastPilot
  int GetIndexOf(clutnode *); //added by mdp 5/30/2008 for GeoCoastPilot
  unsigned char GetComponent(float i, int c);
  unsigned char GetInterpolatedComponent(float i, int c);

  //const Point4ub *GetColorAtValue(float val);
  boost::optional<gz4d::ImageRGBA8::pixel_type> GetColorAtValue(float val);
  int GetNumEntries();
  const Point4ub *GetColorAtIndex(int idx);
  static unsigned int clutChangedEvent() { return clutChangedInputType->value;}
  
  void InitDims(){};
  /** No descriptions */
  void SetHeight(double d);

  virtual void beginTextureOffset(float x, float y, float z, float scale = 1.0);

private:
  clutnode * clut; 		// pointer to the internal clut data structure
  float minheight, maxheight, range; // range between min and max entries
  double scl, trans;		// scale and translation factors
  GLboolean boundsChanged;

  GLboolean useSurfaceColor;  //should surfaceColor be used?
  GLfloat surfaceColor[4]; //change the glColor

  // interaction
  static const EventType *clutChangedInputType;

  void StripNewLine(char * line);
};

#endif
