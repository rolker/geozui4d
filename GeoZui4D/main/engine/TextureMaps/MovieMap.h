/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

/*
	Derived from Don House's ImageMap by Roland, Dec 19, 2001 
*/

#ifndef MOVIEMAP_H
#define MOVIEMAP_H

#include <string.h>

#include <smpeg/smpeg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "TextureMap.h"

class MovieMap: public TextureMap
{
public:
  MovieMap();
  virtual ~MovieMap();

  void Init();			// load deflt or named file
  int Load();
  int Save(std::string const &fn = bfs::path());

  bool Enable(const Point3d offset);	// turn texture map on
  bool Enable();										// turn texture map on
  void Disable();		// turn this texture map off

  void Build();			// build the OpenGL texture
  void Destroy();		// delete the internal data

  void CreateDefault();		// make the default texture

  void Print() const;
  void Draw(bool asQuad = false);
  void Display(Tcl_Interp *interp, float min , float max, char * target);
  
  int TextureIdle(double time);
  void Play();
  void Stop();
  void Pause();
  void Loop(GLboolean repeat = GL_TRUE);

  GLboolean IsPlaying() {return play;}
  void InitDims();
protected:
  int repeat;			//should the image repeat? (mdp 9/27/2001)

//  FILE * mpeg;
//  ImageDesc imgdesc;
  SMPEG* mpeg;
  SMPEG_Info mpeg_info;
  SDL_Surface* surface;

  //Point4ub * frame;
  int buffer;
  
  int play;

  float eastref, northref;	// georeference of image upper lefthand corner
  
//  static  int SDLInitFlag;
};

#endif
