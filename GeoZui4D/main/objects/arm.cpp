// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "arm.h"
#include "engine/ObjectPoseState.h"
#include "widgets/Sv_Standards.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/Window3D.h"
#include "engine/Joystick.h"
#include "engine/Scene3D.h"
#include <iostream>

using namespace std;

extern int dominantEye;
extern Joystick *joystickEventMediator;

REGISTER_OBJECT(arm, "arm");


#define SHOULDER_HEIGHT 0.2032f // 8"
#define SHOULDER_OFFSET 0.1529f  // 6.02"
#define SHOULDER_RAD 0.09842f	// 3.875"
#define LIMB_1_LEN	0.8753f  // 34.46"
#define LIMB_2_LEN 0.5662f  // 22.29"

#define LIMB_2_P1 0.21f
#define LIMB_2_P2 0.45f
#define LIMB_3_LEN 0.12065f // 4.75"
#define LIMB_4_LEN 0.35916f // 14.14
#define BASE_DIA	1.0
#define BASE_HEIGHT 0.2032f

#define JOINT1_HLEN	0.1f
#define JOINT2_HLEN	0.07f

#define PI	float(3.141592)

// convert to meters

arm::arm()
:Interactor("robotArm")
{
	int i;
	for (i=0;i<NJOINTS;++i)
	{
		angle[i] = 0.0;  
		jointMin[i] = -65.0 ;	 jointMax[i] = 65.0;
	}

	jointMin[0] = -137.5;	 jointMax[0] = 137.5;  // shoulder azimuth
	jointMin[1] = -30.0;		jointMax[1] = 90.0; // shoulder elevation
	jointMin[2] = -168.0 ;	 jointMax[2] = -38.0;  //  elbow
	jointMin[3] = -100.0;		jointMax[3] = 100.0;  // wrist pivot
	jointMin[4] = -100.0;		jointMax[4] = 100.0;  // wrist yaw
	jointMin[5] = -170.0;		jointMax[4] = 170.0;  // wrist rotate

	jointMin[6] =  1.25;		jointMax[6] = 20.0;  // gri

// start angles
	angle[0] = 0.0;
	angle[1] = 90.0;
	angle[2] = -45.0;
	angle[6] = 10.0;

	activejoint = 0;
	base = new cylinder(SHOULDER_RAD, BASE_HEIGHT);
	joint1 = new cylinder(0.07f,JOINT1_HLEN*2);
	joint2 = new cylinder(0.06f,JOINT2_HLEN*2);
	limb1 = new box(0.05f,0.08f, LIMB_1_LEN);
	limb2 = new box(0.05f,0.1f,LIMB_2_LEN);
	limb2inner = new box(0.05f,0.05f,LIMB_2_P1);
	limb2outer  = new box(0.05f,0.05f,LIMB_2_P2);
	limb3 = new box(0.05f,0.05f,LIMB_3_LEN);
	limb4 = new cylinder(0.035f,LIMB_4_LEN/2.0);
	gripper = new box(0.01f,0.01f,0.1f);

	joystickEventMediator->PushInteractor(this);
}


// documented in Object
int arm::readFrom(FILE *infile)
{
	char string[160];
	Point3d Position;

	fgets(string, 160, infile); // get comment string, good to use for
							    //identifying the object type,
							    //i.e. [SAMPLEOBJECT]

	//get the geographic X,Y,Z position of the object (mandatory)
	fscanf(infile, "%lf %lf %lf", &Position[0], &Position[1], &Position[2] ); 
	
	//set the geoOffset for use in data fusion (mandatory)
	SetPosition(Position);

	return 0; //returns 0 if ok.
}

void arm::DrawLowRes(float degradation, Window3D *win)
{
    doneFlag = 0;
	glRotatef(90,0.0,0.0,1.0);
	glRotatef(90,1.0,0.0,0.0);
	draw();
}

/*---------------------------------------------------------------------------*/
int arm::DrawHighRes(int eye, Window3D *win)
{
	DrawLowRes(0, NULL);

    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
	    doneFlag = 1;

	return 1;
}


int arm::HandleInput(unsigned int inputType, InputInfo *info)
{
	int redrawFlag = 0;

	Joystick::JoystickInfoType * js;

	if ((inputType & Joystick::JoystickAction))
	{
		js = (Joystick::JoystickInfoType *)(info->val.p);

		if((js->Axes[0] > 1000) || (js->Axes[0] < -1000))
		{
			angle[0] -= (js->Axes[0]/10000.0);
			if (angle[0] > jointMax[0]) angle[0] = jointMax[0];
			if (angle[0] < jointMin[0]) angle[0] = jointMin[0];
			redrawFlag = 1;
		}

		if( (js->Buttons[2]==0) && (js->Buttons[3]==0) &&  (( js->Axes[1] > 1000) || (js->Axes[1] < -1000)))
		{
			angle[1] += (js->Axes[1]/10000.0);
			if (angle[1] > jointMax[1]) angle[1] = jointMax[1];
			if (angle[1] < jointMin[1]) angle[1] = jointMin[1];
			redrawFlag = 1;
		}

		if( (js->Buttons[2]==0) && (js->Buttons[3]==1) &&  (( js->Axes[1] > 1000) || (js->Axes[1] < -1000)))
		{
			angle[2] += (js->Axes[1]/10000.0);
			if (angle[2] > jointMax[2]) angle[2] = jointMax[2];
			if (angle[2] < jointMin[2]) angle[2] = jointMin[2];
			redrawFlag = 1;
		}

		if( (js->Buttons[2]==1) && (js->Buttons[3]==0) &&  (( js->Axes[1] > 1000) || (js->Axes[1] < -1000)))
		{
			angle[3] += (js->Axes[1]/10000.0);
			if (angle[3] > jointMax[3]) angle[3] = jointMax[3];
			if (angle[3] < jointMin[3]) angle[3] = jointMin[3];
			redrawFlag = 1;
		}

//		if((js->Axes[2] > 1000) || (js->Axes[2] < -1000))
		{
			angle[5] = jointMin[5]+((js->Axes[2]/(32767.0*2.0))*(jointMax[5]-jointMin[5]));
//			if (angle[5] > jointMax[5]) angle[5] = jointMax[5];
//			if (angle[5] < jointMin[5]) angle[5] = jointMin[5];
			redrawFlag = 1;
		}

		if((js->Axes[3] > 3500) || (js->Axes[3] < -3500))
		{
			angle[4] -= (js->Axes[3]/15000.0);
			if (angle[4] > jointMax[4]) angle[4] = jointMax[4];
			if (angle[4] < jointMin[4]) angle[4] = jointMin[4];
			redrawFlag = 1;
		}

		if(js->Buttons[0]==1)
		{
			angle[6]-=1.0;
		}
		else
		{
			angle[6]+=1.0;
		}
		if (angle[6] > jointMax[6]) angle[6] = jointMax[6];
		if (angle[6] < jointMin[6]) angle[6] = jointMin[6];
		redrawFlag = 1;			

		if(redrawFlag)
		{
			globalInputMediator->Trigger(Scene3D::objectChangedEvent());
		}

		return SINK_EVENT;
	}

	return PASS_EVENT_THROUGH;
}

void arm:: keyInput(int k)
{
	switch(k)
	{
		case 9:   // tab
			++activejoint;
			activejoint = activejoint%NJOINTS;	
			break;

		case ']': 
			angle[activejoint] += 2.0;

			if (angle[activejoint] > jointMax[activejoint]) angle[activejoint] = jointMax[activejoint];
			break;

		case '[': 
			angle[activejoint] -= 2.0;
			
			if (angle[activejoint] < jointMin[activejoint]) angle[activejoint] = jointMin[activejoint];
			break;
	}

}

void arm:: draw()
{


	glPushMatrix();
	// BASE
	glColor3f(0.6f,0.6f,0.7f);
	glTranslatef(0.0,-2.0,0.0);
	glRotatef(angle[0], 0.0,1.0,0.0);
	glPushMatrix();
		glRotatef(90.0,0.0,0.0,1.0);
		base->draw();
	glPopMatrix();
	glTranslatef(SHOULDER_OFFSET,BASE_HEIGHT,0.0);
	glPushMatrix();
		glRotatef(90,0.0,1.0,0.0); glTranslatef(-JOINT1_HLEN,0.0,0.0);
		glColor3f(0.8f,0.8f,0.9f);
		joint1->draw();
	glPopMatrix();


	// LIMB 1 UPPER ARM
	glRotatef(angle[1], 0.0,0.0,1.0);
	glColor3f(0.6f,0.6f,0.7f);
	limb1->draw();
	glTranslatef(LIMB_1_LEN,0.0,0.0);
	glPushMatrix();
		glRotatef(90,0.0,1.0,0.0); glTranslatef(-JOINT1_HLEN,0.0,0.0);
		glColor3f(0.8f,0.8f,0.9f);
		joint1->draw();
	glPopMatrix();

	// LIBM 2 FOREARM
	glRotatef(angle[2], 0.0,0.0,1.0);
	glColor3f(0.6f,0.6f,0.7f);
	glPushMatrix();
		glRotatef(33.0, 0.0,0.0,1.0);
		limb2inner->draw();
		glTranslatef(0.17f,0.0,0.0);
		glRotatef(-45.0,0.0,0.0,1.0);
		glTranslatef(0.0,0.01f,0.0);
		limb2outer->draw();
	glPopMatrix();	

	glTranslatef(LIMB_2_LEN,0.0,0.0);
	glPushMatrix();
		glRotatef(90,0.0,1.0,0.0);  glTranslatef(-JOINT2_HLEN,0.0,0.0);
		glColor3f(0.8f,0.8f,0.9f);
		joint2->draw();
	glPopMatrix();

	// LIMB 3 WRIST
	glRotatef(angle[3], 0.0,0.0,1.0);
	glColor3f(0.6f,0.6f,0.7f);
	limb3->draw();
	glTranslatef(LIMB_3_LEN,0.0,0.0);
	glPushMatrix();
		glRotatef(90,0.0,0.0,1.0); glTranslatef(-JOINT2_HLEN,0.0,0.0);
		glColor3f(0.8f,0.8f,0.9f);
		joint2->draw();
	glPopMatrix();

	//LIMB 4 HAND
	glColor3f(1.0,0.4f,0.2f);
	glTranslatef(0.0,0.03f,0.0);
	glRotatef(angle[4],0.0,1.0,0.0);
	limb4->draw();
	glTranslatef(LIMB_4_LEN/2.0,0.0,0.0);
	glRotatef(angle[5], 1.0,0.0,0.0);

	//	GRIPPER 
	glColor3f(1.0,1.0,0.0);
	glTranslatef(0.0,0.0,angle[6]*0.004f);
	gripper->draw();
	glTranslatef(0.0,0.0,-angle[6]*0.008f);
	gripper->draw();
	glPopMatrix();	
}


// parts for the arm
arm::box::box(float h, float w, float len )
{
	Points[0][0] = 0.0;
	Points[0][1] = -w;
	Points[0][2] = h;
	Points[1][0] = 0.0;
	Points[1][1] = -w;
	Points[1][2] = -h;

	Points[2][0] = 0.0;
	Points[2][1] = w;
	Points[2][2] = h;
	Points[3][0] = 0.0;
	Points[3][1] = w;
	Points[3][2] = -h;

	Points[4][0] = len;
	Points[4][1] = w;
	Points[4][2] = h;
	Points[5][0] = len;
	Points[5][1] = w;
	Points[5][2] = -h;

	Points[6][0] = len;
	Points[6][1] = -w;
	Points[6][2] = h;
	Points[7][0] = len;
	Points[7][1] = -w;
	Points[7][2] = -h;

}


void arm::box::draw()
{

	glShadeModel(GL_FLAT);

	//draw the sides
	glBegin(GL_QUAD_STRIP);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3fv(Points[0]); 
	glVertex3fv(Points[1]); 
	glVertex3fv(Points[2]); 
	glVertex3fv(Points[3]); 

	glNormal3f(0.0, 1.0, 0.0);
	glVertex3fv(Points[4]); 
	glVertex3fv(Points[5]); 

	glNormal3f(1.0, 0.0, 0.0);
	glVertex3fv(Points[6]); 
	glVertex3fv(Points[7]);		

	glNormal3f(0.0, -1.0, 0.0);
	glVertex3fv(Points[0]); 
	glVertex3fv(Points[1]);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3fv(Points[0]); 
	glVertex3fv(Points[2]); 
	glVertex3fv(Points[4]); 
	glVertex3fv(Points[6]); 
	glEnd();

		glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3fv(Points[1]); 
	glVertex3fv(Points[3]); 
	glVertex3fv(Points[5]); 
	glVertex3fv(Points[7]); 
	glEnd();

		glShadeModel(GL_SMOOTH);

}



arm::cylinder::cylinder(float radius, float len)
{

	rad = radius;
	length = len;

	makeCylinder();
}


void arm::cylinder::draw()
{

	int i;

	//do pairs of points
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < NSEGS; ++i)
	{
		glNormal3fv(Normals[i]);
		glVertex3fv(Points[i*2]);
		glVertex3fv(Points[i*2+1]);
	}
	glNormal3fv(Normals[0]);
	glVertex3fv(Points[0]);
	glVertex3fv(Points[1]);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(length,0.0,0.0);
	for (i = 1; i < NSEGS*2; i+=2)
	{	
		glVertex3fv(Points[i]);
	}
	glVertex3fv(Points[1]);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(0.0,0.0,0.0);
	for (i = 0; i < NSEGS*2; i+=2)
	{	
		glVertex3fv(Points[i]);
	}
	glVertex3fv(Points[0]);
	glEnd();
}



void arm::cylinder::makeCylinder()
{
	int i;
	float x,y,angle;

	for ( i = 0; i < NSEGS; i++) // nStrips
	{
		angle = float(i)/float(NSEGS);
		x = float(sin(2.0*angle*PI));
		y = float(cos(2.0*angle*PI));

		Points[2*i][0] = 0.0;
		Points[2*i][1] = x*rad;
		Points[2*i][2] = y*rad;
		Points[2*i+1][0] = length;
		Points[2*i+1][1] = x*rad;
		Points[2*i+1][2] = y*rad;

		Normals[i][0]=0.0;
		Normals[i][1] =  x;
		Normals[i][2]= y;
	}
}







