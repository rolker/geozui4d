/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include <math.h>
#include <iostream>
#include <string.h>
#include "MovieMap.h"
#include <boost/filesystem.hpp>
#include <GL/glu.h>

#define DEFAULTSIZE	1000
#define ALPHAMAX	0.75	// max alpha channel value for image

using namespace std;

MovieMap::MovieMap()
{
  type = MOVIETEXTURE;
  repeat = GL_FALSE;
  surface = NULL;
  twidth = 0;
  theight = 0;
  iwidth = 0;
  iheight = 0;
  play = 0;
  mpeg = NULL;
  
  // set up the reader to not look for BEGIN line for an MOVIETEXTURE
  SetTypeAndVersion("");
  
      if( Mix_OpenAudio(44100, AUDIO_S16, 2,4096))
      {
      	cerr << "Unable to open audio!\n";
      }

//  CreateDefault();
  built = 0;
}

MovieMap::~MovieMap()
{
  Destroy();
}

void MovieMap::Init()
{
  Destroy();

  if (GetFilename().empty())
    {
      SetFilename(bfs::path("default.mgw"));
    }
	
  if (Load() != 0)
    {
      cerr << "Creating default image" << endl;
      CreateDefault();
    }
}

void MovieMap::InitDims()
{
	string imfilename = GetFilename().string();	// get file name, including path
	string ext = extension(GetFilename());

	if(ext == ".mgw")
		imfilename = imfilename.substr(0,imfilename.size()-3) + "mpg";

	mpeg = SMPEG_new(imfilename.c_str(), &mpeg_info, 0 );

	if(mpeg)
	{
		iheight=mpeg_info.height;
		iwidth=mpeg_info.width;
	}
}

int MovieMap::Load()
{
	int i;
	float rot1, rot2;
	float eastscale, northscale;
	char line[256];

	string imfilename = GetFilename().string();	// get file name, including path
	string ext = extension(GetFilename());

	if(ext == ".mgw")
	{
		imfilename = imfilename.substr(0,imfilename.size()-3) + "mpg";

		if(Open())
			return 1;

		i = fscanf(in, "%f %f %f %f %f %f", &eastscale, &rot1, &rot2, 
							&northscale, &eastref, &northref);
		if(i != 6 || rot1 != 0 || rot2 != 0)	// must have all 6 entries, and 0 rots
			return 1;
		//Following added by mdp 9/27/2001 to get repeating textures
		if (!feof(in))
		{
			fscanf(in, "%s", line);
			if (!strncmp(line, "REPEAT", 6))
				repeat = GL_TRUE;
		}

		Close();
	}
	else
	{
		// no metafile
		rot1 = rot2 = 0;
		eastscale = 1.0;
		northscale = 1.0;
		eastref = 0.0;
		northref = 0.0;
	}

	Destroy();		// make sure that we make a new image texture map
  
	mpeg = SMPEG_new(imfilename.c_str(), &mpeg_info, 0 );

	if(mpeg)
	{
		iheight=mpeg_info.height;
		iwidth=mpeg_info.width;

		std::cout << "Allocating " << iwidth << " x " << iheight << " surface" << std::endl;

		surface = SDL_AllocSurface( SDL_SWSURFACE,
				iwidth,
				iheight,
				32,
				0x000000FF,
				0x0000FF00,
				0x00FF0000,
				0xFF000000 );

		SMPEG_setdisplay(mpeg,surface,NULL,NULL);

		SMPEG_loop(mpeg,1);

//      SMPEG_play(mpeg);
     
      // record the horizontal and vertical image scales
		hsize = (float)fabs(eastscale) * iwidth;
		vsize = (float)fabs(northscale) * iheight;
		hOffsetScale = (float(twidth) / iwidth) * hsize;
		vOffsetScale = -(float(theight) / iheight) * vsize;

      // adjust the north and east georefs as needed to lower lefthand corner
		if(northscale < 0)
			northref -= vsize;
		if(eastscale < 0)
			eastref -= hsize;
		loaded = 1;  
      return 0;
	}
	else
	{
		image = NULL;
		return 1;
	}
}

int MovieMap::Save(std::string const &fn)
{
  return 1;
}

bool MovieMap::Enable(const Point3d offset)
{
  if(surface == NULL)		// forget it if image failed to be created
    return false;


  glBegin( GL_LINES ); //These lines are here to make textures work with my
  glEnd();			   //Wildcat card.  Otherwise, texture gets munged. (mdp)

 
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, textureID);
  if(!built)		// create OpenGL texture just in time
    Build();

  // generate texture coords automatically in both s and t directions
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  GLfloat splane[] = {(float)(1 / hOffsetScale), 0.0, 0.0, 0.0};
  glTexGenfv(GL_S, GL_OBJECT_PLANE, splane);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  GLfloat tplane[] = {0.0, (float)(1 / vOffsetScale), 0.0, 0.0};
  glTexGenfv(GL_T, GL_OBJECT_PLANE, tplane);
       
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);

  glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    float xoffset = (float)((offset[0] - eastref) / hOffsetScale);
    float yoffset = (float)((offset[1] - northref) / vOffsetScale);
    glTranslatef(xoffset, yoffset, 0);
    
  glBegin( GL_LINES ); //These lines are here to make textures work with my
  glEnd();			   //Wildcat card.  Otherwise, texture gets munged. (mdp)

  glMatrixMode(GL_MODELVIEW);
  return true;
}

bool MovieMap::Enable()
{
    
  if(surface == NULL)		// forget it if image failed to be created
    return false;
    
  if(!built)		// create OpenGL texture just in time
    Build();

  glBegin( GL_LINES ); //These lines are here to make textures work with my
  glEnd();			   //Wildcat card.  Otherwise, texture gets munged. (mdp)

  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);

  glEnable(GL_TEXTURE_2D);
  if(!built)		// create OpenGL texture just in time
    Build();

  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);
  return true;
}

void MovieMap::Disable()
{
  if(surface == NULL)		// forget it if image failed to be created
    return;

  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_2D);

  glMatrixMode(GL_TEXTURE);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
}

void MovieMap::Build()
{
  int texturesize;
  GLint w, h;
  int m, n;
  int i;
  buffer = 2;

  if (repeat)
	  buffer = 0;

  // find minimum power of 2 > each dimension (plus border)
  for(theight = 1, m = 0; theight < iheight + buffer; theight *= 2, m++);
  for(twidth = 1, n = 0; twidth < iwidth + buffer; twidth *= 2, n++);
  
  // first reduce dimensions to a supported size if necessary
  do{
    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, twidth, theight,
		 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);	// include border
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    if(w == 0 || h == 0){
      twidth /= 2; n--;
      theight /= 2; m--;
    }
    texturesize = (theight < twidth? theight: twidth);
  }while((w == 0 || h == 0) && texturesize > 64);

  widthRatio = (float)iwidth/(float)twidth;
  heightRatio = (float)iheight/(float)theight;
  
  if (!textureID) //texture creation moved here because there was flakiness when glGenTextures was called from a non-rendering thread (mdp 2/3/2005)
	glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  image = new Point4ub[twidth*theight];
  for(i=0;i<twidth*theight;i++)
  {
      image[i][0]=127;
      image[i][1]=127;
      image[i][2]=127;
	  image[i][3]=127;
  }

  if(image != NULL)
    {
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, image);


      // the texture should not be interpolated from the mip-map
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      // the image texture should not repeat over the surface
	  if (repeat)
	  {
	      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	  }
	  else
	  {
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	  }
    }

  built = 1;
}

int MovieMap::TextureIdle(double time)
{
    SMPEGstatus status;
    if(built && play)
    {
    
	    status = SMPEG_status(mpeg);
		if (status != SMPEG_PLAYING)
			return 0;

        glBindTexture(GL_TEXTURE_2D, textureID);
        
        if(image != NULL)
        {
	        glTexSubImage2D(GL_TEXTURE_2D, 0,0,0, iwidth, iheight, GL_RGBA,
	             GL_UNSIGNED_BYTE, surface->pixels);
        }
        return 1;
    }
    return 0;
}

void MovieMap::Play()
{
	if (mpeg_info.has_audio)
	{
		SMPEG_enableaudio(mpeg, 0);
		SDL_AudioSpec audiofmt;
		Uint16 format;
		int freq, channels;

		/* Tell SMPEG what the audio format is */
		Mix_QuerySpec(&freq, &format, &channels);
		audiofmt.format = format;
		audiofmt.freq = freq;
		audiofmt.channels = channels;
		SMPEG_actualSpec(mpeg, &audiofmt);

		/* Hook in the MPEG music mixer */
		Mix_HookMusic(SMPEG_playAudioSDL, mpeg);
		SMPEG_enableaudio(mpeg, 1);
	}
	SMPEG_play(mpeg);
    
	play = 1;
}

void MovieMap::Stop()
{
	SMPEG_stop(mpeg);
    SMPEG_rewind(mpeg);
	play = 0;
}

void MovieMap::Pause()
{
	SMPEG_stop(mpeg);
	play = 0;
}

void MovieMap::Loop(GLboolean repeat)
{
	SMPEG_loop(mpeg, repeat);
}

void MovieMap::Destroy()
{
  delete []image;
  image = NULL;

  if(mpeg)
  {
    SMPEG_stop(mpeg);
	SMPEG_delete(mpeg);
	mpeg = NULL;
  }

//  glDeleteTextures(1, &textureID);
//  glGenTextures(1, &textureID);
  built = 0;  
}

//
// as default prepare to create a 512 x 512 mid-grey mipmap, with alpha = 0.5
// with DEFAULTSIZE as metric
//
void MovieMap::CreateDefault()
{
 // int i;

  iwidth = iheight = 510;
  hsize = vsize = DEFAULTSIZE;
  eastref = northref = 0;

  
  Build();
}

void MovieMap::Print() const
{
  cout << "image dimensions: [" << iheight << ", " << iwidth << "] ";
  cout << "texture dimensions: [" << theight << ", " << twidth << "] ";
}

void MovieMap::Draw(bool asQuad)
{
  if(image == NULL) return;

  glViewport(0, 0, iwidth, iheight);
  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, iwidth, 0, iheight);
  glMatrixMode(GL_MODELVIEW);

  glRasterPos2i(0, 0);
  glDrawPixels(iwidth, iheight, GL_RGB, GL_UNSIGNED_BYTE, image);
}

void MovieMap::Display(Tcl_Interp *interp, float min , float max, char * target)
{
}

