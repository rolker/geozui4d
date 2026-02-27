/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// SDV_Cursor.h

///////////////////////////////////////////////////////////////////////////////

#ifndef SDV_CURSOR_H
#define SDV_CURSOR_H

#include "engine/Interactor.h"
#include "SDV_types.h"
/** 
 * This is a software cursor that displays in one eye only. It allows easier
 * interaction in stereo using a 2d cursor in 3d space.
 * \author Roland Arsenault (rja)
 * \date 2000
 */
class SDV_Cursor : public Interactor
{
public:
    SDV_Cursor(int * sf);
    int HandleInput(unsigned int inputType, InputInfo *info);
    void DrawCursor(ToglWindow *targetWindow = NULL);
    void RemoveCursor(ToglWindow *targetWindow = NULL);
	void SetDominantEye(int de); // LEFT_EYE or RIGHT_EYE
	int  GetDominantEye(); // LEFT_EYE or RIGHT_EYE
	void SetOneEyedFlagPtr(int * oef);
	void Set2DMode(GLboolean on = GL_TRUE);
	void Resample(ToglWindow *targetWindow = NULL);

	GLboolean trackCursor;

private:
    int x,y; //latest cursor position
    int savedx, savedy; //position where cursor was drawn, to be able to erase it again
    int wwidth, wheight;
	int * stereo_flag; //points to the system's stero_flag
    ToglWindow * window;
    GLubyte blackcursor[48];
	GLubyte whitecursor[48];
    GLubyte savedbg[1536]; // 16*24 pixels, 4 ubytes/pixel
	int * oneEyedFlag;

    GLint savedreadbuffer, saveddrawbuffer;
	int bufferSaved;
};

#endif
