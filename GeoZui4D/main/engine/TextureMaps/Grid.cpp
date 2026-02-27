/* Copyright (c) 2000 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include <iostream>
#include "Grid.h"
#include <boost/filesystem.hpp>
#include <GL/glu.h>

#define GRIDMAGICNUMBER		"begin grid 1.0"

#define DEFAULTDIVISIONS	10
#define DEFAULTLINEWIDTH	5
#define DEFAULTRED		0.2
#define DEFAULTGREEN		0.2
#define DEFAULTBLUE		0.7
#define DEFAULTALPHA		0.7


using namespace std;

// definitions for parsing the grid description file
enum GridParams{IMAGE, MAINGRID, SUBGRID, 
		WIDTH, HEIGHT, HSIZE, VSIZE, 
		LINEWIDTH, COLOR, HDIVS, VDIVS};
const char *GridNames[] = {"IMAGE", "MAINGRID", "SUBGRID", 
		     "WIDTH", "HEIGHT", "HSIZE", "VSIZE", 
		     "LINEWIDTH", "COLOR", "HDIVS", "VDIVS", NULL};

//
// Construct default grid element
//
GridElement::GridElement()
{
  hdivs = vdivs = DEFAULTDIVISIONS;
  linewidth = DEFAULTLINEWIDTH;
  linecolor[0] = int(DEFAULTRED * 255);
  linecolor[1] = int(DEFAULTGREEN * 255);
  linecolor[2] = int(DEFAULTBLUE * 255);
  linecolor[3] = int(DEFAULTALPHA * 255);
}

void GridElement::Print() const
{
  cout << "divisions: [" << hdivs << ", " << vdivs << "] ";
  cout << "linewidth: " << linewidth << " ";
  cout << "color: [" << int(linecolor[0]) << ", " << int(linecolor[1]) << ", "
       << int(linecolor[2]) << ", " << int(linecolor[3]) << "]";
}

Grid::Grid()
{
    type = GRIDTEXTURE;
    applyMode = GL_MODULATE;

  // set up the reader to look for BEGIN GRID 1.0, and with set of keywords
  SetTypeAndVersion("GRID", 1, 0);
  SetKeywordTable(GridNames);

  CreateDefault();
}

Grid::~Grid()
{
  Destroy();
}

void Grid::Init()
{
  Destroy();

  if(GetFilename().empty())
    SetFilename("default.grid");
	
  if(Load() != 0)
    {
      cerr << "Creating default grid" << endl;
      CreateDefault();
    }
}

int Grid::Load()
{
  int item;
  int mode = IMAGE;
  char **fields = NULL;
  char *fname;
  int i;

  if(Open())
    return 1;

  Destroy();		// make sure that we build a new grid texture map

  for(fields = NextLine(); fields != NULL; fields = NextLine())
    {
      for(i = 0, fname = fields[i++]; fname != NULL; fname = fields[i++])
	{
	  item = Lookup(fname);
	  // first error check the item to make sure in correct file segment
	  switch(item)
	    {
	    case IMAGE:
	    case MAINGRID:
	      mode = item;
	      break;
	    case SUBGRID:
	      mode = item;
	      if(nsubgrids == MAXSUBGRIDS){
		cerr << "maximum of " << MAXSUBGRIDS << 
		  " subgrids can be specified!" << endl;
		return 1;
	      }
	      subgrid[nsubgrids] = new GridElement;
	      if(subgrid[nsubgrids++] == NULL)
		{
		  cerr << "Out of memory allocating subgrid!" << endl;
		  return 1; 
		}
	      break;
	    case WIDTH:
	    case HEIGHT:
	      if(mode != IMAGE){
		cerr << "width and height only specified for image!" << endl;
		return 1;
	      }
	      break;
	    case HSIZE:
	    case VSIZE:
	      if(mode != MAINGRID){
		cerr << "hsize and vsize only specified for main grid!" << endl;
		return 1;
	      }
	      break;
	    case LINEWIDTH:
	      if(mode == IMAGE){
		cerr << "linewidth and color only specified for a grid!" << endl;
		return 1;
	      }
	      break;
	    case HDIVS:
	    case VDIVS:
	      if(mode != SUBGRID){
		cerr << "hdivs and vdivs only specified for a subgrid!" << endl;
		return 1;
	      }
	      break;
	    case -1:
	      cerr << "Invalid field " << fname << 
		" in grid description file!" << endl;
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
	    case LINEWIDTH:
	      if(mode == MAINGRID)
		maingrid.linewidth = ConvertInt(i);
	      else
		subgrid[nsubgrids - 1]->linewidth = ConvertInt(i);
	      break;
	    case COLOR:
	      switch(mode)
		{
		case IMAGE:
		  ConvertFloatColor(backcolor, i);
		  break;
		case MAINGRID:
		  ConvertFloatColor(maingrid.linecolor, i);
		  break;
		case SUBGRID:
		  ConvertFloatColor(subgrid[nsubgrids - 1]->linecolor, i);
		}
	      break;
	    case HDIVS:
	      subgrid[nsubgrids - 1]->hdivs = ConvertInt(i);
	      break;
	    case VDIVS:
	      subgrid[nsubgrids - 1]->vdivs = ConvertInt(i);
	      break;
	    }
	}
    }

	hOffsetScale = hsize;
	vOffsetScale = vsize;
  Close();
  return 0;
}

int Grid::Save(std::string const &fn)
{
  int i;

  if(OpenWrite(fn))
    return 1;

  fprintf(out, GRIDMAGICNUMBER);

  fprintf(out, "\n\nimage\n");
  fprintf(out, "width %d\n", iwidth);
  fprintf(out, "height %d\n", iheight);
  fprintf(out, "color %5.2f %5.2f %5.2f %5.2f\n", 
	  backcolor[0] / 255.0, backcolor[1] / 255.0, 
	  backcolor[2] / 255.0, backcolor[3] / 255.0);

  fprintf(out, "\nmaingrid\n");
  fprintf(out, "hsize %f\n", hsize);
  fprintf(out, "vsize %f\n", vsize);
  fprintf(out, "linewidth %d\n", maingrid.linewidth);
  fprintf(out, "color %5.2f %5.2f %5.2f %5.2f\n", 
	  maingrid.linecolor[0] / 255.0, maingrid.linecolor[1] / 255.0, 
	  maingrid.linecolor[2] / 255.0, maingrid.linecolor[3] / 255.0);

  for(i = 0; i < nsubgrids; i++)
    {
      fprintf(out, "\nsubgrid\n");
      fprintf(out, "hdivs %d\n", subgrid[i]->hdivs);
      fprintf(out, "vdivs %d\n", subgrid[i]->vdivs);
      fprintf(out, "linewidth %d\n", subgrid[i]->linewidth);
      fprintf(out, "color %5.2f %5.2f %5.2f %5.2f\n", 
	   subgrid[i]->linecolor[0] / 255.0, subgrid[i]->linecolor[1] / 255.0, 
	   subgrid[i]->linecolor[2] / 255.0, subgrid[i]->linecolor[3] / 255.0);
    }

  CloseWrite();
  return 0;
}

bool Grid::Enable(const Point3d offset)
{
  if(!built)		// create grid texturemap image just in time
    Build();

  if(image.empty())	// forget it if image failed to be created
    return false;

  glBegin( GL_LINES ); //These lines are here to make textures work with my
  glEnd();		//Wildcat card.  Otherwise, texture gets munged. (mdp)

  // if background transparent then layers of surfaces are desired
  if(backcolor[3] != 255)
    {
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, transparentBlendThreshold);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, textureID);

  // generate texture coords automatically in both s and t directions
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  GLfloat splane[] = {1 / hOffsetScale, 0.0, 0.0, 0.0};
  glTexGenfv(GL_S, GL_OBJECT_PLANE, splane);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  GLfloat tplane[] = {0.0, 1 / vOffsetScale, 0.0, 0.0};
  glTexGenfv(GL_T, GL_OBJECT_PLANE, tplane);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);

  glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    float xoffset = (float)(offset[0] / hOffsetScale - int(offset[0] / hOffsetScale));
    float yoffset = (float)(offset[1] / vOffsetScale - int(offset[1] / vOffsetScale));
    glTranslatef(xoffset, yoffset, 0);

  glBegin( GL_LINES ); // These lines are here to make textures work with my
  glEnd();	       // Wildcat card.  Otherwise, texture gets munged. (mdp)

  glMatrixMode(GL_MODELVIEW);
  return true;
}

bool Grid::Enable()
{
  if(!built)		// create grid texturemap image just in time
    Build();

  if(image.empty())	// forget it if image failed to be created
    return false;

  glBegin( GL_LINES ); //These lines are here to make textures work with my
  glEnd();		//Wildcat card.  Otherwise, texture gets munged. (mdp)

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

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);
  return true;
}

void Grid::Disable()
{
  if(image.empty())	// forget it if image failed to be created
    return;

  if(backcolor[3] != 255)
    {
      glDisable(GL_ALPHA_TEST);
      glAlphaFunc(GL_ALWAYS, 0);
//      glDisable(GL_BLEND); //added by mdp during merge on 5/22/2001  ; commented out by mdp 8/31/2004
      glBlendFunc(GL_ONE, GL_ZERO);
    }

  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_2D);

  glMatrixMode(GL_TEXTURE);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
}

void Grid::Build()
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
    texturesize = (theight < twidth? theight: twidth);
  }while((w == 0 || h == 0) && texturesize > 1);

  // now that we have a tentative size, create the image
  BuildImage(twidth, theight);

  if (!textureID) //texture creation moved here because there was flakiness when glGenTextures was called from a non-rendering thread (mdp 2/3/2005)
        glGenTextures(1, &textureID);
  if(image.empty())
    {
      glBindTexture(GL_TEXTURE_2D, textureID);

      // the grid texture should repeat over the surface
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      // create the mip-maps to be used as texture
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, twidth, theight,
			GL_RGBA, GL_UNSIGNED_BYTE, image.raw());

      // the grid texture should be interpolated from a mip-map
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		      GL_LINEAR_MIPMAP_LINEAR);
    }

  built = 1;
}

void Grid::Destroy()
{
  image.reset();
  twidth = theight = 0;
  iwidth = iheight = 0;

  for(int i = 0; i < nsubgrids; i++)
    delete subgrid[i];
  nsubgrids = 0;

//  glDeleteTextures(1, &textureID);
//  glGenTextures(1, &textureID);
  built = 0;
}

void Grid::CreateDefault()
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

  maingrid.hdivs = maingrid.vdivs = 1;
  maingrid.linewidth = 10;
  maingrid.linecolor[0] = int(0.3 * 255);
  maingrid.linecolor[1] = int(0.3 * 255);
  maingrid.linecolor[2] = int(0.9 * 255);
  maingrid.linecolor[3] = int(0.7 * 255);

  nsubgrids = 0;
}

void Grid::Print() const
{
  cout << "Grid Texture\n";
  cout << "image dimensions: [" << iheight << ", " << iwidth << "] ";
  cout << "texture dimensions: [" << theight << ", " << twidth << "] ";
  cout << "size: [" << vsize << ", " << hsize << "] ";
  cout << "background color: [" << int(backcolor[0]) << ", " << 
    int(backcolor[1]) << ", " << int(backcolor[2]) << ", " << 
    int(backcolor[3]) << "]\n";

  cout << "main grid\n";
  maingrid.Print();

  for(int i = 0; i < nsubgrids; i++){
    cout << "\n";
    cout << "subgrid " << i << "\n";
    subgrid[i]->Print();
  }
}

void Grid::Draw(bool asQuad)
{
  if(!image.empty()) return;

  glViewport(0, 0, iwidth, iheight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, iwidth, 0, iheight);
  glMatrixMode(GL_MODELVIEW);

  glRasterPos2i(0, 0);
  glDrawPixels(iwidth, iheight, GL_RGBA, GL_UNSIGNED_BYTE, image.raw());
}

void Grid::Display(Tcl_Interp *interp, float min , float max, char * target)
{
}

void Grid::clear(const Point4ub &clearcolor)
{
    image = gz4d::ImageRGBA8::raster_type(iwidth,iheight, gz4d::ImageRGBA8::pixel_type(gz4d::Black()));
//   int size = iwidth * iheight;
//   int i;
//   Point4ub *imptr;
// 
//   for(i = 0, imptr = image; i < size; i++, imptr++)
//     {
//       (*imptr)[0] = clearcolor[0];
//       (*imptr)[1] = clearcolor[1];
//       (*imptr)[2] = clearcolor[2];
//       (*imptr)[3] = clearcolor[3];
//     }
}

void Grid::drawrow(float y, int linewidth, const Point4ub &color) 
{
  Point4ub *imptr;
  unsigned char *cimptr;
  int offset, deltaoffset;
  int i, j, k;

  int row = int(y + 0.5);
  if(row < 0 || row >= iheight || linewidth <= 0) return;

  // draw the first scanline of the row into the image
  for(i = 0, imptr = reinterpret_cast<Point4ub*>(image.raw()) + row * iwidth; i < iwidth; i++, imptr++)
    for(j = 0; j < 4; j++)
      (*imptr)[j] = color[j];

  // draw the remaining scanlines of the row by copying the first
  for(k = 1; k < linewidth; k++){
    if(row + k >= iheight) return;
    offset = row * iwidth;
    deltaoffset = 4 * k * iwidth;
    for(i = 0, cimptr = (unsigned char *)(image.raw() + offset); i < 4 * iwidth; 
	i++, cimptr++)
      *(cimptr + deltaoffset) = *(cimptr);
  }
}

void Grid::drawcol(float x, int linewidth, const Point4ub &color)
{
  Point4ub *imptr;
  unsigned char *cimptr;
  int i, j, k;

  int col = int(x + 0.5);
  if(col < 0 || col >= iwidth || linewidth <= 0) return;

  // draw the first line of the column into the image
  for(i = 0, imptr = reinterpret_cast<Point4ub*>(image.raw()) + col; i < iheight; i++, imptr += iwidth)
    for(j = 0; j < 4; j++)
      (*imptr)[j] = color[j];

  // draw the remaining lines of the column by copying the first
  for(i = 0, cimptr = (unsigned char *)(image.raw() + col*4); i < iheight; 
      i++, cimptr += 4 * iwidth)
    for(k = 0; k < (linewidth - 1) * 4; k++)
      if((col + k / 4) < iwidth)
	*(cimptr + k + 4) = *(cimptr + k);
}

void Grid::BuildImage(int w, int h)
{
  int size = w * h;
  int thickness, extra;
  float delta;
  int i, j;

  // update image size instance variables
  iwidth = w;
  iheight = h;

  // create an initialized image of texture map size
  //image = new Point4ub[size];
  //if(image == NULL)
  image.Resize(iwidth,iheight);
  if(image.empty())
    {
      cerr << "Out of memory allocating image for grid texture!" << endl;
      return;
    }
  clear(backcolor);

  // draw the subgrids in reverse order to enforce drawing precedence
  for(i = nsubgrids - 1; i >= 0; i--)
    {
      thickness = subgrid[i]->linewidth;

      delta = (float)iheight / (float)subgrid[i]->vdivs;
      for(j = 1; j <  subgrid[i]->vdivs; j++)
	drawrow(j * delta - thickness / 2, thickness, subgrid[i]->linecolor);
      
      delta = (float)iwidth / (float)subgrid[i]->hdivs;
      for(j = 1; j <  subgrid[i]->hdivs; j++)
	drawcol(j * delta - thickness / 2, thickness, subgrid[i]->linecolor);
    }

  // draw the main outline of the grid last to enforce its top precedence
  extra = maingrid.linewidth % 2;
  thickness = maingrid.linewidth / 2;
  drawrow(0, thickness + extra, maingrid.linecolor);
  drawrow((float)(iheight - thickness), thickness, maingrid.linecolor);
  drawcol(0, thickness + extra, maingrid.linecolor);
  drawcol((float)(iwidth - thickness), thickness, maingrid.linecolor);
}
