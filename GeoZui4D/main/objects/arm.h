// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// arm.h
/*---------------------------------------------------------------------------*/
/** 
 *  Robot arm modeled from the PREDATOR arm used by WHOI on Jason II
 *
 *	Created by colin and roland April 10th, 2002
 */
/*---------------------------------------------------------------------------*/
#ifndef ARM_H
#define ARM_H


#include "engine/Object.h"
#include "interactors/Pickable.h"
#include "engine/Interactor.h"

class PosableStateLock;


#define NJOINTS 7
#define NSEGS 12  // cylinder segments


class arm : public Object, public Interactor
{
public:
	arm();
	DERIVE_OBJECT(arm);
	void keyInput(int k);
	void draw();
	//File-related functions
    int readFrom(FILE *file); //read from a file
    //int writeTo(FILE *file);  //define this if you want to be able to save to a file

	//Display-related functions
    void DrawLowRes(float degradation, Window3D *win);
    int DrawHighRes(int eye, Window3D *win);

	int HandleInput(unsigned int inputType, InputInfo *info);
private:
	class cylinder;
	class box;
	
	float angle[NJOINTS];
	float jointMin[NJOINTS];
	float jointMax[NJOINTS];
	int activejoint;
	cylinder *base;
	cylinder *joint1,*joint2, *limb4;
	box *limb1,*limb2,*limb3;
	box *limb2inner, *limb2outer;
	box *gripper;

	class box 
	{
	public:
		box(float w, float h, float len);
		void draw();
	private:
		float Points[8][3]; // to store the cube vertices	
	};
	
	class cylinder
	{
	public:
		cylinder(float radius, float length);
		void draw();
	private:
		void makeCylinder();
		float Normals[NSEGS][3];
		float Points[NSEGS*2][3];
		float rad, length;	
	};

};

#endif
