// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string.h>
#include "engine/InputMediator.h"
#include "engine/TextureMaps/Clut.h"
#include <filesystem>
#include <GL/glu.h>

const EventType *Clut::clutChangedInputType =
					Interactor::newEventType("clutChanged", Interactor::DataAsInt);

using namespace std;


Clut::Clut()
{
  type = CLUTTEXTURE;
  applyMode = GL_MODULATE;

  // set up the reader to not look for BEGIN line for a CLUT
  SetTypeAndVersion("");

  clut = NULL;
  minheight = maxheight = range = 0;
  scl = 1;
  trans = 0;
  built = 0;
  boundsChanged = GL_FALSE;

  useSurfaceColor = GL_FALSE;
  surfaceColor[0] = surfaceColor[1] = surfaceColor[2] = surfaceColor[3] = 1.0;

  CreateDefault();
}

Clut::~Clut()
{
  Destroy();
}

void Clut::Init()
{
	Destroy();

	if(GetFilename().empty())
		SetFilename("default.clut");

	if(Load())
	{
		CreateDefault();
	}
}

int Clut::Load()
{
  clutnode * newnode;
  char instring[1000];
  double h;
  int r,g,b,a;
  float ver;

  if(Open())
    return 1;

  Destroy();

  if(fgets(instring, 1000, in) == NULL)
    {
      cerr << "Load error, can't read file!\n";
      return 1;
    }

  StripNewLine(instring);

  if (strstr(instring, "clut1.0") != 0)
	  ver = 1.0;
  else if (strstr(instring, "clut2.0") != 0)
	  ver = 2.0;
  else
  {
	cerr << "Load error, header error!\n";
	return 1;
  }

  for(char * ret = fgets(instring, 1000, in); ret && strstr(instring, "done") == 0;
      ret = fgets(instring, 1000, in))				     
    {
		if (ver >= 2.0f && strstr(instring, "base") == instring)
		{
			sscanf(instring, "%*s %d %d %d %d", &r, &g, &b, &a);
			surfaceColor[0] = r / 255.0f;
			surfaceColor[1] = g / 255.0f;
			surfaceColor[2] = b / 255.0f;
			surfaceColor[3] = a / 255.0f;
			useSurfaceColor = GL_TRUE;
			continue;
		}
      newnode = new clutnode;

      sscanf(instring, "%lf %d %d %d %d", &h,&r,&g,&b,&a);
      newnode->height = h;
      newnode->color[0] = r;
      newnode->color[1] = g;
      newnode->color[2] = b;
	  if (ver == 1.0)
		newnode->color[3] = 255;
	  else
		newnode->color[3] = a;

      AddNode(newnode);
    }

  SetMinMax();

  Close();
  return 0;
  
}

int Clut::Save(std::string const &fn)
{
  clutnode * cursor = NULL;

  if(OpenWrite(fn))
    return 1;

  fprintf(out, "clut2.0\n");
  for(cursor = clut; cursor != NULL; cursor = cursor->next)
    fprintf(out, "%f %i %i %i %i\n", cursor->height, cursor->color[0], 
	    cursor->color[1], cursor->color[2], cursor->color[3]);

  fprintf(out, "done\n");

  CloseWrite();
  return 0;
}

bool Clut::Enable(const Point3d offset)
{

  glBegin( GL_LINES ); //These lines are here to make textures work with my
  glEnd();			   //Wildcat card.  Otherwise, texture gets munged. (mdp)

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);

  //mdp added alpha stuff 8/31/2004
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, transparentBlendThreshold);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_1D);

  glBindTexture(GL_TEXTURE_1D, textureID);
  if(!built)		// create clut OpenGL texturemap just in time
    Build();

  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  GLfloat textplane[] = {0.0, 0.0, 1.0, 0.0};
  glTexGenfv(GL_S, GL_OBJECT_PLANE, textplane);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);

  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glLoadIdentity();
  glScalef((float)scl, 1.0, 1.0);
  glTranslatef((float)(trans + offset[2]), 0.0, 0.0);

  glBegin( GL_LINES );  //These lines are here to make textures work with my
  glEnd();		//Wildcat card.  Otherwise, texture gets munged. (mdp)

  glMatrixMode(GL_MODELVIEW);

  if (useSurfaceColor)
	  glColor4fv(surfaceColor);

  return true;
}

bool Clut::Enable()
{
  if(!built)		// create clut OpenGL texturemap just in time
    Build();

  glBegin( GL_LINES ); //These lines are here to make textures work with my
  glEnd();			   //Wildcat card.  Otherwise, texture gets munged. (mdp)

  //mdp added alpha stuff 8/31/2004
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, transparentBlendThreshold);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, textureID);
  
  glDisable(GL_TEXTURE_GEN_S);
//  glDisable(GL_TEXTURE_GEN_T);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);
  
  if (useSurfaceColor)
	  glColor4fv(surfaceColor);
  return true;
}

void Clut::SetHeight(double d)
{
    glTexCoord1d((trans + d) * scl);
}

void Clut::beginTextureOffset(float x, float y, float z, float scale)
{
	GLint matrixMode;

	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glTranslatef(z, 0.0, 0.0);
	glScalef(scale, 1.0, 1.0);
	glMatrixMode(matrixMode);
}

void Clut::Disable()
{
  //glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_1D);
  //glBindTexture(GL_TEXTURE_1D,0);

  //mdp added alpha stuff 8/31/2004
  glDisable(GL_ALPHA_TEST);
  //glDisable(GL_BLEND);
  glAlphaFunc(GL_ALWAYS, 0);
  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_TEXTURE_GEN_S);
  glMatrixMode(GL_TEXTURE);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);

  if (useSurfaceColor)
	  glColor4f(1.0, 1.0, 1.0, 1.0);
}

void Clut::Print() const
{
  clutnode * cursor;

  for(cursor = clut; cursor != NULL; cursor = cursor->next)
    printf("%f %i %i %i %i\n", cursor->height, cursor->color[0], 
	   cursor->color[1], cursor->color[2], cursor->color[3]);
}

void Clut::Build()
{
	int i;
	float h;
	int texturesize;
	GLint w;
	GLboolean wasImage = GL_TRUE;
	
	if(clut == NULL)
		return;
	
	if (image.empty())
	{
		wasImage = GL_FALSE;
		texturesize = TEXTUREMAP_TEXTURE_SIZE;
		
		// must first check if texture size is supported
		// and reduce the desired texture size to fit texture memory
		do{
			glTexImage1D(GL_PROXY_TEXTURE_1D, 0, GL_RGBA, texturesize, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glGetTexLevelParameteriv(GL_PROXY_TEXTURE_1D, 0, GL_TEXTURE_WIDTH, &w);
			if(w == 0)
				texturesize /= 2;
		}while(w == 0 && texturesize > 1);
		
		twidth = texturesize;
		theight = 1;
		iheight = theight;
		iwidth = twidth;
                image.Resize(iwidth,iheight);
	}
	else
            texturesize = twidth;
	
	for(i = 0; i < texturesize; i++)
        {
		h = minheight + ((i * range) / (texturesize - 1));
                image.GetRaster().GetRow(0)[i][0] = GetComponent(h, 0);
                image.GetRaster().GetRow(0)[i][1] = GetComponent(h, 1);
                image.GetRaster().GetRow(0)[i][2] = GetComponent(h, 2);
                image.GetRaster().GetRow(0)[i][3] = GetComponent(h, 3);
        }
	
        if (!textureID) //texture creation moved here because there was flakiness when glGenTextures was called from a non-rendering thread (mdp 2/3/2005)
                glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_1D, textureID);
        
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, texturesize, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, image.raw());
        
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP); 
	
	//scale and translate amounts map the texture to the proper clut values
	scl = 1.0 / range;
	trans = range - maxheight + (range / (twidth * 2));
	
	if (boundsChanged)
	{
                globalInputMediator->Trigger(clutChangedInputType->value, (void *)textureID);
		boundsChanged = GL_FALSE;
	}
	
	// indicate OpenGL texture is now built
	built = 1;
}

void Clut::Destroy()
{
  clutnode * cursor;

  while (clut != NULL)
    {
      cursor = clut;
      clut = clut->next;
      delete cursor;
    }

  // kill OpenGL texture and indicate it is no longer built
/*
  glDeleteTextures(1, &textureID);	// free up existing texture
  glGenTextures(1, &textureID);		// and get new id in case needed later
*/
  built = 0;
}

void Clut::CreateDefault()
{
  clutnode * newnode;
		
  newnode = new clutnode;
  newnode->height = -6000;
  newnode->color[0] = 17;
  newnode->color[1] = 10;
  newnode->color[2] = 59;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -5000;
  newnode->color[0] = 18;
  newnode->color[1] = 10;
  newnode->color[2] = 59;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -4000;
  newnode->color[0] = 23;
  newnode->color[1] = 49;
  newnode->color[2] = 111;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -3000;
  newnode->color[0] = 20;
  newnode->color[1] = 90;
  newnode->color[2] = 140;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -2500;
  newnode->color[0] = 27;
  newnode->color[1] = 104;
  newnode->color[2] = 164;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -2000;
  newnode->color[0] = 30;
  newnode->color[1] = 114;
  newnode->color[2] = 179;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -1500;
  newnode->color[0] = 29;
  newnode->color[1] = 139;
  newnode->color[2] = 196;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -1000;
  newnode->color[0] = 27;
  newnode->color[1] = 165;
  newnode->color[2] = 210;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -500;
  newnode->color[0] = 28;
  newnode->color[1] = 184;
  newnode->color[2] = 224;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -200;
  newnode->color[0] = 27;
  newnode->color[1] = 204;
  newnode->color[2] = 236;
  newnode->color[3] = 255;
  AddNode(newnode);
		
  newnode = new clutnode;
  newnode->height = -100;
  newnode->color[0] = 27;
  newnode->color[1] = 213;
  newnode->color[2] = 241;
  newnode->color[3] = 255;
  AddNode(newnode);

  newnode = new clutnode;
  newnode->height = -50;
  newnode->color[0] = 38;
  newnode->color[1] = 223;
  newnode->color[2] = 241;
  newnode->color[3] = 255;
  AddNode(newnode);
  
  newnode = new clutnode;
  newnode->height = -25;
  newnode->color[0] = 49;
  newnode->color[1] = 230;
  newnode->color[2] = 236;
  newnode->color[3] = 255;
  AddNode(newnode);
  
  newnode = new clutnode;
  newnode->height = -10;
  newnode->color[0] = 105;
  newnode->color[1] = 242;
  newnode->color[2] = 233;
  newnode->color[3] = 255;
  AddNode(newnode);
  
  newnode = new clutnode;
  newnode->height = 0;
  newnode->color[0] = 161;
  newnode->color[1] = 255;
  newnode->color[2] = 230;
  newnode->color[3] = 255;
  AddNode(newnode);
  
  newnode = new clutnode;
  newnode->height = 50;
  newnode->color[0] = 195;
  newnode->color[1] = 209;
  newnode->color[2] = 80;
  newnode->color[3] = 255;
  AddNode(newnode);
  newnode = new clutnode;
  
  newnode->height = 100;
  newnode->color[0] = 226;
  newnode->color[1] = 215;
  newnode->color[2] = 102;
  newnode->color[3] = 255;
  AddNode(newnode);
  newnode = new clutnode;
  newnode->height = 200;
  newnode->color[0] = 223;
  newnode->color[1] = 196;
  newnode->color[2] = 92;
  newnode->color[3] = 255;
  AddNode(newnode);
  newnode = new clutnode;
  newnode->height = 300;
  newnode->color[0] = 211;
  newnode->color[1] = 178;
  newnode->color[2] = 81;
  newnode->color[3] = 255;
  AddNode(newnode);
  
  newnode = new clutnode;
  newnode->height = 400;
  newnode->color[0] = 189;
  newnode->color[1] = 150;
  newnode->color[2] = 60;
  newnode->color[3] = 255;
  AddNode(newnode);
  
  newnode = new clutnode;
  newnode->height = 500;
  newnode->color[0] = 163;
  newnode->color[1] = 127;
  newnode->color[2] = 47;
  newnode->color[3] = 255;
  AddNode(newnode);

  newnode = new clutnode;
  newnode->height = 600;
  newnode->color[0] = 153;
  newnode->color[1] = 118;
  newnode->color[2] = 43;
  newnode->color[3] = 255;
  AddNode(newnode);

  newnode = new clutnode;
  newnode->height = 700;
  newnode->color[0] = 143;
  newnode->color[1] = 110;
  newnode->color[2] = 39;
  newnode->color[3] = 255;
  AddNode(newnode);

  newnode = new clutnode;
  newnode->height = 800;
  newnode->color[0] = 135;
  newnode->color[1] = 104;
  newnode->color[2] = 36;
  newnode->color[3] = 255;
  AddNode(newnode);

  newnode = new clutnode;
  newnode->height = 1000;
  newnode->color[0] = 128;
  newnode->color[1] = 98;
  newnode->color[2] = 33;
  newnode->color[3] = 255;
  AddNode(newnode);

  newnode = new clutnode;
  newnode->height = 1500;
  newnode->color[0] = 117;
  newnode->color[1] = 88;
  newnode->color[2] = 29;
  newnode->color[3] = 255;
  AddNode(newnode);

  newnode = new clutnode;
  newnode->height = 2000;
  newnode->color[0] = 98;
  newnode->color[1] = 71;
  newnode->color[2] = 24;
  newnode->color[3] = 255;
  AddNode(newnode);

  newnode = new clutnode;
  newnode->height = 4000;
  newnode->color[0] = 203;
  newnode->color[1] = 203;
  newnode->color[2] = 203;
  newnode->color[3] = 255;
  AddNode(newnode);

  newnode = new clutnode;
  newnode->height = 9000;
  newnode->color[0] = 220;
  newnode->color[1] = 220;
  newnode->color[2] = 220;
  newnode->color[3] = 255;
  AddNode(newnode);
}

void Clut::Draw(bool asQuad)
{
  int i;

  if(image.empty()) return;

  glViewport(0, 0, iwidth, 100);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, iwidth, 0, 100);
  glMatrixMode(GL_MODELVIEW);

  for(i = 0; i < 100; i++)
    {
      glRasterPos2i(0, i);
      glDrawPixels(iwidth, 1, GL_RGBA, GL_UNSIGNED_BYTE, image.raw());
    }
}

void Clut::SetMinMax()
{
  clutnode * cursor;
  float oldMinHeight = minheight;
  float oldMaxHeight = maxheight;

  cursor = clut;

  if (cursor != NULL)
    {
      minheight = (float)cursor->height;
      while(cursor->next != NULL)
	{
	  cursor = cursor->next;
	}
      maxheight = (float)cursor->height;
      range = maxheight - minheight;
    }
  if ((oldMinHeight != minheight || oldMaxHeight != maxheight) && built)
	  boundsChanged = GL_TRUE;
}

float Clut::GetMinheight()
{
  return minheight;
}

float Clut::GetMaxheight()
{
  return maxheight;
}

float Clut::GetHeightRange()
{
  return range;
}

void Clut::AddNode(clutnode * newnode)
{
  clutnode * cursor;
  clutnode * prev = NULL;

  newnode->next = NULL;
  cursor = clut;
  if (cursor == NULL)
  {
    clut = newnode;
    clut->next = NULL;
	clut->previous = NULL;
  }
  else
  {
    while(cursor != NULL && cursor->height < newnode->height)
    {
	  prev = cursor;
	  cursor = cursor->next;
    }
    if (cursor == clut)
    {
	  newnode->next = cursor;
	  newnode->previous = NULL;
	  cursor->previous = newnode;
	  clut = newnode;
    }
    else
    {
	  newnode->next = cursor;
	  newnode->previous = prev;
	  if (cursor)
		cursor->previous = newnode;
	  prev->next = newnode;
    }
  }
  if(newnode->next != NULL)
  {
    if(newnode->height == newnode->next->height)
	{
	  cursor = newnode->next;
	  newnode->next = newnode->next->next;
	  if (newnode->next)
		  newnode->next->previous = newnode;
	  delete cursor;
	}
  }

  SetMinMax();
}

// If we change the height of a node, might need to re-sort it in the list...
void Clut::ReAddNode(clutnode *node)
{
  clutnode * cursor;
  clutnode * prev;
  prev = NULL;

  if (clut->next != NULL)
    {
      cursor = clut;
      while(cursor->next != NULL && cursor != node)
	{
	  prev = cursor;
	  cursor = cursor->next;
	}
      if (cursor == node)
	{
	  if (prev == NULL)
	    {
	      clut = cursor->next;
		  cursor->previous = NULL;
	      AddNode(node);
	    }
	  else
	    {
	      prev->next = cursor->next;
		  cursor->previous = prev;
	      AddNode(node);
	    }
	}
    }
}

clutnode * Clut::Delete(clutnode *element)
{	
  clutnode * cursor;
  clutnode * prev;
  prev = NULL;
  cursor = clut;

  if (clut == element)
    {
      if (clut->next != NULL)
	{
	  clut = clut->next;
	  clut->previous = NULL;
	  delete element;
	}
    }
  else
    {
      while (cursor != element && cursor->next != NULL)
	{
	  prev = cursor;
	  cursor = cursor->next;
	}
      prev->next = cursor->next;
	  if (prev->next)
		  prev->next->previous = prev;
      delete element;
    }

  SetMinMax();
  return clut;
}

clutnode * Clut::GetNodePtr(float i)
{
  clutnode * cursor;

  cursor = clut;

  while(i > cursor->height && cursor->next != NULL)
    {
      cursor = cursor->next;
    }

  return cursor;
}

/*---------------------------------------------------------------------------*/
clutnode *Clut::GetNodePtrAtIndex(int i) //added by mdp 5/30/2008 for GeoCoastPilot
{
  clutnode * cursor, *lastKnownGood = NULL;
  bool findLast = false;

  if (i < 0) //get the last one if i is less than zero
  {
      findLast = true;
  }

  for(cursor = clut; cursor != NULL && (findLast || i > 0); i--)
  {
      lastKnownGood = cursor;
      cursor = cursor->next;
  }

  if (findLast)
      return lastKnownGood;
  else
      return cursor;
}

/*---------------------------------------------------------------------------*/
int Clut::GetIndexOf(clutnode *node) //added by mdp 5/30/2008 for GeoCoastPilot
{
    clutnode * cursor;
    int i = 0;

    for (cursor = clut; cursor != NULL; cursor = cursor->next)
    {
        if (cursor == node)
            return i;
        i++;
    }
    return -1; //not found!
}

/*---------------------------------------------------------------------------*/
unsigned char Clut::GetComponent(float i, int c)
{
  clutnode * cursor;

  cursor = clut;

  while(i > cursor->height && cursor->next != NULL)
    {
      cursor = cursor->next;
    }

  return cursor->color[c];

}

/*---------------------------------------------------------------------------*/
unsigned char Clut::GetInterpolatedComponent(float i, int c)
{
  clutnode * cursor;
  clutnode * prev;
  float range;
  
  cursor = clut;
  prev = cursor;
  
  if (cursor->height >= i)
      return cursor->color[c];

  while(i > cursor->height && cursor->next != NULL)
  {
    prev = cursor;
    cursor = cursor->next;
  }
  
  if (i >= cursor->height)
    return cursor->color[c];
  
  range = (float)(cursor->height - prev->height);
  cerr << "Range: " << range << " ratio: " << ((i-prev->height)/range)
          << ":" << ((cursor->height-i)/range) << "\t" << (int)prev->color[c]
          << ":" << (int)cursor->color[c] << "\n";
  return (unsigned char)(((cursor->height-i)/range)*prev->color[c])
          +(unsigned char)(((i-prev->height)/range)*cursor->color[c]);
}

/*---------------------------------------------------------------------------*/
//const Point4ub *Clut::GetColorAtValue(float val)
boost::optional<gz4d::ImageRGBA8::pixel_type> Clut::GetColorAtValue(float val)
{
	int index;

	if (!image)
		Build();

	index = (int)(twidth * (val - minheight) / range);
	if (index >= 0 && index <= twidth)
		//return &image[index];
            return image.GetRaster().GetRow(0)[index];
// 	else
// 		return NULL;
        return boost::optional<gz4d::ImageRGBA8::pixel_type>();
}
/*---------------------------------------------------------------------------*/
int Clut::GetNumEntries()
{
  clutnode * cursor;
  int n;

  for(n = 0, cursor = clut; cursor != NULL; n++, cursor = cursor->next);
 
  return n;
}

  
/*---------------------------------------------------------------------------*/
const Point4ub *Clut::GetColorAtIndex(int idx)
{
  clutnode * cursor;
  int i;

  for(i = 0, cursor = clut; cursor != NULL && i < idx; i++, 
	cursor = cursor->next);

  if(cursor == NULL)
    return NULL;
  else
    return &(cursor->color);
}

/*---------------------------------------------------------------------------*/
void Clut::Display(Tcl_Interp *interp, float min , float max, char * target)
{
  char tclstr[200];
  char rgb[7];
  clutnode * cursor;
  float lc, hc; // low corner, high corner of color element for canvas

  if (min == 0 && max == 0)
    {
      min = (float)(minheight - (0.1 * range));
      max = (float)(maxheight + (0.1 * range));
    }

  float crange = max - min;
	
  sprintf(tclstr,".cluted.%s.max configure -text %f",target,max);
  Tcl_Eval(interp, tclstr);

  sprintf(tclstr,".cluted.%s.min configure -text %f",target,min);
  Tcl_Eval(interp, tclstr);

  if (clut != NULL)
    {
      if (clut->next == NULL)
	{
	  // sprintf(tclstr,".cluted.clut create rectangle 0 10 50 500 -fill # -tags "
	}
      else
	{
	  cursor = clut;

	  sprintf(rgb,"%02x%02x%02x",cursor->color[0],cursor->color[1],cursor->color[2]);
	  hc = (float)(500.0-((((cursor->height)-min)/crange)*500.0));
	  lc = 500;

	  sprintf(tclstr,".cluted.%s.clut create rectangle 0 %f 50 %f -fill #%s -tags t%f",target,hc,lc,rgb,cursor->height);
	  Tcl_Eval(interp, tclstr);

	  while(cursor->next->next != NULL)
	    {
	      cursor = cursor->next;
	      lc = hc;
	      sprintf(rgb,"%02x%02x%02x",cursor->color[0],cursor->color[1],cursor->color[2]);
	      hc = (float)(500.0-((((cursor->height)-min)/crange)*500.0));
	      sprintf(tclstr,".cluted.%s.clut create rectangle 0 %f 50 %f -fill #%s -tags t%f",target,hc,lc,rgb,cursor->height);
	      Tcl_Eval(interp, tclstr);
	    }

	  cursor = cursor->next;

	  lc = hc;
	  sprintf(rgb,"%02x%02x%02x",cursor->color[0],cursor->color[1],cursor->color[2]);
	  hc = 0;

	  sprintf(tclstr,".cluted.%s.clut create rectangle 0 %f 50 %f -fill #%s -tags t%f",target,hc,lc,rgb,cursor->height);
	  Tcl_Eval(interp, tclstr);

	}
    }
}

/*---------------------------------------------------------------------------*/
void Clut::StripNewLine(char *line)
{
  while(line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\f' || 
	line[strlen(line) - 1] == '\r' )
    {
      line[strlen(line) - 1] = 0;
    }
}

/*---------------------------------------------------------------------------*/
void Clut::ModifyExtents(float scale, float shift)
{
  clutnode * cursor = NULL;
  for(cursor = clut; cursor != NULL; cursor = cursor->next)
  {
    cursor->height = (cursor->height*scale)+shift;
  }
  SetMinMax();
  Build();
}

/*---------------------------------------------------------------------------*/
void Clut::CreateRange(float max, float min, float stepsize)
{
  clutnode * templist;
  clutnode * cursor;
  float height = min;
  
  templist = new clutnode;
  cursor = templist;
  cursor->next=NULL;
  cursor->height = height;
  cursor->color[0]=  GetInterpolatedComponent(height,0);
  cursor->color[1]=  GetInterpolatedComponent(height,1);
  cursor->color[2]=  GetInterpolatedComponent(height,2);
  cursor->color[3]=  GetInterpolatedComponent(height,3);
	   
  
  for(height = min+stepsize; height <= max; height += stepsize)
  {
      cerr << height << "\n";
      cursor->next = new clutnode;
      cursor = cursor->next;
      cursor->next=NULL;
      cursor->height = height;
	  cursor->color[0]=  GetInterpolatedComponent(height,0);
  	  cursor->color[1]=  GetInterpolatedComponent(height,1);
   	  cursor->color[2]=  GetInterpolatedComponent(height,2);
   	  cursor->color[3]=  GetInterpolatedComponent(height,3);
  }

  while(templist != NULL)
  {
    cursor = templist->next;
    AddNode(templist);
    templist = cursor;
  }
    
    
  SetMinMax();
  Build();
    
}


void Clut::ClearRange(float max, float min)
{
    clutnode * cursor = clut;
    clutnode * prev = clut;

    while(cursor)
    {
        if (cursor->height >= min && cursor->height <= max)
        {
            if (cursor == clut)
            {
                clut = clut->next;
                delete cursor;
                cursor = clut;
                prev = clut;
            }
            else
            {
                prev->next = cursor->next;
                delete cursor;
                cursor = prev->next;
            }
        }
        else
        {
            prev = cursor;
            cursor = cursor->next;
        }
    }
    SetMinMax();
    Build();
}

void Clut::Clear()
{
    clutnode * cursor;
    
    while(clut != NULL)
    {
        cursor = clut->next;
        delete clut;
        clut = cursor;
    }
    
    cursor = new clutnode;
    cursor->previous = cursor->next = NULL;
    cursor->height = 0;
    cursor->color[0] = 0;
    cursor->color[1] = 0;
    cursor->color[2] = 255;
    cursor->color[3] = 255;
    
    AddNode(cursor);
    
    SetMinMax();
    Build();

}
