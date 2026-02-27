#include"PBuffer.h"
#include<iostream>

using namespace std;

#ifndef WIN32

PBuffer::PBuffer(unsigned int w, unsigned int h)
{

	width = w;
	height = h;
	pbufferTextureID = 0;

	int xvisattribs[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_MAX_PBUFFER_WIDTH, w,
		GLX_MAX_PBUFFER_HEIGHT, h,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DRAWABLE_TYPE,GLX_PBUFFER_BIT,
		None};
	                     
	int pbattribs[]={
		GLX_PBUFFER_WIDTH, w,
		GLX_PBUFFER_HEIGHT, h,
		GLX_NONE};
	                     
	int fbconfigCount;

	//xdisplay = XOpenDisplay(NULL);
	xdisplay = glXGetCurrentDisplay();
	if(!xdisplay)
	{
		cerr << "problem opening x display\n";
	}

	xscreen =  DefaultScreen(xdisplay);
  
	fbconfig = glXChooseFBConfig(xdisplay,xscreen,xvisattribs,&fbconfigCount);
//	fbconfig = glXGetFBConfigs(xdisplay,xscreen,&fbconfigCount);
//	fbconfig = glXChooseFBConfigSGIX(xdisplay,xscreen,xvisattribs,&fbconfigCount);
	
	if (fbconfig == NULL || fbconfigCount <= 0)
	{
		cerr << "problem choosing fb\n";
	}

	pbuffer = glXCreatePbuffer(xdisplay,fbconfig[0],pbattribs);
//	pbuffer = glXCreateGLXPbufferSGIX(xdisplay,fbconfig[0],w,h,pbattribs);
	
	if(pbuffer == None)
	{
		cerr << "problem creating pbuffer\n";
	}

	context = glXCreateNewContext(xdisplay,fbconfig[0],GLX_RGBA_TYPE,glXGetCurrentContext(),GL_TRUE);

//	context = glXCreateContextWithConfigSGIX(xdisplay,fbconfig[0],GLX_RGBA_TYPE,glXGetCurrentContext(),GL_TRUE);
	
	if (!context)
	{
		cerr << "problem creating context\n";
	}

	glXQueryDrawable(xdisplay,pbuffer,GLX_WIDTH,&width);
	glXQueryDrawable(xdisplay,pbuffer,GLX_HEIGHT,&height);

	cerr << "PBuffer of size " << width << " x " << height << " created\n";
}

PBuffer::~PBuffer()
{
  if(pbuffer != None)
    glXDestroyPbuffer(xdisplay, pbuffer);
}


void PBuffer::MakeContextCurrent()
{
	savedcontext = glXGetCurrentContext();
	saveddisplay = glXGetCurrentDisplay();
	saveddrawable = glXGetCurrentDrawable();
	glXMakeCurrent(xdisplay,pbuffer,context);
}

void PBuffer::RestoreSavedContext()
{
	glXMakeCurrent(saveddisplay,saveddrawable,savedcontext);
}

/*---------------------------------------------------------------------------*/
void PBuffer::SwapBuffers()
{
/*
   if (doubleBuffered) {
#if defined(WIN32)
      int res = SwapBuffers(togl->tglGLHdc);
      assert(res == TRUE);
#elif defined(X11)
      glXSwapBuffers( Tk_Display(togl->TkWin), Tk_WindowId(togl->TkWin) );
#elif defined(macintosh)
      aglSwapBuffers(togl->aglCtx);
#endif
   }
   else
*/
	glFlush();
}

int PBuffer::bind(void)
{
	// bind a texture object that we will associate a pbuffer with
	if (!pbufferTextureID)
		glGenTextures(1, &pbufferTextureID);
	glBindTexture(GL_TEXTURE_2D, pbufferTextureID);

	glReadBuffer(GL_FRONT);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0,0,width,height);

	return true;
}

void PBuffer::release(void)
{
}



#else  //-----above is unix, bellow is win32----------------------
//The code below was adapted from a paper on the ATI website
//written by Christopher Oat: "Rendering to an off-screen buffer with
//WGL_ARB_pbuffer".  Initial adaptation by mdp 2/27/2004. (Requires
//an implementation of OpenGL that supports all these functions).
//Finally implemented and tested by mdp 5/26/2005.

#include <stdlib.h>
#include <math.h>
//#include "globals.h"
#include "PBuffer.h"

// allows us to foolproof the interface to prevent using the pbuffer before
// it is initialized
bool PBuffer::pbufferInitialized = false;
bool PBuffer::pbufferSystemInitialized = false;

#if 0//need GLee
// pbuffer function prototypes
PFNWGLMAKECONTEXTCURRENTARBPROC PBuffer::wglMakeContextCurrentARB = NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC PBuffer::wglChoosePixelFormatARB = NULL;
PFNWGLCREATEPBUFFERARBPROC PBuffer::wglCreatePbufferARB = NULL;
PFNWGLDESTROYPBUFFERARBPROC PBuffer::wglDestroyPbufferARB = NULL;
PFNWGLGETPBUFFERDCARBPROC PBuffer::wglGetPbufferDCARB = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC PBuffer::wglReleasePbufferDCARB = NULL;
PFNWGLQUERYPBUFFERARBPROC PBuffer::wglQueryPbufferARB = NULL;
PFNWGLBINDTEXIMAGEARBPROC PBuffer::wglBindTexImageARB = NULL;
PFNWGLRELEASETEXIMAGEARBPROC PBuffer::wglReleaseTexImageARB = NULL;
PFNWGLSETPBUFFERATTRIBARBPROC PBuffer::wglSetPbufferAttribARB = NULL;
#endif

// ****************************************************************************
// pbufferInit: initializes a pbuffer of size width and height. this function
// takes care of finding a reasonable pixel format and then allocates the a
// chunk of video memory for use as an off screen buffer.
// ****************************************************************************
PBuffer::PBuffer(unsigned int w, unsigned int h)
{
	pbufferInitialized = false;

	if (!pbufferSystemInitialized)
	{
		if (!Init())
			return;
	}

	createBuffer(w, h);
}

/*---------------------------------------------------------------------------*/
void PBuffer::createBuffer(unsigned int w, unsigned int h)
{
	pbufferTextureID = 0;

	// keep track of the pbuffer size in case we have to create a new one
	// because of a display mode change.
	width = w;
	height = h;

#if 0//need GLee
	// get the current opengl device and render context
	hdc = wglGetCurrentDC();
	hglrc = wglGetCurrentContext();
	if (hdc == NULL)
	{
		printError("Error: unable to get GL Device Context");
		return;
	}
	if (hglrc == NULL)
	{
		printError("Error: unable to get GL Render Context");
		return;
	}

	// define the minimum pixel format requirements we will need for our pbuffer
	// a pbuffer is just like a frame buffer, it can have a depth buffer associated
	// with it and it can be double buffered.
	int attr[] =
	{
		WGL_SUPPORT_OPENGL_ARB, TRUE, // pbuffer will be used with gl
		WGL_DRAW_TO_PBUFFER_ARB, TRUE, // enable render to pbuffer
		WGL_BIND_TO_TEXTURE_RGBA_ARB, TRUE, // pbuffer will be used as a texture
		WGL_RED_BITS_ARB, 8, // at least 8 bits for RED channel
		WGL_GREEN_BITS_ARB, 8, // at least 8 bits for GREEN channel
		WGL_BLUE_BITS_ARB, 8, // at least 8 bits for BLUE channel
		WGL_ALPHA_BITS_ARB, 8, // at least 8 bits for ALPHA channel
		WGL_DEPTH_BITS_ARB, 24, // at least 24 bits for depth buffer
		WGL_DOUBLE_BUFFER_ARB, FALSE, // we don�t require double buffering
		0 // zero terminates the list
	};

	// choose a pixel format that meets our minimum requirements
	unsigned int count = 0;
	int pixelFormat;
	wglChoosePixelFormatARB(hdc,(const int*)attr, NULL, 1, &pixelFormat, &count);
	if(count == 0)
	{
		printError("Error: Unable to find an acceptable pixel format");
		return;
	}

	// set some pbuffer attributes so that we can use this pbuffer as a
	// 2D RGBA texture target
	int pAttrib[] =
	{
		WGL_TEXTURE_FORMAT_ARB, // our pbuffer will have texture format RGBA
		WGL_TEXTURE_RGBA_ARB,
		WGL_TEXTURE_TARGET_ARB, // of texture type GL_TEXTURE_2D
		WGL_TEXTURE_2D_ARB,
		0 // zero terminates the list
	};

	// allocate the pbuffer
	hPBuffer = wglCreatePbufferARB(hdc, pixelFormat, width, height, pAttrib);
	hPBufferDC = wglGetPbufferDCARB(hPBuffer);
	hPBufferRC = wglCreateContext(hPBufferDC);
	if(!hPBuffer)
	{
		printError("Error: Unable to create P-buffer");
		return;
	}

	//share the textures and display lists
	if(!wglShareLists(hglrc, hPBufferRC))
	{
		printError("Error: Unable to share textures and display lists with P-buffer");
		return;
	}

	/*
	int madeW, madeH;
	wglQueryPbufferARB(hPBuffer, WGL_PBUFFER_WIDTH_ARB, &madeW);
	wglQueryPbufferARB(hPBuffer, WGL_PBUFFER_HEIGHT_ARB, &madeH);

	cerr << "Attempted to make Pbuffer of " << width << "x" << height << "; created "<< madeW << "x" << madeH << endl;
	*/

#endif
	pbufferInitialized = true;
}

/*---------------------------------------------------------------------------*/
PBuffer::~PBuffer()
{
	// make sure the pbuffer has been initialized
	if (!(pbufferInitialized))
	{
		return;
	}
#if 0 //need GLee
	wglReleaseTexImageARB(hPBuffer, WGL_FRONT_LEFT_ARB);
	wglDeleteContext(hPBufferRC);
	wglReleasePbufferDCARB(hPBuffer, hPBufferDC);
	wglDestroyPbufferARB(hPBuffer);
#endif
	pbufferInitialized = false;
}

/*---------------------------------------------------------------------------*/
int PBuffer::Init()
{
#if 0 //need GLee
	// initialize extension entry points
	wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)
		wglGetProcAddress("wglMakeContextCurrentARB");
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
		wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)
		wglGetProcAddress("wglCreatePbufferARB");
	wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)
		wglGetProcAddress("wglDestroyPbufferARB");
	wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)
		wglGetProcAddress("wglGetPbufferDCARB");
	wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)
		wglGetProcAddress("wglReleasePbufferDCARB");
	wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)
		wglGetProcAddress("wglQueryPbufferARB");
	wglBindTexImageARB =
		(PFNWGLBINDTEXIMAGEARBPROC)wglGetProcAddress("wglBindTexImageARB");
	wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC)
		wglGetProcAddress("wglReleaseTexImageARB");
	wglSetPbufferAttribARB = (PFNWGLSETPBUFFERATTRIBARBPROC)
		wglGetProcAddress("wglSetPbufferAttribARB");
	// do some error checking to make sure function pointers were initialized correctly
	if (wglMakeContextCurrentARB == NULL)
	{
		printError ("Can't init wglMakeContextCurrentARB");
		return(false);
	}
	if (wglChoosePixelFormatARB == NULL)
	{
		printError ("Can't init wglChoosePixelFormatARB");
		return(false);
	}
	if (wglCreatePbufferARB == NULL)
	{
		printError ("Can't init wglCreatePbufferARB");
		return(false);
	}
	if (wglDestroyPbufferARB == NULL)
	{
		printError ("Can't init wglDestroyPbufferARB");
		return(false);
	}
	if (wglGetPbufferDCARB == NULL)
	{
		printError ("Can't init wglGetPbufferDCARB");
		return(false);
	}
	if (wglReleasePbufferDCARB == NULL)
	{
		printError ("Can't init wglReleasePbufferDCARB");
		return(false);
	}
	if (wglQueryPbufferARB == NULL)
	{
		printError ("Can't init wglQueryPbufferARB");
		return(false);
	}
	if (wglBindTexImageARB == NULL)
	{
		printError ("Can't init wglBindTexImageATIX");
		return(false);
	}
	if (wglReleaseTexImageARB == NULL)
	{
		printError ("Can't init wglReleaseTexImageATIX");
		return(false);
	}
	if (wglSetPbufferAttribARB == NULL)
	{
		printError ("Can't init wglSetPbufferAttribATIX");
		return(false);
	}
#endif
	pbufferSystemInitialized = true;
	return true;
}

/*---------------------------------------------------------------------------*/
void PBuffer::MakeContextCurrent()
{
	// make sure the pbuffer has been initialized
	if (!(pbufferInitialized))
	{
		return;
	}

#if 0 //need GLee
	// make sure we haven't lost our pbuffer due to a display mode change
	int flag = 0;
	wglQueryPbufferARB(hPBuffer, WGL_PBUFFER_LOST_ARB, &flag);
	if (flag != 0)
	{
		return;
	}

	// set read/write context to pbuffer
	wglMakeContextCurrentARB(hPBufferDC, hPBufferDC, hPBufferRC);

	// draw & read the front buffer of pbuffer
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);

	// resize view port. generally you'll want to set this to the
	// size of your pbuffer so that you render to the entire pbuffer
	// but there are cases where you might want to render to just a
	// sub-region of the pbuffer.
	glViewport(0.0, 0.0, width, height);
#endif
	return;
}

/*---------------------------------------------------------------------------*/
void PBuffer::RestoreSavedContext()
{
	// make sure the pbuffer has been initialized
	if (!(pbufferInitialized))
	{
		return;
	}
#if 0 //need GLee
	// make sure we haven't lost our pbuffer due to a display mode change
	int flag = 0;
	wglQueryPbufferARB(hPBuffer, WGL_PBUFFER_LOST_ARB, &flag);
	if (flag != 0)
	{
		return;
	}

	// set read/write context back to gl device
	wglMakeContextCurrentARB(hdc, hdc, hglrc);

	// draw to the back buffer of frame buffer
	// read from the front buffer of the frame buffer
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_FRONT);

	// reset the view port
	glViewport(0.0, 0.0, width, height);
#endif
	return;
}

// ****************************************************************************
// pbufferBind: bind the pbuffer to a texture. while the pbuffer is bound to a
// texture object, it is illegal to read or write to the pbuffer until it is
// released.
// ****************************************************************************
int PBuffer::bind(void)
{
	// make sure the pbuffer has been initialized
	if (!(pbufferInitialized))
	{
		return false;
	}

#if 0 //need GLee
	// make sure we haven't lost our pbuffer due to a display mode change
	int flag = 0;
	wglQueryPbufferARB(hPBuffer, WGL_PBUFFER_LOST_ARB, &flag);
	if (flag != 0)
	{
		createBuffer(width, height);
	}
	// bind a texture object that we will associate a pbuffer with
	if (!pbufferTextureID)
		glGenTextures(1, &pbufferTextureID);
	glBindTexture(GL_TEXTURE_2D, pbufferTextureID);

	// bind pbuffer to the currently bound texture object
	// while the pbuffer is bound to a texture it is illegal
	// to read or write from the pbuffer until it is released
	wglBindTexImageARB(hPBuffer, WGL_FRONT_LEFT_ARB);
#endif
	return true;
}

// ****************************************************************************
// pbufferRelease: release the pbuffer from the texture it was bound to. once
// the pbuffer is released you will be able to render to it again by calling
// pbufferEnable()
// ****************************************************************************
void PBuffer::release(void)
{
	// make sure the pbuffer has been initialized
	if (!(pbufferInitialized))
	{
		return;
	}
#if 0 //need GLee

	// release the pbuffer so that it can be rendered to again
	wglReleaseTexImageARB(hPBuffer, WGL_FRONT_LEFT_ARB);
#endif
	return;
}

/*---------------------------------------------------------------------------*/
void PBuffer::printError(const char *msg)
{
	cerr << "PBuffer " << msg << endl;
}

/*---------------------------------------------------------------------------*/
void PBuffer::SwapBuffers()
{
/*
   if (doubleBuffered) {
#if defined(WIN32)
      int res = SwapBuffers(togl->tglGLHdc);
      assert(res == TRUE);
#elif defined(X11)
      glXSwapBuffers( Tk_Display(togl->TkWin), Tk_WindowId(togl->TkWin) );
#elif defined(macintosh)
      aglSwapBuffers(togl->aglCtx);
#endif
   }
   else
*/
	glFlush();
}

#endif

