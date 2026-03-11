// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef PBUFFER_H
#define PBUFFER_H

#ifdef WIN32
#include<windows.h>
#include<wingdi.h>
#include <GL/gl.h>
#include <GL/glut.h>
//#include "glati.h"
//#include "wglati.h"
#else
#include<gz4d/glo.h>
#include<GL/glx.h>
#endif

class PBuffer
{
public:
	PBuffer(unsigned int w,unsigned int h);
	~PBuffer();
	void MakeContextCurrent();
	void RestoreSavedContext();
	unsigned int getWidth(){return width;};
	unsigned int getHeight(){return height;};
	void SwapBuffers();
	int bind();
	void release();
#ifdef WIN32
	void printError(const char *msg);
	int Init();
	void createBuffer(unsigned int w, unsigned int h);
#endif

private:



#ifdef WIN32

	HDC hdc;
	HGLRC hglrc;

	// allows us to foolproof the interface to prevent using the pbuffer before
	// it is initialized
	static bool pbufferInitialized;
	static bool pbufferSystemInitialized;

#if 0//without GLee, following code is gibberish

	// handle, device context and render context for pbuffer buffer
	HPBUFFERARB hPBuffer;
	HDC hPBufferDC;
	HGLRC hPBufferRC;

	// pbuffer function prototypes
	static PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB;
	static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	static PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB;
	static PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB;
	static PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB;
	static PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB;
	static PFNWGLQUERYPBUFFERARBPROC wglQueryPbufferARB;
	static PFNWGLBINDTEXIMAGEARBPROC wglBindTexImageARB;
	static PFNWGLRELEASETEXIMAGEARBPROC wglReleaseTexImageARB;
	static PFNWGLSETPBUFFERATTRIBARBPROC wglSetPbufferAttribARB;
#endif
#else
	//X offscreeen rendering stuff
	Display * xdisplay;
	GLXFBConfig *fbconfig;
	GLXPbuffer pbuffer;
	GLXContext context;
	GLXContext savedcontext;
	GLXDrawable saveddrawable;
	Display * saveddisplay;
	int xscreen;
#endif

	GLuint pbufferTextureID;
	unsigned int width,height;

};


#endif
