//
// Implementation of Sv_Slider3 class
//	This code has at least a 10 year history, Glenn Franck, Nhu Le, Colin Ware, and Matt Plumlee
// Major simplification in the fall of 2001
#include <math.h>
#include<gz4d/glo.h>
#include <GL/glut.h>
#include "engine/Scene3D.h"
#include "Sv_Slider3.h"

Sv_Slider3::Sv_Slider3( GLfloat theXCenter, GLfloat theYCenter, GLfloat theZCenter,
							  GLfloat theXOrientation, GLfloat theYOrientation, GLfloat theZOrientation,
							  GLfloat theRadius, GLboolean showTip)
{
	itsXCenter = theXCenter; 
	itsYCenter = theYCenter; 
	itsZCenter = theZCenter;
	
	itsXOrientation = theXOrientation; 
	itsYOrientation = theYOrientation; 
	itsZOrientation = theZOrientation;
	
	itsRadius = theRadius;
	itsSliderType = SV_SLIDER3_ARC;

	SetInactiveColor(250, 140, 160);
	SetHighlightColor(220, 70, 90);

	setRingColor(0.75f, 0.75f, 0.7f);
	setRingNotchColor(0.65f, 0.65f, 0.6f);
	
	itsWidgetTypeId = -1;
	itsId = -1;
	itsVelocityRatio = 0.5;

	azimuthVelocity = 0.0;
	itsElevation = 0.0;

	drawTip = showTip;

	HighlightTab = false;
	CalculateAngles();

	azimuthTab = new Sv_Pointer(itsRadius);  
}


Sv_Slider3::~Sv_Slider3(void)
{
	delete azimuthTab;

}

void Sv_Slider3::SetInactiveColor(short red, short green, short blue)
{
	PointerRed = red; 
	PointerGreen = green; 
	PointerBlue = blue;
}
		
void Sv_Slider3::SetHighlightColor(short red, short green, short blue)
{
	PointerHighlightR = red; 
	PointerHighlightG = green; 
	PointerHighlightB = blue;
}

void Sv_Slider3::setRingColor(float red, float green, float blue)
{
	ringRed = red;
	ringGreen = green;
	ringBlue = blue;
}

void Sv_Slider3::setRingNotchColor(float red, float green, float blue)
{
	ringNotchRed = red;
	ringNotchGreen = green;
	ringNotchBlue = blue;
}

void Sv_Slider3::HighlightOn()
{
	HighlightTab = true;
}


void Sv_Slider3::HighlightOff()
{
	HighlightTab = false;
}


////////////////////////////////////////////////////////////////////////////////
// Function  : SetIdentifier
// Arguments : 
// Returns   : 
// Purpose   : 
//  Allows the user to specify an theId which will be loaded
//  onto the name stack to allow identification of the slider during
//  the 'pick' or 'select' operations. There will be two separate names
//  for the slider parts (slider bar and pointer), which will be identified
//  by a set of three shorts:
//  1) 'typeident', specified by the user; this
//	likely will be used to identify that a slider was the object
//	picked or selected. This value should be the same, and unique
//	for all sliders only. The default value is -1.
//  2) 'theId', also specified by the user;
//	this is a unique ID for a particular slider. The default is -1
//	always, so note that until it is set, if there is more than one
//	slider bar on the screen, then unique idetification will be
//	impossible.
//  3) a part theType, predefined to be '0' for the
//	slider bar section, and '1' for the pointer arrow.
//
void Sv_Slider3::setIdentifier( GLshort typeident, GLshort theId )
{
	itsWidgetTypeId = typeident;
	itsId = theId;
}

void Sv_Slider3::Draw( void )	
{
	GLint i;
	GLboolean lightingWasEnabled = GL_TRUE;
	
//	glInitNames(); //eliminated by mdp 8/9/2002

	glPushName(itsWidgetTypeId);
	glPushName(itsId);
	
	if (!glIsEnabled(GL_LIGHTING))
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		lightingWasEnabled = GL_FALSE;
	}
	glShadeModel(GL_SMOOTH);

	glPushMatrix();

	glTranslatef(itsXCenter,  itsYCenter,  itsZCenter);
		
	glRotatef(itsZOrientation, 0, 0, 1);
	glRotatef(itsYOrientation, 0, 1, 0);
	glRotatef(itsXOrientation, 1, 0, 0);

	
	glPushName(AZIMUTH_TAB);
	glPushMatrix();
	
		glRotatef(azimuthVelocity, 0, 1, 0);
		glRotatef(180.0, 0, 0, 1);

		if (HighlightTab)
		{
			glColor3ub(PointerHighlightR,PointerHighlightG,PointerHighlightB);
		}
		else
		{
			glColor3ub(PointerRed,  PointerGreen,  PointerBlue);
		}
		azimuthTab->DrawPointer();				

	glPopMatrix();
	glPopName();

	// handle on the top of the arc
	if (drawTip)
	{
		glPushName(ELEVATION_CONTROL);  
		glColor3f(0.85f,0.85f,1.0);
		glPushMatrix();
			glRotatef(90.0,1,0,0);
			//glTranslatef(0.0,0.0,-30.0);
			glutSolidCone(17.0,50.0,12,2);
			//glTranslatef(0.0,0.0,-3.0);
			//glColor3f(.9f,.9f,.2f);
			//glutSolidSphere(15.0,10,7);
		glPopMatrix();
	}

	glShadeModel(GL_SMOOTH);

	glBegin(GL_TRIANGLE_STRIP);
	for (i=0; i<NUM_VERTICES; i++)
	{	
		if (i%2 == 0)	glColor3f(ringRed, ringGreen, ringBlue);
		else glColor3f(ringNotchRed, ringNotchGreen, ringNotchBlue);
		glNormal3fv(itsNormal[i]);			
		glVertex3f(itsVertex[i][X], itsVertex[i][Y]+WIDTH, itsVertex[i][Z]);
		glVertex3f(itsVertex[i][X], itsVertex[i][Y], itsVertex[i][Z]);
	}
	glEnd();
	glPopName();
	
    glPopMatrix();

	if (!lightingWasEnabled)
		glDisable(GL_LIGHTING);
}

GLfloat Sv_Slider3::getElevation( GLfloat ydelta)
{
	GLfloat changeElevation;

	changeElevation = (ydelta);

	itsElevation -= changeElevation*0.5;

	if (itsElevation < -30)
		itsElevation = -30;
	else if (itsElevation > 90)
		itsElevation = 90;
 
	return (itsElevation);
}

GLfloat Sv_Slider3::getCurrentElevation()
{
	return itsElevation;
}

void Sv_Slider3::setElevation( GLfloat val )
{
	itsElevation = val;
}

void Sv_Slider3::setAzimuth( GLfloat val )
{
	azimuth = val;
}

void Sv_Slider3::resetAzimuth( void )  // used to reset 
{
	azimuthVelocity = 0.0;
}


void Sv_Slider3::setAzimuthVel( GLfloat xdelta)   // set azimuth rotate velocity
{
	azimuthVelocity -= xdelta; 
}


GLfloat Sv_Slider3::updateAzimuth( void )
{
	azimuth += azimuthVelocity*itsVelocityRatio * Scene3D::timeForLastFrame()/400.0; //mdp
	
	while (azimuth > 360.0)
				azimuth -= 360.0f;
	while (azimuth < -360.0)
				azimuth += 360.0f;	

	return azimuth; 
		
}

void Sv_Slider3::CalculateAngles( void )
{    // this is  used for constructing the ring
	GLint i; 

	GLfloat angle,sina,cosa;
  
	for(i=0;i<NUM_VERTICES;++i)
	{
		if (i%2  == 0)
		{
			angle = float(i*10)/720.0;
		}
		else
		{
			angle = float((i-1)*10+5)/720.0; // 2 deg stripes		
		}
		sina = sin(angle*2.0*M_PI);
		cosa = cos(angle*2.0*M_PI);
		
		itsVertex[i][0] = itsRadius*sina;
		itsVertex[i][1] = 0.0; 
		itsVertex[i][2] = itsRadius*cosa;

		itsNormal[i][0] = sina;
		itsNormal[i][1] = 0.0;  
		itsNormal[i][2] = cosa;
	
	}
}
