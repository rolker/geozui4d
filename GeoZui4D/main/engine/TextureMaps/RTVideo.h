/* Copyright (c) 2005 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef RTVIDEO_H
#define RTVIDEO_H

//#ifndef WIN32
#ifdef LINUX


#include"TextureMap.h"

#include <asm/types.h>          /* for videodev2.h */
//#include <linux/compiler.h>
#include <linux/videodev2.h>

class RTVideo: public TextureMap
{
public:
	RTVideo();
	~RTVideo();
	
	void Init();
	
	virtual int Load(){return 0;};
	virtual int Save(std::string const &fn = bfs::path()){return 0;};
  
	virtual bool Enable(const Point3d offset); // turn texture on
  virtual bool Enable(); // turn texture on, but without auto tex coord gen
  virtual void Disable();		// turn this texture map off

	virtual void Build();
	virtual void Destroy();
	
	virtual int TextureIdle();
	
	virtual void CreateDefault(){};	// make a default texture

  virtual void Print() const;	// text display of texture parameters
  virtual void Draw(bool asQuad = false);	// draw texture to OpenGL window
  virtual void Display(Tcl_Interp *interp, float min , float max, char * target); // Tcl draw of texture for GUI
  virtual void InitDims();
	
	void setInput(int i);
	void setStandard(int i);
	void setControl(int c, int v);

private:
	int fd;

	v4l2_capability cap;
	v4l2_input input;
	v4l2_standard standard;
	v4l2_std_id std_id;
	v4l2_format fmt;
	v4l2_queryctrl queryctrl;
	
	
	unsigned char *surface;
	
	int initialized;
	int repeat;
};

#endif

#endif

