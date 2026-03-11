// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef IMAGEMAP_H
#define IMAGEMAP_H

#include <string.h>

#include "TextureMap.h"
#include "widgets/Sv_Raster.h"
#include "utilities/Cacheable.h"
#include <memory>
#include <gz4d/Image.h>
#include <gz4d/netcdf.h>

class ImageMap: public TextureMap, public Cacheable
{
public:
  ImageMap();
  virtual ~ImageMap();

  void Init();          // load deflt or named file
  int Load();
  int Save(std::string const &fn = "");

  bool Enable(const Point3d offset);    // turn texture map on
  bool Enable();    // turn texture map on
  void Disable();       // turn this texture map off
  void Bind();

  void SetRepeat() {repeat = GL_TRUE; noBorder = GL_TRUE;};
  void SetNoBorder() {noBorder = GL_TRUE;};

  void Build();         // build the OpenGL texture
  void Destroy();       // delete the internal data

  void CreateDefault();     // make the default texture

  void Print() const;
  void Draw(bool asQuad = false);
  void Display(Tcl_Interp *interp, float min , float max, char * target);
  int TextureIdle(double time);
  virtual GLboolean IsPlaying();

  void Fuse(Gutm * gutm, float transparency = 0.0);

  float GetTexT(float t);
  void InitDims();
  void SetRef(double e,double n,float es,float ns);
  void UseMipmaps(bool useThem = true) {usemipmaps = (int)useThem;}

  virtual void CombineTextures(ImageMap *textures[], int texturesWide = 2,
                               int texturesHigh = 2,
                               float xScaleFactor = 0.5,
                               float yScaleFactor = 0.0); //added by mdp 12/26/2008

  //Cacheable methods
  virtual float load(float min = 1.0, float max = 1.0, int policy = 0);
  virtual float unload(float min = 1.0, int policy = 0);
  virtual float getCurrentSize();
  virtual float getFullSize();
  virtual double *getMeasures(int policy = 0);

  void allowCacheing(bool flag = true) {canBeCached = flag;}
  void boostCachePriority(int nudges) {cacheDistanceBoost = nudges * 0.0001;}

protected:
    /// Quickly find the index for a given time.
    /// If we have a time series of images, this will try to
    /// find the index corresponding to the time step at the 
    /// given time. This is done by assuming that the times 
    /// are evenly spaced in timeIndes and guessing the probable
    /// index that would return the queried time. If the guess is
    /// wrong, simple move up or down the array, one element at a
    /// time until the correct index is found;
    unsigned long findTimeIndex(double t);
    
    
    void makemipmap();
    void maketexture();
    void checkMake();
    void deleteImage();
    bool imageIsReady(bool tryToBuild = true);

//  int mipw, miph;     // mipmap base width, height (powers of 2)
  int miplevels;        // number of levels needed in mipmap
  Point4ub *mipbase;        // image forming mipmap base
  int repeat;           //should the image repeat? (mdp 9/27/2001)
  int noBorder;         //should the image have no border? (mdp 3/8/2005)
  bool flipYcoord;      //should the Y texture coordinates be inverted?

  std::shared_ptr<gz4d::ImageRGBA8> gzImage;
  std::shared_ptr<gz4d::ImageRGBA5_5_5_1> gzSmallImage;

  float eastref, northref;  // georeference of image upper lefthand corner
  float eastscale, northscale;

  bool firstmake; // flag used to determine the first call to maketexture
  int usemipmaps;
  bool needsMake;
  bool forceLoad;
  bool enabled;
  bool haveMetaInfo;
  
  gz4d::netcdf::File ncfile;
  gz4d::netcdf::Var ncimage;

  std::vector<double> timeIndex;
  //unsigned long timeSteps;
  unsigned long currentTimeStep;

  enum {cacheLastDrawn, cacheNumDistances};
  double cacheDistances[cacheNumDistances];
  bool canBeCached;
  double cacheDistanceBoost;

private:
    //helper function for CombineTextures()
    void determineContributions(int tv, float scaleFactor,
                                        float &pixel, float &bucketAmount,
                                        float &overflow);
//     void addToPixel(float bucket[4], GLubyte *sourcePixel,
//                     float factor);
    void addToPixel(float bucket[4], gz4d::Pixel<gz4d::RGBA8> const &sourcePixel,
                    float factor) const;
    void copyToPixel(GLubyte *pixel, float sourcePixel[4]);
    void copyToPixel(float pixel[4], float sourcePixel[4]);
};

#endif
