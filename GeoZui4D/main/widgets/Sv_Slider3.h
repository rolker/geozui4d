// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

//
// Declaration of Sv_Slider3 class.
// Sv_Slider3 objects are used as part of the widget object to handle 
// rotation sliders in the set of widgets. 
//

#ifndef SV_SLIDER3_H
#define SV_SLIDER3_H

#include "Sv_Pointer.h"
#include "widgets/Sv_Standards.h"
//#include "glui_v2/glui.h"

#define NUM_VERTICES	73
#define AZIMUTH_TAB		2
#define ELEVATION_CONTROL	3

//Slider types
const GLshort	SV_SLIDER3_BAR = 0,
			      SV_SLIDER3_ARC = 1,
					SV_SLIDER3_WEDGE = 2;

const GLshort  SV_SLIDER3_POS_X = 0, 
			      SV_SLIDER3_NEG_X = 1, 
				   SV_SLIDER3_NO_X = 2;

const GLshort  SV_SLIDER3_POS_Y = 0,
			      SV_SLIDER3_NEG_Y = 1,
				   SV_SLIDER3_NO_Y = 2;

const GLshort  LIMIT_SET = 0, 
			      NO_LIMIT_SET = 1;

const GLfloat WIDTH = 20.0f;  // the thickness of the ring cw.

class Sv_Slider3 
	{
   public:
		Sv_Slider3( GLfloat theXCenter, GLfloat theYCenter, GLfloat theZCenter, // the center
						GLfloat theXOrientation, GLfloat theYOrientation, GLfloat theZOrientation,
						GLfloat theRadius, GLboolean showTip = GL_TRUE);
		~Sv_Slider3( void );

		//Member functions
		void  setIdentifier( GLshort typeident, GLshort theId );
		void  Draw( void );

		void  SetSlider3Pointer( GLfloat pointersize );
		void SetInactiveColor(short red, short green, short blue);
		void SetHighlightColor(short red, short green, short blue);
		void setRingColor(float red, float green, float blue);
		void setRingNotchColor(float red, float green, float blue);

		GLfloat getCurrentElevation();

		void  HighlightOn();
		void  HighlightOff();

		void setAzimuthVel( GLfloat xdelta);
		GLfloat  updateAzimuth( void );	
		void  resetAzimuth( void );

		void  setAzimuth( GLfloat val );  // needs a new name
		void  setElevation( GLfloat val );

		GLfloat getElevation(GLfloat ydelta);

	protected:
	
		void  CalculateAngles( void );

		GLfloat itsVertex[NUM_VERTICES][3];
		GLfloat itsNormal[NUM_VERTICES][3];

		GLint HighlightTab;
		float itsElevation;

		// slider variables
		GLshort itsWidgetTypeId;//use in picking, represents ALL sliders
		GLshort itsId;////use in picking, represent individual slider
		GLshort itsSliderType;
		GLfloat itsXCenter, itsYCenter, itsZCenter;
		
		GLfloat itsXOrientation, itsYOrientation, itsZOrientation;

		GLfloat itsRadius;//the radius of the arc, also affect the length of the bar slider
		GLint   itsAngle;//only apply to arc slider, the length of the arc in 1/10 degree increment
		GLfloat itsHeight;// the thickness

		// the azimuth control tab
		Sv_Pointer *azimuthTab;

		//GLfloat itsPixelRatio;
		GLshort PointerRed, PointerGreen, PointerBlue;
		GLshort PointerHighlightR, PointerHighlightG, PointerHighlightB;
		GLfloat ringRed, ringGreen, ringBlue;
		GLfloat ringNotchRed, ringNotchGreen, ringNotchBlue;

		GLshort itsPointerType;
		GLfloat azimuth;
		
		GLfloat azimuthVelocity;
		GLfloat itsVelocityRatio;

		GLboolean drawTip;
	};
#endif

