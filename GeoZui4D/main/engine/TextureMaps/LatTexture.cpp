// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

//#define PRINTKERNEL
//#define PRINTSTATS

#include <iostream>
#include <math.h>
#include "utilities/random.h"
#include "LatTexture.h"
#include "utilities/gauss.h"
#include <filesystem>
#include <GL/glu.h>

#define LATMAGICNUMBER  "LATTICE"
#define LATVERSION  "1.0"

#define T_SIZE  33
void gabor(float size, float angle, Point4ub color, float ratio, 
       Point4ub image[T_SIZE][T_SIZE]);

#define sqr(x) ((x) * (x))

#define DEFAULTROWSCOLS     10
#define DEFAULTTHICKNESS    0.1f
#define DEFAULTRED      0.2
#define DEFAULTGREEN        0.2
#define DEFAULTBLUE     0.7
#define DEFAULTALPHA        0.7

using namespace std;


// definitions for parsing the grid description file
enum LatTextureParams{IMAGE, LATTICE,
              WIDTH, HEIGHT, HSIZE, VSIZE, COLOR, 
              FILTER, NOFILTER, GAUSS, BOX,
              ROWS, COLS, 
              PROBABILITY,
              HJITTER, VJITTER, RJITTER, 
              CONSTANT, NORMAL, UNIFORM,
              STYLE, LINE, DOT, GABOR};
const char *LatTextureNames[] = {"IMAGE", "LATTICE", 
               "WIDTH", "HEIGHT", "HSIZE", "VSIZE", "COLOR", 
               "FILTER", "NOFILTER", "GAUSS", "BOX",
               "ROWS", "COLS", 
               "PROBABILITY",
               "HJITTER", "VJITTER", "RJITTER",
               "CONSTANT", "NORMAL", "UNIFORM", 
               "STYLE", "LINE", "DOT", "GABOR", ""};

//
// Default lattice: 10 x 10 divisions, color = (0.2,0.2,0.7,0.7),
//   100% probability of being drawn
//   no randomization of position and 0 offset
//   lattice elements drawn as dots of radius 0.1 m
//
LatTexture::LatticeElement::LatticeElement()
{
  nrows = ncols = DEFAULTROWSCOLS;
  color[0] = int(DEFAULTRED * 255);
  color[1] = int(DEFAULTGREEN * 255);
  color[2] = int(DEFAULTBLUE * 255);
  color[3] = int(DEFAULTALPHA * 255);

  probability = 1.0;

  hjitter = Constant;
  hoffset = hrange = 0;
  vjitter = Constant;
  voffset = vrange = 0;
  rjitter = Constant;
  roffset = rrange = 0;

  style = Dot;
  angle = length = 0;
  thickness = DEFAULTTHICKNESS;
}

void LatTexture::LatticeElement::Print() const
{
  cout << "dimensions: [" << nrows << ", " << ncols << "] ";
  cout << "color: [" << int(color[0]) << ", " << int(color[1]) << ", "
       << int(color[2]) << ", " << int(color[3]) << "]";
  cout << "\n";

  cout << "probability: " << probability << "\n";

  cout << "horiz jitter - ";
  switch(hjitter){
  case Constant:
    cout << "constant: offset " << hoffset;
    break;
  case Normal:
    cout << "normal: mean " << hoffset << " std. dev. " << hrange;
    break;
  case Uniform:
    cout << "uniform: offset " << hoffset << " range " << hrange;
    break;
  }
  cout << "\n";

  cout << "vert jitter - ";
  switch(vjitter){
  case Constant:
    cout << "constant: offset " << voffset;
    break;
  case Normal:
    cout << "normal: mean " << voffset << " std. dev. " << vrange;
    break;
  case Uniform:
    cout << "uniform: offset " << voffset << " range " << vrange;
    break;
  }
  cout << "\n";

  cout << "rotational jitter - ";
  switch(rjitter){
  case Constant:
    cout << "constant: offset " << roffset;
    break;
  case Normal:
    cout << "normal: mean " << roffset << " std. dev. " << rrange;
    break;
  case Uniform:
    cout << "uniform: offset " << roffset << " range " << rrange;
    break;
  }
  cout << "\n";

  cout << "drawing style - ";
  switch(style){
  case Line:
    cout << "line: angle " << angle << " length " << length << 
      " thickness " << thickness;
    break;
  case Dot:
    cout << "dot: radius " << thickness;
    break;
  case Gabor:
    cout << "gabor: angle " << angle << " size " << length << 
      " ratio " << thickness;
  }
  cout << endl;
}

LatTexture::LatTexture()
{
  type = LATTICETEXTURE;

  // set up the reader to look for BEGIN LATTICE 1.0, and with set of keywords
  SetTypeAndVersion("LATTICE", 1, 0);
  SetKeywordTable(LatTextureNames);

  CreateDefault();
}

LatTexture::~LatTexture()
{
  Destroy();
}

void LatTexture::Init()
{
  Destroy();

  if(GetFilename().empty())
    {
      SetFilename("default.lat");
    }

  if(Load() != 0)
    {
      cerr << "Creating default lattice texture" << endl;
      CreateDefault();
    }
}

int LatTexture::Load()
{
  int item;
  int mode = IMAGE;
  char **fields = NULL;
  char *fname;
  int i;

  if(Open())
      return 1;

  Destroy();        // make sure that we build a new lattice texture map

  for(fields = NextLine(); fields != NULL; fields = NextLine())
    {
      for(i = 0, fname = fields[i++]; fname != NULL; fname = fields[i++])
    {
      item = Lookup(fname);
      // first error check the item to make sure in correct file segment
      switch(item)
        {
        case IMAGE:
          mode = IMAGE;
          break;
        case LATTICE:
          mode = LATTICE;
          if(nlattices == MAXLATTICES){
        cerr << "maximum of " << MAXLATTICES << 
          " lattices can be specified!" << endl;
        exit(1);
          }
          lattice[nlattices] = new LatticeElement;
          if(lattice[nlattices++] == NULL)
        {
          cerr << "Out of memory allocating space for lattice texture!" << endl;
          return 1; 
        }
          break;
        case WIDTH:
        case HEIGHT:
          if(mode != IMAGE){
        cerr << "width and height only specified for an image!" << endl;
        exit(1);
          }
          break;
        case HSIZE:
        case VSIZE:
          if(mode != IMAGE){
        cerr << "hsize and vsize only specified for an image!" << endl;
        exit(1);
          }
          break;
        case FILTER:
          if(mode != IMAGE){
        cerr << "filter only specified for an image!" << endl;
        exit(1);
          }
          mode = FILTER;
          break;      
        case NOFILTER:
        case GAUSS:
        case BOX:
          if(mode != FILTER){
        cerr << "nofilter gauss or box only specified for filter!" << endl;
        exit(1);
          }
          break;
        case HJITTER: 
        case VJITTER:
        case RJITTER:
        case STYLE:
          if(mode == IMAGE){
        cerr << "jitter and style only specified for a lattice!" << endl;
        exit(1);
          }
          mode = item;
          break;
        case ROWS: case COLS:  
        case PROBABILITY:
          if(mode == IMAGE){
        cerr << "rows, cols and probabilty only specified for lattice!" << endl;
        exit(1);
          }
          break;
        case CONSTANT: 
        case NORMAL: 
        case UNIFORM:
          if(mode != HJITTER && mode != VJITTER && mode != RJITTER){
        cerr << "constant, normal or uniform only specified for jitter!" << endl;
        exit(1);
          }
          break;
        case LINE: 
        case DOT: 
        case GABOR:
          if(mode != STYLE){
        cerr << "line, dot or gabor only specified for style!" << endl;
        exit(1);
          }
          break;
        case -1:
          cerr << "Invalid field " << fname << 
        " in texture description file!" << endl;
          return 1;
        }
      
      // next get the numeric data associated with the item
      switch(item)
        {
        case WIDTH:
          iwidth = ConvertInt(i);
          break;
        case HEIGHT:
          iheight = ConvertInt(i);
          break;
        case HSIZE:
          hsize = (float) ConvertInt(i);
          break;
        case VSIZE:
          vsize = (float) ConvertInt(i);
          break;
        case COLOR:
          if(mode == IMAGE)
        ConvertFloatColor(backcolor, i);
          else
        ConvertFloatColor(lattice[nlattices - 1]->color, i);
          break;
        case NOFILTER:
          filtertype = Nofilter;          
        case GAUSS:     
          filtertype = Gaussian;
          filterwidth = ConvertInt(i);
          break;
        case BOX:       
          filtertype = Box;
          filterwidth = ConvertInt(i);
          break;
        case ROWS:
          lattice[nlattices - 1]->nrows = ConvertInt(i);
          break;
        case COLS:
          lattice[nlattices - 1]->ncols = ConvertInt(i);
          break;
        case PROBABILITY:
          lattice[nlattices - 1]->probability = ConvertFloat(i);
          break;
        case CONSTANT: 
          switch(mode){
          case HJITTER:
        lattice[nlattices - 1]->hjitter = Constant;
        lattice[nlattices - 1]->hoffset = ConvertFloat(i);
        break;
          case VJITTER:
        lattice[nlattices - 1]->vjitter = Constant;
        lattice[nlattices - 1]->voffset = ConvertFloat(i);
            break;
          case RJITTER:
        lattice[nlattices - 1]->rjitter = Constant;
        lattice[nlattices - 1]->roffset = ConvertFloat(i);
            break;
          }
          break;
        case NORMAL: 
          switch(mode){
          case HJITTER:
        lattice[nlattices - 1]->hjitter = Normal;
        lattice[nlattices - 1]->hoffset = ConvertFloat(i);
        lattice[nlattices - 1]->hrange = ConvertFloat(i);
        break;
          case VJITTER:
        lattice[nlattices - 1]->vjitter = Normal;
        lattice[nlattices - 1]->voffset = ConvertFloat(i);
        lattice[nlattices - 1]->vrange = ConvertFloat(i);
        break;
          case RJITTER:
        lattice[nlattices - 1]->rjitter = Normal;
        lattice[nlattices - 1]->roffset = ConvertFloat(i);
        lattice[nlattices - 1]->rrange = ConvertFloat(i);
        break;
          }
          break;
        case UNIFORM:
          switch(mode){
          case HJITTER:
        lattice[nlattices - 1]->hjitter = Uniform;
        lattice[nlattices - 1]->hoffset = ConvertFloat(i);
        lattice[nlattices - 1]->hrange = ConvertFloat(i);
        break;
          case VJITTER:
        lattice[nlattices - 1]->vjitter = Uniform;
        lattice[nlattices - 1]->voffset = ConvertFloat(i);
        lattice[nlattices - 1]->vrange = ConvertFloat(i);
        break;
          case RJITTER:
        lattice[nlattices - 1]->rjitter = Uniform;
        lattice[nlattices - 1]->roffset = ConvertFloat(i);
        lattice[nlattices - 1]->rrange = ConvertFloat(i);
        break;
          }
          break;
        case LINE: 
          lattice[nlattices - 1]->style = Line;
          lattice[nlattices - 1]->angle = ConvertFloat(i);
          lattice[nlattices - 1]->length = ConvertFloat(i);
          lattice[nlattices - 1]->thickness = ConvertFloat(i);
          break;
        case DOT:
          lattice[nlattices - 1]->style = Dot;
          lattice[nlattices - 1]->thickness = ConvertFloat(i);
          break;
        case GABOR:
          lattice[nlattices - 1]->style = Gabor;
          lattice[nlattices - 1]->angle = ConvertFloat(i);
          lattice[nlattices - 1]->length = ConvertFloat(i);
          lattice[nlattices - 1]->thickness = ConvertFloat(i);
          break;
        }
    }
    }

    hOffsetScale = hsize;
    vOffsetScale = vsize;
  Close();
  return 0;
}

int LatTexture::Save(std::string const &fn)
{
  int i;

  if(OpenWrite(fn))
      return 1;

  fprintf(out, LATMAGICNUMBER);

  fprintf(out, "\n\nimage\n");
  fprintf(out, "width %d\n", iwidth);
  fprintf(out, "height %d\n", iheight);
  fprintf(out, "hsize %f\n", hsize);
  fprintf(out, "vsize %f\n", vsize);
  fprintf(out, "color %5.2f %5.2f %5.2f %5.2f\n", 
      backcolor[0] / 255.0, backcolor[1] / 255.0, 
      backcolor[2] / 255.0, backcolor[3] / 255.0);
  fprintf(out, "filter ");
  switch(filtertype){
  case Nofilter:
    fprintf(out, "nofilter\n");
    break;
  case Gaussian:
    fprintf(out, "gaussian %d\n", filterwidth);
    break;
  case Box:
    fprintf(out, "box %d\n", filterwidth);
    break;
  }

  for(i = 0; i < nlattices; i++)
    {
      fprintf(out, "\nlattice\n");
      fprintf(out, "rows %d\n", lattice[i]->nrows);
      fprintf(out, "cols %d\n", lattice[i]->ncols);
      fprintf(out, "color %5.2f %5.2f %5.2f %5.2f\n", 
       lattice[i]->color[0] / 255.0, lattice[i]->color[1] / 255.0, 
       lattice[i]->color[2] / 255.0, lattice[i]->color[3] / 255.0);
      fprintf(out, "probability %5.2f\n", lattice[i]->probability);

      fprintf(out, "hjitter ");
      switch(lattice[i]->hjitter){
      case Constant:
    fprintf(out, "constant %f\n", lattice[i]->hoffset);
    break;
      case Gaussian:
    fprintf(out, "normal %f %f\n", lattice[i]->hoffset, 
        lattice[i]->hrange);
    break;
      case Box:
    fprintf(out, "box %f %f\n", lattice[i]->hoffset, 
        lattice[i]->hrange);
    break;
      }

      fprintf(out, "vjitter ");
      switch(lattice[i]->vjitter){
      case Constant:
    fprintf(out, "constant %f\n", lattice[i]->voffset);
    break;
      case Normal:
    fprintf(out, "normal %f %f\n", lattice[i]->voffset, 
        lattice[i]->vrange);
    break;
      case Uniform:
    fprintf(out, "uniform %f %f\n", lattice[i]->voffset, 
        lattice[i]->vrange);
    break;
      }

      fprintf(out, "rjitter ");
      switch(lattice[i]->rjitter){
      case Constant:
    fprintf(out, "constant %f\n", lattice[i]->roffset);
    break;
      case Normal:
    fprintf(out, "normal %f %f\n", lattice[i]->roffset, 
        lattice[i]->rrange);
    break;
      case Uniform:
    fprintf(out, "uniform %f %f\n", lattice[i]->roffset, 
        lattice[i]->rrange);
    break;
      }

      fprintf(out, "style ");
      switch(lattice[i]->style){
      case Line:
    fprintf(out, "line %f %f %f\n", lattice[i]->angle, 
        lattice[i]->length, lattice[i]->thickness);
    break;
      case Dot:
    fprintf(out, "dot %f\n", lattice[i]->thickness);
    break;
      case Gabor:
    fprintf(out, "gabor %f %f %f\n", lattice[i]->angle, 
        lattice[i]->length, lattice[i]->thickness);
    break;
      }
    }

  CloseWrite();
  return 0;
}

bool LatTexture::Enable(const Point3d offset)
{
  if(!built)        // create lattice texturemap image just in time
    Build();

  if(image.empty()) // forget it if image failed to be created
    return false;

  glBegin( GL_LINES );  //These lines are here to make textures work with my
  glEnd();      //Wildcat card.  Otherwise, texture gets munged. (mdp)

  // if background transparent then layers of surfaces are desired
  if(backcolor[3] != 255)
    {
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, transparentBlendThreshold);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // generate texture coords automatically in both s and t directions
  glEnable(GL_TEXTURE_GEN_S);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  GLfloat splane[] = {1 / hOffsetScale, 0.0, 0.0, 0.0};
  glTexGenfv(GL_S, GL_OBJECT_PLANE, splane);
  glEnable(GL_TEXTURE_GEN_T);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  GLfloat tplane[] = {0.0, 1 / vOffsetScale, 0.0, 0.0};
  glTexGenfv(GL_T, GL_OBJECT_PLANE, tplane);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);

  glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    float xoffset = (float)(offset[0] / hOffsetScale - int(offset[0] / hOffsetScale));
    float yoffset = (float)(offset[1] / vOffsetScale - int(offset[1] / vOffsetScale));
    glTranslatef(xoffset, yoffset, 0);
  glMatrixMode(GL_MODELVIEW);

  return true;
}

bool LatTexture::Enable()
{
  if(!built)        // create lattice texturemap image just in time
    Build();

  if(image.empty()) // forget it if image failed to be created
    return false;

  glBegin( GL_LINES );  //These lines are here to make textures work with my
  glEnd();      //Wildcat card.  Otherwise, texture gets munged. (mdp)

  // if background transparent then layers of surfaces are desired
  if(backcolor[3] != 255)
    {
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, transparentBlendThreshold);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);
  return true;
}

void LatTexture::Disable()
{
  if(image.empty()) // forget it if image failed to be created
    return;

  if(backcolor[3] != 255)
    {
      glDisable(GL_ALPHA_TEST);
      glAlphaFunc(GL_ALWAYS, 0);
      glBlendFunc(GL_ONE, GL_ZERO);
    }

  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_2D);

  glMatrixMode(GL_TEXTURE);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
}

void LatTexture::Build()
{
  int texturesize;
  GLint h, w;
  twidth = iwidth;
  theight = iheight;

  // first reduce dimensions to a supported size if necessary
  do{
    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, twidth, theight,
         0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    if(w == 0 || h == 0){
      twidth /= 2; 
      theight /= 2;
    }
    texturesize = (twidth < theight? theight: twidth);
  }while((w == 0 || h == 0) && texturesize > 1);

  // now that we have a tentative size, create the image
  BuildImage(twidth, theight);

  if (!textureID) //texture creation moved here because there was flakiness when glGenTextures was called from a non-rendering thread (mdp 2/3/2005)
    glGenTextures(1, &textureID);
  if(!image.empty())
    {
      glBindTexture(GL_TEXTURE_2D, textureID);
      
      // the lattice texture should repeat over the surface
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      
      // create the mip-maps to be used as texture
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, twidth, theight,
            GL_RGBA, GL_UNSIGNED_BYTE, image.raw());

      // the lattice texture should be interpolated from a mip-map
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

  built = 1;
}

void LatTexture::Destroy()
{
  //delete []image;
  //image = NULL;
  image.reset();
  iwidth = iheight = 0;
  twidth = theight = 0;

  for(int i = 0; i < nlattices; i++)
    delete lattice[i];
  nlattices = 0;

//  glDeleteTextures(1, &textureID);
//  glGenTextures(1, &textureID);
  built = 0;
}

//
// default lattice texture is 512 x 512 with black opaque background
// world coordinate dimensions 100 x 100 m.
// one default 10 x 10 lattice of red dots
// no filtering
//
void LatTexture::CreateDefault()
{
  //image = NULL;
  image.reset();

  backcolor[0] = int(0.0 * 255);
  backcolor[0] = int(0.0 * 255);
  backcolor[0] = int(0.0 * 255);
  backcolor[0] = int(0.0 * 255);

  iwidth = iheight = 512;
  hsize = vsize = 100;
  hOffsetScale = hsize;
  vOffsetScale = vsize;

  filtertype = Nofilter;
  filterwidth = 0;

  nlattices = 1;
  lattice[0] = new LatticeElement;
  if(lattice[0] == NULL)
    {
      cerr << "Out of memory allocating space for lattice texture!" << endl;
      nlattices = 0;
    }
}

void LatTexture::Print() const
{
  cout << "Lattice Texture\n";
  cout << "image dimensions: [" << iheight << ", " << iwidth << "] ";
  cout << "texture dimensions: [" << theight << ", " << twidth << "] ";
  cout << "size: [" << vsize << ", " << hsize << "] ";
  cout << "background color: [" << int(backcolor[0]) << ", " << 
    int(backcolor[1]) << ", " << int(backcolor[2]) << ", " << 
    int(backcolor[3]) << "]\n";

  cout << "filter type  - ";
  switch(filtertype){
  case Nofilter:
    cout << "none";
    break;
  case Gaussian:
    cout << "gaussian: width " << filterwidth;
    break;
  case Box:
    cout << "box: width " << filterwidth;
    break;
  }
  cout << endl;

  for(int i = 0; i < nlattices; i++){
    cout << "lattice " << i << "\n";
    lattice[i]->Print();
  }
}

void LatTexture::Draw(bool asQuad)
{
  if(image.empty()) return;

  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, iwidth, 0, iheight);
  glMatrixMode(GL_MODELVIEW);

  glRasterPos2i(0, 0);
  glDrawPixels(iwidth, iheight, GL_RGBA, GL_UNSIGNED_BYTE, image.raw());
}

void LatTexture::Display(Tcl_Interp *interp, float min , float max, char * target)
{
}

#ifdef PRINTSTATS
void printminmaxalpha(int w, int h, Point4ub *image)
{
  int i, j;
  int alpha;
  int min = image[0][3];
  int max = image[0][3];
  int hist[10] = {0,0,0,0,0,0,0,0,0,0};
  int idx;

  for(i = 0; i < h; i++)
    for(j = 0; j < w; j++){
      alpha = image[i * w + j][3];
      idx = 10 - (255 - alpha) - 1;
      if(idx > 0) hist[idx]++;
      if(alpha < min)
    min = alpha;
      else if(alpha > max)
    max = alpha;
    }

  printf("alpha range [%d .. %d]\n", min, max);
  printf("histogram [246..255]: ");
  for(i = 0; i < 10; i++)
    printf("%d ", hist[i]);
  printf("\n");
}
#endif

//
// Create the image that will be tiled to form the texture
//
void LatTexture::BuildImage(int w, int h)
{
  int i, j, k;
  double dx, dy;
  double starty, startx;
  double probability;
  double xjitter, yjitter, rjitter;
  double x, y;

  rjitter = 0;
  
  // update image size instance variables
  iwidth = w;
  iheight = h;

  // create an image of texture map size
//   image = new Point4ub[iwidth * iheight];
//   if(image == NULL)
      image.Resize(iwidth,iheight);
  if(image.empty())
      
    {
      cerr << "Out of memory allocating space for lattice texture!" << endl;
      return;
    }

  glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDrawBuffer(GL_BACK);

    glViewport(0, 0, iwidth, iheight);  // use specified image size in pixels
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, hsize, 0, vsize);    // scale image to its world coord size

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);

    glShadeModel(GL_FLAT);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, transparentBlendThreshold);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // clear the image to its background color
    glClearColor(backcolor[0] / 255.0f, backcolor[1] / 255.0f, 
             backcolor[2] / 255.0f, backcolor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw the lattices in reverse order to enforce drawing precedence
    for(i = nlattices - 1; i >= 0; i--)
      {
    glColor4ub(lattice[i]->color[0], lattice[i]->color[1], 
           lattice[i]->color[2], lattice[i]->color[3]);

    dx = hsize / lattice[i]->ncols;
    dy = vsize / lattice[i]->nrows;
    startx = dx / 2;
    starty = dy / 2;
    probability = lattice[i]->probability;

    if(probability > 0)
      {
        for(j = 0; j < lattice[i]->nrows; j++)
          {
        y = starty + j * dy;
        for(k = 0; k < lattice[i]->ncols; k++)
          {
            if(probability == 1 || realrand() <= probability)
              {
            x = startx + k * dx;
            xjitter = jitter(lattice[i]->hjitter, 
                     lattice[i]->hoffset,
                     lattice[i]->hrange);
            yjitter = jitter(lattice[i]->vjitter, 
                     lattice[i]->voffset, 
                     lattice[i]->vrange);
            if(lattice[i]->style != Dot)
              rjitter = jitter(lattice[i]->rjitter, 
                       lattice[i]->roffset, 
                       lattice[i]->rrange);
            switch(lattice[i]->style){
            case Line:
              drawline((float)(x + xjitter), (float)(y + yjitter), (float)(rjitter), lattice[i]);
              break;
            case Dot:
              drawdot((float)(x + xjitter), (float)(y + yjitter), lattice[i]);
              break;
            case Gabor:
              drawgabor((float)(x + xjitter), (float)(y + yjitter), (float)(rjitter), lattice[i]);
              break;
            }
              }
          }
          }
      }
      }
    glFinish();

    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, iwidth, iheight, GL_RGBA, GL_UNSIGNED_BYTE, image.raw());

#ifdef PRINTSTATS
    printminmaxalpha(iwidth, iheight, image);
#endif
    filter();
#ifdef PRINTSTATS
    printminmaxalpha(iwidth, iheight, image);
#endif

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
  glPopAttrib();
}

double LatTexture::jitter(int type, double offset, double range)
{
  double boink = 0.0;

  switch(type)
    {
    case Constant:
      boink = offset;
      break;
    case Normal:
      boink = gauss(offset, range, 111);
      break;
    case Uniform:
      boink = offset + range * (2 * realrand() - 1);
      break;
    }

  return boink;
}

//
// Apply a filter to the image
//
// This code assumes seperable, square, symmetric filter kernel,
// so that only a 1-D kernel needs to be defined, and convolved
// twice -- once along the rows creating an intermediate image, 
// and once along the columns of the intermediate image creating
// the final filtered image
//
void LatTexture::filter()
{
  double *kernel = NULL;
  double *newim;
  double mu, sigma;
  double weight, sum;
  double value[4];
  int ivalue;
  int i, j, k, l, ii, jj;
  int df;
  double x;

  switch(filtertype)
    {
    case Nofilter:
      return;

    case Gaussian:
      kernel = new double[filterwidth];
      if(kernel == NULL)
    {
      cerr << "Out of memory allocating space for texture filter!" << endl;
      filtertype = Nofilter;
      return;
    }
      mu = filterwidth / 2.0;
      sigma = filterwidth / 4.0;
      for(i = 0, sum = 0; i < filterwidth; i++)
    {
      x = i + 0.5;
      weight = exp(-sqr((x - mu) / sigma));
      sum += weight;
      kernel[i] = weight;
    }
      for(i = 0; i < filterwidth; i++)
    kernel[i] /= sum;
      break;
      
    case Box:
      kernel = new double[filterwidth];
      if(kernel == NULL)
    {
      cerr << "Out of memory allocating space for texture filter!" << endl;
      filtertype = Nofilter;
      return;
    }
      for(i = 0; i < filterwidth; i++)
    kernel[i] = 1.0 / filterwidth;
      break;
    }

#ifdef PRINTKERNEL
  double gain;
  for(i = 0, gain = 0; i < filterwidth; i++)
    gain += kernel[i];
  printf("gain: %f, kernel: ", gain);
  for(i = 0; i < filterwidth; i++)
    printf("%f ", kernel[i]);
  printf("\n");
#endif

  newim = new double[(iwidth  * 4) * iheight];
  if(newim == NULL)
    {
      cerr << "Out of memory allocating space for texture filter!" << endl;
      filtertype = Nofilter;   
      delete []kernel;
    }

  df = filterwidth / 2;
  // convolve along the rows
  for(i = 0; i < iheight; i++)
    for(j = 0; j < iwidth; j++)
      {
    for(l = 0; l < 4; l++)
      value[l] = 0;
    for(k = 0; k < filterwidth; k++)
      {
        jj = j + k - df;
        // if col index off image, wraparound (i.e. filter on a cylinder)
        if(jj < 0)
          jj = iwidth + jj;
        else if(jj >= iwidth)
          jj = jj - iwidth;
        for(l = 0; l < 4; l++)
          value[l] += kernel[k] * image.GetRaster().GetRow(i)[jj][l]; // image[i * iwidth + jj][l];
      }
    for(l = 0; l < 4; l++)
      newim[i * (iwidth * 4) + (j * 4) + l] = value[l];
      }

  // convolve along the columns
  for(j = 0; j < iwidth; j++)
    for(i = 0; i < iheight; i++)
      {
    for(l = 0; l < 4; l++)
      value[l] = 0;
    for(k = 0; k < filterwidth; k++)
      {
        ii = i + k - df;
        // if row index off image, wraparound (i.e. filter on a cylinder)
        if(ii < 0)
          ii = iheight + ii;
        else if(ii >= iheight)
          ii = ii - iheight;
        for(l = 0; l < 4; l++)
          value[l] += kernel[k] * newim[ii * (iwidth * 4) + (j * 4) + l];
      }
    for(l = 0; l < 4; l++)
      {
        ivalue = int(value[l] >= 0? value[l] + 0.5: 0);
        //image[i * iwidth + j][l] = (ivalue <= 255? ivalue: 255);
        image.GetRaster().GetRow(i)[j][l] = (ivalue <= 255? ivalue: 255);
      }
      }
  
  delete []newim;
  delete []kernel;
}

void LatTexture::drawline(float x, float y, float rjitter, LatticeElement *e)
{
  static const double PI = 3.1415926536;
  static const double DEG2RAD = PI / 180.0;
  
  float delx = (float)(0.5 * e->length * cos(DEG2RAD * (e->angle + rjitter)));
  float dely = (float)(0.5 * e->length * sin(DEG2RAD * (e->angle + rjitter)));
  float dx, dy;
  double r = e->thickness;
  int pixwidth = int(r / (hsize / iwidth) + 0.5);
  pixwidth = (pixwidth > 0? pixwidth: 1);

  glLineWidth((float)pixwidth);
  glBegin(GL_LINES);
    glVertex2f(x - delx, y - dely);
    glVertex2f(x + delx, y + dely);
  glEnd();

  dx = dy = 0;
  if(x - (delx + r / 2) < 0)
    dx = hsize;
  else if(x + (delx + r / 2) > hsize)
    dx = -hsize;
  if(y - (dely + r / 2) < 0)
    dy = vsize;
  else if(y + (dely + r / 2) > vsize)
    dy = -vsize;
  if(dx != 0){
    glPushMatrix();
      glTranslatef(dx, 0, 0);
      glBegin(GL_LINES);
        glVertex2f(x - delx, y - dely);
        glVertex2f(x + delx, y + dely);
      glEnd();
    glPopMatrix();
  }
  if(dy != 0){
    glPushMatrix();
      glTranslatef(0, dy, 0);
      glBegin(GL_LINES);
        glVertex2f(x - delx, y - dely);
        glVertex2f(x + delx, y + dely);
      glEnd();
    glPopMatrix();
  }
  if(dx != 0 && dy != 0){
    glPushMatrix();
      glTranslatef(dx, dy, 0);
      glBegin(GL_LINES);
        glVertex2f(x - delx, y - dely);
        glVertex2f(x + delx, y + dely);
      glEnd();
    glPopMatrix();
  }
}

void LatTexture::drawdot(float x, float y, LatticeElement *e)
{
  static int dotdisplaylist = -1;
  static const double PI = 3.1415926536;
  static const double CIRC_INC = (2 * PI / 36); // radians per circle segment

  float r = e->thickness / 2;
  double theta;
  float dx, dy;

  if(r <= 0) return;

  if(dotdisplaylist == -1)
    {
      dotdisplaylist = glGenLists(1);

      glNewList(dotdisplaylist, GL_COMPILE);
        glBegin(GL_POLYGON);
          for(theta = 0.0; theta < 2 * PI; theta += CIRC_INC)
        glVertex2f((float)cos(theta), (float)sin(theta));
        glEnd();
      glEndList();
    }

  glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(r, r, 1);
    glCallList(dotdisplaylist);
  glPopMatrix();

  dx = dy = 0;
  if(x - r < 0)
    dx = hsize;
  else if(x + r > hsize)
    dx = -hsize;
  if(y - r < 0)
    dy = vsize;
  else if(y + r > vsize)
    dy = -vsize;
  if(dx != 0){
    glPushMatrix();
      glTranslatef(x + dx, y, 0);
      glScalef(r, r, 1);
      glCallList(dotdisplaylist);
    glPopMatrix();
  }
  if(dy != 0){
    glPushMatrix();
      glTranslatef(x, y + dy, 0);
      glScalef(r, r, 1);
      glCallList(dotdisplaylist);
    glPopMatrix();
  }
  if(dx != 0 && dy != 0){
    glPushMatrix();
      glTranslatef(x + dx, y + dy, 0);
      glScalef(r, r, 1);
      glCallList(dotdisplaylist);
    glPopMatrix();
  }
}

void LatTexture::drawgabor(float x, float y, float rjitter, LatticeElement *e)
{
  Point4ub splat[T_SIZE][T_SIZE];
  int ix, iy;
  int x0, y0;
  int w, h;
  int dw, dh;

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, iwidth, 0, iheight);
  glMatrixMode(GL_MODELVIEW);

  gabor(e->length, e->angle + rjitter, e->color, e->thickness, splat);

  ix = int((x / hsize) * iwidth);
  iy = int((y / vsize) * iheight);

  x0 = ix - T_SIZE / 2;
  y0 = iy - T_SIZE / 2;

  dw = dh = 0;

  if(x0 < 0){
    dw = -x0;
    x0 = 0;
  }
  //else if(x0 + T_SIZE >= iwidth){
  //  dw = iwidth - x0;
  //  x0 = 0;
  //}
  if(y0 < 0){
    dh = -y0;
    y0 = 0;
  }
  //else if(y0 + T_SIZE >= iheight){
  //  dh = iheight - y0;
  //  y0 = 0;
  //}
  w = T_SIZE - dw;
  h = T_SIZE - dh;

  //if(dw != 0 || dh != 0)
  //  printf("ix-iy[%d %d] x0-y0[%d %d] dw-dh[%d %d] w-h[%d %d]\n", ix, iy, 
    //   x0, y0, dw, dh, w, h);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, T_SIZE);

  glRasterPos2i(x0, y0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, dw);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, dh);

  glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, splat);

  // take care of any wraparound so that texture tiles
  if(dw > 0){
    //printf("horz x0-y0[%d %d] dw-dh[%d %d] w-h[%d %d]\n", iwidth - dw, y0, 
    //   0, dh, dw, h);
    glRasterPos2i(iwidth - dw, y0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, dh);
    glDrawPixels(dw, h, GL_RGBA, GL_UNSIGNED_BYTE, splat);
  }
  if(dh > 0){
    //printf("vert x0-y0[%d %d] dw-dh[%d %d] w-h[%d %d]\n", x0, iheight - dh, 
    //   dw, 0, w, dh);
    glRasterPos2i(x0, iheight - dh);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, dw);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glDrawPixels(w, dh, GL_RGBA, GL_UNSIGNED_BYTE, splat);
  }
  if(dh > 0 && dw > 0){
    //printf("crnr x0-y0[%d %d] dw-dh[%d %d] w-h[%d %d]\n", iwidth-dw, iheight-dh,
    //   0, 0, dw, dh);
    glRasterPos2i(iwidth - dw, iheight - dh);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glDrawPixels(dw, dh, GL_RGBA, GL_UNSIGNED_BYTE, splat);
  }

  // clean up for future glDrawPixels() calls
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/
/* Gabor "splat" generator

    Colin Ware
    conversion to C++ -- Apr. 17, 2001, Donald House

   input orientation in degrees
   size is the wavelength expressed as pixels
   orientation is given by angle (in degrees)
   contrast is the contrast amplitude
   ratio is the ratio of gaussian size to wavelength
     If ratio is one the function decays
     to 1/e in Lambda = 1.0 (Lamdda span)
*/
void gabor(float size, float angle, Point4ub color, float ratio, 
       Point4ub image[T_SIZE][T_SIZE])
{
  static const double PI = 3.1415926536;
  static const double DEG2RAD = PI / 180.0;
  static const double halfx = T_SIZE / 2.0;
  static const double halfy = T_SIZE / 2.0;

  double freq;
  double prod;
  double cosr, sinr;
  double x, y;
  int ix, iy;
  double value;

  sinr = -cos(DEG2RAD * angle); // make axis 90 deg. to splat long axis
  cosr = sin(DEG2RAD * angle);

  freq = 2.0 * PI / size;
  ratio = (float) (1.0 / (ratio * 2.0 * PI));
  ratio = -ratio * ratio;

  for (ix = 0; ix < T_SIZE; ++ix)
    for(iy = 0; iy < T_SIZE; ++iy){
      x = (ix - halfx) * freq;
      y = (iy - halfy) * freq;

      /* rotate x and y */
      prod = (cosr * x + sinr * y);

      value = cos(prod) * exp((x * x + y * y) * ratio);
      value = ((value < 0)? 0: ((value > 1)? 1: value));
      image[iy][ix][0] = int(color[0] * value);
      image[iy][ix][1] = int(color[1] * value);
      image[iy][ix][2] = int(color[2] * value);
      image[iy][ix][3] = int(color[3] * value);
    }
}
