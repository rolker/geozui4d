// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include "interactors/SDV_Cursor.h"
#include <GL/glu.h>
#include "engine/Window3D.h"
#include "objects/CursorObject.h"
#include "interactors/Pickable.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
/** 
 *This sets up the cursor images in memory. The cursor consists of two images:
 *The white cursor, and it's black outline. OpenGL draws bitmaps from the
 *bottom up, so the images are read into memory so that the bottom comes first
 */
SDV_Cursor::SDV_Cursor(int * sf)
:Interactor("oneEyedCursor")
{
	stereo_flag = sf;
	trackCursor = GL_FALSE;

    int i;
    GLubyte white[48] = { 0x00,0x00,
                       0x00,0x00,
                       0x40,0x00,
                       0x60,0x00,
                       0x70,0x00,
                       0x78,0x00,
                       0x7c,0x00,
                       0x7e,0x00,
                       0x7f,0x00,
                       0x7f,0x80,
                       0x7f,0xc0,
                       0x7f,0xe0,    
                       0x7f,0xf0,
                       0x7f,0xf8,
                       0x7f,0xe0,
                       0x7f,0x80,
                       0x79,0xc0,
                       0x61,0xc0,
                       0x00,0xe0,
                       0x00,0xe0,
                       0x00,0x70,
                       0x00,0x70,
                       0x00,0x20,
                       0x00,0x00};

    GLubyte black[48] = { 0xc0,0x00,
                       0xe0,0x00,
                       0xb0,0x00,
                       0x98,0x00,
                       0x8c,0x00,
                       0x86,0x00,
                       0x83,0x00,
                       0x81,0x80,
                       0x80,0xc0,
                       0x80,0x60,
                       0x80,0x30,
                       0x80,0x18,    
                       0x80,0x0c,
                       0x80,0x06,
                       0x80,0x18,
                       0x80,0x60,
                       0x86,0x20,
                       0x9a,0x20,
                       0xe1,0x10,
                       0x81,0x10,
                       0x00,0x88,
                       0x00,0x88,
                       0x00,0x58,
                       0x00,0x60};

    for(i = 0; i < 24; i++)
    {
        whitecursor[i*2] = white[(23-i)*2];
        whitecursor[i*2+1] = white[((23-i)*2)+1];
        blackcursor[i*2] = black[(23-i)*2];
        blackcursor[i*2+1] = black[((23-i)*2)+1];
    }
	memset(savedbg,0,1536);
	savedx = savedy = 0;
	bufferSaved = 0;
}

/*---------------------------------------------------------------------------*/
/** 
 *This receives mouse movement events from the togl widgets. The previous cursor
 *is deleted, and a new one is drawn.
 *
 */
int SDV_Cursor::HandleInput(unsigned int inputType, InputInfo * info)
{
	if (*oneEyedFlag == 1 || trackCursor)
	{
		if (inputType & EnumeratedAction)
		{
			if (trackCursor && inputType == Object::objectSelectedEvent())
			{
				ToglWindow::toggleMouseTracking();
				Window3D::linkageCursor->updateUsing(NULL);
				Window3D::linkageCursor->coupleTo((PosableStateBase*)info->val.p);
			}
			return PASS_EVENT_THROUGH;
		}
		else if (inputType & (MouseEnter | MouseLeave | MouseMotion))
		{
			x = info->x.i;
			y = info->y.i;
			window = info->window;
			if (window)
			{
				if (*oneEyedFlag == 1)
				{
					wwidth = Togl_Width(window->GetTogl());
					wheight = Togl_Height(window->GetTogl());
					if ((inputType & MouseEnterMask) == 0) //Leave or move
						RemoveCursor();
					if ((inputType & MouseLeaveMask) == 0) //Enter or move
						DrawCursor();
				}
				if (trackCursor)
					Window3D::linkageCursor->updateUsing(info);
			}
		}
    }
    return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/** 
 *This first saves a copy of the backgroud behind the location where the cursor
 *is going to apear. The cursor and it's outline bitmaps are then drawn.
 */

void SDV_Cursor::DrawCursor(ToglWindow *targetWindow)
{
	if (targetWindow && (targetWindow != window))
		return;

	savedx = x;
    savedy = y;
    
	Set2DMode();
    
	Resample();

    glColor3f(1.0,1.0,1.0);
    glRasterPos2i(savedx,wheight-savedy);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBitmap (16, 24, 0.0, 24.0, 0.0, 0.0, whitecursor);

    glColor3f(0.0,0.0,0.0);
    glRasterPos2i(savedx,wheight-savedy);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBitmap (16, 24, 0.0, 24.0, 0.0, 0.0, blackcursor);

   	Set2DMode(GL_FALSE);

}

/*---------------------------------------------------------------------------*/
/** 
 *This removes the cursor by redrawing the saved background patch.
 */
void SDV_Cursor::RemoveCursor(ToglWindow *targetWindow)
{
	if (targetWindow && (targetWindow != window))
		return;

	Set2DMode();

	if (savedy+24 > wheight)
		glRasterPos2i(savedx,0);
	else
		glRasterPos2i(savedx,wheight-(savedy+24));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glDrawPixels(16,24,GL_RGBA,GL_UNSIGNED_BYTE,savedbg);

	Set2DMode(GL_FALSE);
}

/*---------------------------------------------------------------------------*/
/** 
 *This sets in which eye the cursor is drawn in for stereo viewing. Uses 
 *the definitions in SDV_Types.h of LEFT_EYE and RIGHT_EYE
 */
void SDV_Cursor::SetDominantEye(int de)
{
	dominantEye = de;
}

int SDV_Cursor::GetDominantEye()
{
	return dominantEye;
}

/*---------------------------------------------------------------------------*/
/** 
 *This sets in which eye the cursor is drawn in for stereo viewing. Uses 
 *the definitions in SDV_Types.h of LEFT_EYE and RIGHT_EYE
 */
void SDV_Cursor::SetOneEyedFlagPtr(int * oef)
{
	oneEyedFlag = oef;
}

/*---------------------------------------------------------------------------*/
/** 
 *	If we haven't already turned 2D mode on/off, do it.  Bump the bufferSaved
 *	counter to reflect how many times we've been told to do each.
 */
void SDV_Cursor::Set2DMode(GLboolean on)
{
	if (on && !bufferSaved++)
	{
	    glGetIntegerv(GL_READ_BUFFER,&savedreadbuffer);// Being nice and saving previous values
	    glGetIntegerv(GL_DRAW_BUFFER,&saveddrawbuffer);// Being nice and saving previous values

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0.0,wwidth,0.0,wheight);
    
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		if(dominantEye == LEFT_EYE)
			glDrawBuffer(GL_FRONT_LEFT);
		else
			glDrawBuffer(GL_FRONT_RIGHT);
	}
	else if (!on && !--bufferSaved)
	{
		glFlush();
		glPopMatrix();
    
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glReadBuffer(savedreadbuffer);
		glDrawBuffer(saveddrawbuffer);
	}
}

/*---------------------------------------------------------------------------*/
/** 
 *	Update our notion of what is under the cursor.
 */
void SDV_Cursor::Resample(ToglWindow *targetWindow)
{
	if (targetWindow && (targetWindow != window))
		return;

	Set2DMode();

    glRasterPos2i(savedx,wheight-savedy);
	if(*stereo_flag == 1)
	{
		if(dominantEye == LEFT_EYE)
			glReadBuffer(GL_FRONT_LEFT);
		else
			glReadBuffer(GL_FRONT_RIGHT);
	}
	else
		glReadBuffer(GL_FRONT);

	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	if (savedy+24 > wheight)
		glReadPixels(savedx, 0, 16,24,GL_RGBA,GL_UNSIGNED_BYTE,savedbg);
	else
		glReadPixels(savedx, wheight-(savedy+24), 16,24,GL_RGBA,GL_UNSIGNED_BYTE,savedbg);

	Set2DMode(GL_FALSE);
}
