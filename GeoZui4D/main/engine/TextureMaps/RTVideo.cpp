/* Copyright (c) 2005 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

//#ifndef WIN32
#ifdef LINUX

#include"RTVideo.h"

#include<iostream>
#include<fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/stat.h>

#include "MainWindow.h"
#include"engine/ToglWindow.h"

extern MainWindow * mainWindow;

extern Slog gzlog;

static int RTVideo_Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

RTVideo::RTVideo()
{
  type = RTVIDEOTEXTURE;
  twidth = 0;
  theight = 0;
  iwidth = 0;
  iheight = 0;
	
	surface = NULL;
	
	initialized = 0;
  repeat = GL_TRUE;

}

RTVideo::~ RTVideo()
{
}

void RTVideo::Init()
{
	if(initialized)
		return;

	LogState initScope(&gzlog,"RTVideo::Init",TRACE);
	
	struct stat st;
	
	if(stat ("/dev/video",&st) == -1)
	{
		gzlog << "Problem with /dev/video: " << strerror (errno) << endl;
		return;
	}
	
	if (!S_ISCHR (st.st_mode))
	{
		gzlog << "/dev/video is no device" << endl;
		return;
	}
	
	fd = open("/dev/video",O_RDWR);
	if(fd == -1)
	{
		gzlog << "Can't open /dev/video: " << strerror(errno) << endl;
		return;
	}
	
	if(ioctl(fd,VIDIOC_QUERYCAP, &cap)==-1)
	{
		gzlog << "Error querying capabilities" << endl;
		return;
	}
	
	if(!cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
	{
		gzlog << "Device not able to capture video" << endl;
		return;
	}
	
	if(!cap.capabilities &V4L2_CAP_READWRITE)
	{
		gzlog << "Read/Write IO type not supported" << endl;
		return;
	}

/*	char tclstr[200];
	sprintf(tclstr,"source [file join $appdir \"RTVideo.tcl\"]");
	mainWindow->Tcl_Eval(tclstr);
	
	sprintf(tclstr,"LabelFrame $RTVideo::RTVideo.inputFrame -text Input:; pack $RTVideo::RTVideo.inputFrame -fill x");
	mainWindow->Tcl_Eval(tclstr);
	
	sprintf(tclstr,"ComboBox $RTVideo::RTVideo.inputFrame.f.input -editable false -modifycmd {rtvideo setInput [$RTVideo::RTVideo.input getvalue]}; pack $RTVideo::RTVideo.inputFrame.f.input");
	mainWindow->Tcl_Eval(tclstr);*/
	
	int savedint;
	
	if(ioctl(fd,VIDIOC_G_INPUT, &savedint)==-1)
	{
		gzlog << "Error retriving current input" << endl;
		return;
	}
	
	cerr << savedint << endl;
	
	memset(&input,0,sizeof(input));

/*	char substr[200];
	sprintf(tclstr,"$RTVideo::RTVideo.inputFrame.f.input configure -values {");
	
	input.index = 0;
	while(ioctl(fd,VIDIOC_ENUMINPUT,&input)==0)
	{
		sprintf(substr," %s ",input.name);
		strcat(tclstr,substr);
		input.index++;
	}
	strcat(tclstr,"}");
	
	mainWindow->Tcl_Eval(tclstr);
	
	sprintf(tclstr,"$RTVideo::RTVideo.inputFrame.f.input setvalue @%i",savedint);
	mainWindow->Tcl_Eval(tclstr);
	
	memset(&queryctrl,0,sizeof(queryctrl));
	
	queryctrl.id = V4L2_CID_BASE;
	
	v4l2_control ctrl;
	
	
	
	while(ioctl(fd,VIDIOC_QUERYCTRL,&queryctrl)==0)
	{
		if(!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED))
		{
			if(queryctrl.type == V4L2_CTRL_TYPE_INTEGER)
			{
				sprintf(tclstr,"LabelFrame $RTVideo::RTVideo.ctrl%iFrame -text %s; pack $RTVideo::RTVideo.ctrl%iFrame -fill x",queryctrl.id,queryctrl.name,queryctrl.id);
				mainWindow->Tcl_Eval(tclstr);
				sprintf(tclstr,"scale $RTVideo::RTVideo.ctrl%iFrame.f.scale -orient horizontal -from %i -to %i -resolution %i -showvalue false -command {rtvideo ctrl %i}; pack $RTVideo::RTVideo.ctrl%iFrame.f.scale -fill x",queryctrl.id,queryctrl.minimum,queryctrl.maximum,queryctrl.step,queryctrl.id,queryctrl.id);
				mainWindow->Tcl_Eval(tclstr);
				ctrl.id = queryctrl.id;
				if(ioctl(fd,VIDIOC_G_CTRL,&ctrl)==0)
				{
					sprintf(tclstr,"$RTVideo::RTVideo.ctrl%iFrame.f.scale set %i",ctrl.id, ctrl.value);
					mainWindow->Tcl_Eval(tclstr);
				}
			}
		}
		queryctrl.id++;
	}
	
	
	memset(&standard,0,sizeof(standard));
	standard.index = 0;*/
		
	//sprintf(tclstr,"LabelFrame $RTVideo::RTVideo.standardFrame -text Standard:; pack $RTVideo::RTVideo.standardFrame");
	//mainWindow->Tcl_Eval(tclstr);
	
	//sprintf(tclstr,"ComboBox $RTVideo::RTVideo.standard -editable false -modifycmd {rtvideo setStandard [$RTVideo::RTVideo.standard getvalue]}; pack $RTVideo::RTVideo.standard");
	//mainWindow->Tcl_Eval(tclstr);
	
	//sprintf(tclstr,"$RTVideo::RTVideo.standard configure -values {");
	
	//while (ioctl (fd, VIDIOC_ENUMSTD, &standard)==0)
	//{
	//	sprintf(substr," %s ",standard.name);
//		strcat(tclstr,substr);
//		standard.index++;
//	}
	
	//strcat(tclstr,"}");
	
//	mainWindow->Tcl_Eval(tclstr);

	v4l2_std_id stdid = V4L2_STD_PAL;

	if(ioctl(fd,VIDIOC_S_STD, &stdid)==-1)
	{
		gzlog << "Error setting standard" << endl;
		return;
	}
	
//	sprintf(tclstr,"$RTVideo::RTVideo.standard setvalue @%i",savedint);
//	mainWindow->Tcl_Eval(tclstr);
	
	memset(&fmt,0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR32;
	fmt.fmt.pix.width = 1024;
	fmt.fmt.pix.height = 768;
	
	if(ioctl(fd,VIDIOC_S_FMT,&fmt)==-1)
	{
		gzlog << "Error setting format" << endl;
		exit(1);
	}
	
	gzlog << "width: " << (int)(fmt.fmt.pix.width) << endl;
	gzlog << "height: " << (int)(fmt.fmt.pix.height) << endl;
	gzlog << "Pixel format: " << (char *)&fmt.fmt.pix.pixelformat << endl;

	iwidth = fmt.fmt.pix.width;
	iheight = fmt.fmt.pix.height;
	
	Tcl_CreateCommand(ToglWindow::interp, "rtvideo", RTVideo_Cmd,(ClientData) this, NULL);
	initialized = 1;
}


bool RTVideo::Enable(const Point3d offset)
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
//    float xoffset = (float)((offset[0] - eastref) / hOffsetScale);
//    float yoffset = (float)((offset[1] - northref) / vOffsetScale);
    float xoffset = (float)((offset[0]) / hOffsetScale);
    float yoffset = (float)((offset[1]) / vOffsetScale);
    glTranslatef(xoffset, yoffset, 0);
    
  glBegin( GL_LINES ); //These lines are here to make textures work with my
  glEnd();			   //Wildcat card.  Otherwise, texture gets munged. (mdp)

  glMatrixMode(GL_MODELVIEW);
  return true;
}

bool RTVideo::Enable()
{
    
  if(surface == NULL)		// forget it if image failed to be created
    return false;
    

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

void RTVideo::Disable()
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


void RTVideo::Destroy()
{
    TextureMap::Destroy();
  delete []surface;
  surface = NULL;

  built = 0;  
}

void RTVideo::Print() const
{
  cout << "image dimensions: [" << iheight << ", " << iwidth << "] ";
  cout << "texture dimensions: [" << theight << ", " << twidth << "] ";
}

void RTVideo::Draw(bool asQuad)
{
  if(surface == NULL) return;

  glViewport(0, 0, iwidth, iheight);
  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, iwidth, 0, iheight);
  glMatrixMode(GL_MODELVIEW);

  glRasterPos2i(0, 0);
  glDrawPixels(iwidth, iheight, GL_RGB, GL_UNSIGNED_BYTE, image);
}

void RTVideo::Display(Tcl_Interp *interp, float min , float max, char * target)
{
}

void RTVideo::InitDims()
{
	if(!initialized)
		Init();
}

void RTVideo::Build()
{
  int texturesize;
  int w, h;
  int m, n;
  int i;
  int buffer = 2;
  
  if(!initialized)
		Init();
	
	if(!initialized)
		return;

  if (repeat)
	  buffer = 0;

	surface = new unsigned char[fmt.fmt.pix.sizeimage];

	
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
	
	cerr << "Texture size: " << twidth << ", " << theight << endl;

  widthRatio = (float)iwidth/(float)twidth;
  heightRatio = (float)iheight/(float)theight;
  
  if (!textureID) //texture creation moved here because there was flakiness when glGenTextures was called from a non-rendering thread (mdp 2/3/2005)
	glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  image = new Point4ub[twidth*theight];
  for(i=0;i<twidth*theight;i++)
	{
		image[i][0]=0;
		image[i][1]=0;
		image[i][2]=192;
		image[i][3]=127;
	}

  if(image != NULL)
	{
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, image);


      // the texture should not be interpolated from the mip-map
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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

int RTVideo::TextureIdle()
{
	if(built)
	{
		if(read(fd,surface,fmt.fmt.pix.sizeimage)!=-1)
		{
			glBindTexture(GL_TEXTURE_2D, textureID);
			if(surface != NULL)
			{
				glTexSubImage2D(GL_TEXTURE_2D, 0,0,0, iwidth, iheight, GL_BGRA, GL_UNSIGNED_BYTE, surface);
			}
			return 1;
		}
	}
	return 0;
}

void RTVideo::setInput(int i)
{
	if(ioctl (fd, VIDIOC_S_INPUT, &i) == -1)
	{
		cerr << "Error setting input\n";
		return;
	}
}

void RTVideo::setStandard(int i)
{
	memset(&standard,0,sizeof(standard));
	standard.index = i;
	
	cerr << i << endl;
	
	if(ioctl (fd, VIDIOC_ENUMSTD, &standard) == 0)
	{
		cerr << standard.name << endl;
		cerr << standard.id << endl;
		//if(ioctl (fd, VIDIOC_S_STD, &standard.id) == -1)
		//{
			//cerr << "Error setting standard\n";
			//return;
		//}
	}
	else
	{
		cerr << "Error setting standard\n";
	}
}

void RTVideo::setControl(int c, int v)
{
	v4l2_control ctrl;
	
	ctrl.id = c;
	ctrl.value = v;
	
	if(ioctl (fd, VIDIOC_S_CTRL, &ctrl) == -1)
	{
		cerr << "Error setting ctrl\n";
	}

}

static int RTVideo_Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv)
{
	RTVideo * rtv = (RTVideo*)clientData;
	int i;

	if(argc < 2)
	{
		return TCL_ERROR;
	}

	if(strcmp(argv[1],"setInput")==0)
	{
		if(argc == 3)
		{
			rtv->setInput(atoi(argv[2]));
			return TCL_OK;
		}
		else
		{
			return TCL_ERROR;
		}
	}
	
	if(strcmp(argv[1],"setStandard")==0)
	{
		if(argc == 3)
		{
			rtv->setStandard(atoi(argv[2]));
			return TCL_OK;
		}
		return TCL_ERROR;
	}
	
	if(strcmp(argv[1],"ctrl")==0)
	{
		if(argc == 4)
		{
			rtv->setControl(atoi(argv[2]),atoi(argv[3]));
			return TCL_OK;
		}
		return TCL_ERROR;
	}
	
	for(i=0;i<argc;i++)
	{
		cerr << argv[i] << " ";
	}
	cerr << endl;
	cerr << "not yet supported\n";
	return TCL_OK;
}

#endif
