/* Copyright (c) 2000 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef TEXTUREMAP_H
#define TEXTUREMAP_H

#include "SDV_types.h"
#include "engine/Persistence.h"
#include <gz4d/Image.h>

class Gutm;

// used as starting default texture size when needed by a texture subclass
// currently only used by Clut
#define TEXTUREMAP_TEXTURE_SIZE 2048   // must be a power of 2

// values returned by the Type() method. If new subclasses of TextureMap
// are added, a corresponding entry to this emum must be made, and all
// constructors of the new type are responsible to set type to this value
enum TextureTypes{CLUTTEXTURE, GRIDTEXTURE, IMAGETEXTURE, LATTICETEXTURE, MOVIETEXTURE,RTVIDEOTEXTURE};

class TextureMap:virtual public Persistence
{
public:
  TextureMap();
  virtual ~TextureMap();

  virtual int Load() = 0;       // load texture from file
  virtual void Init() = 0;      // load deflt or named file
  int Reload();             // destroy old and load
  virtual int Save(std::string const &fn = "") = 0;  // save texture to file
  virtual void SaveXML(FILE * out, std::string path);

  virtual bool Enable(const Point3d offset) = 0; // turn texture on
  virtual bool Enable() = 0; // turn texture on, but without auto tex coord gen
  virtual void EnsureBuilt(); //build if hasn't happened yet
  
  virtual void Disable() = 0;       // turn this texture map off

  virtual void Build() = 0;     // build the OpenGL texture
  virtual void Destroy() = 0;       // delete data & OpenGL texture

  virtual void CreateDefault() = 0; // make a default texture

  virtual void Print() const = 0;   // text display of texture parameters
  virtual void Draw(bool asQuad = false) = 0;   // draw texture to OpenGL window
  virtual void Display(Tcl_Interp *interp, float min , float max, 
               char * target) = 0; // Tcl draw of texture for GUI
  
  virtual int TextureIdle(double time){return 0;};

  virtual void Fuse(Gutm* gutm, float transparency = 0.0){};

  virtual GLboolean IsPlaying() {return GL_FALSE;}
  virtual void Play(){};
  virtual void Stop(){};
  virtual void Pause(){};
  virtual void Loop(GLboolean loop = GL_TRUE) {};

  static void setTransparentBlendThreshold(float newValue);

  int Type();       // returns the type of this texture

  virtual void InitDims() = 0;
  virtual void SetDims(int width, int height);
  int GetWidth() {return iwidth;};
  int GetHeight() {return iheight;};
  int isLoaded() {return loaded;};
  
  //float GetTexS(float s) {return s*widthRatio;};
  virtual float GetTexT(float t);
  virtual void SetHeight(double d){};

  virtual void beginTextureOffset(float x, float y, float z, float scale = 1.0);
  void endTextureOffset();
    int IsBuilt(){return built;};
    void setApplyMode(GLint mode){applyMode = mode;};

  enum ColorFormat {RGBA16, RGBA8, RGBA4, RGB5, RGB_byte};
  void setColorFormat(ColorFormat newFormat = RGBA8);
  ColorFormat getColorFormat() {return tColorFormat;}

protected:
  int type;     ///< the type of texture so outsiders can test type
  int built;        ///< true only if OpenGL texture already built
  GLuint textureID; ///< OpenGL texture ID
  int loaded;
  GLuint applyMode;

  //Point4ub * image;     ///< texture image pixmap
  gz4d::ImageRGBA8 image;
  

  GLint twidth, theight;    ///< dimensions of the texture in pixels
  float hsize, vsize;   ///< dimensions of the texture in world coordinates
  float hOffsetScale, vOffsetScale; ///<factors for beginTextureOffset (provided by each implementation);
  
  int iwidth, iheight;  ///< dimensions of the image going into the texture
  
  float widthRatio, heightRatio;    ///< Convert image coordinate to texture coordinate

  static float transparentBlendThreshold;

  unsigned long colorFormat; //how the texture should store the colors
  ColorFormat tColorFormat;
};

std::shared_ptr<TextureMap> CreateTexture(std::string const &fname);

#endif
