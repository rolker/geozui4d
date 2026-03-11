// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include"Reson.h"
#include<iostream>
//#include<time.h>
#include <filesystem>

REGISTER_OBJECT(Reson, "pts");

using namespace std;

Reson::Reson()
{
    quadtree = new SDV_Quadtree(.5);
    deg = 0;
    starttime = endtime = 0;
}

int Reson::Load()
{
    Point3d s;
	char tmpstr[1000];
	
	infile = fopen(GetFilename().c_str(), "r");
	if (infile == NULL)
	{
		printf("\nCannot open %s\n", GetFilename().c_str());
		return(1);
	}

        while (fgets(tmpstr,1000,infile) && sscanf(tmpstr,"SOL E: %lf N: %lf ",&s[0],&s[1])==0)
	{}

	s[2]=0;

	SetPosition(s);
	doneloading = 0;

	return 0;
}

void Reson::Idle(double time)
{
	Object::Idle(time);
	LoadNext();
}

void Reson::LoadNext()
{
    Point3f s;
	char tmpstr[1000];

    if(doneloading == 1)
    {
        return;
    }

    while (fgets(tmpstr,1000,infile) && sscanf(tmpstr,"%f %f %f %*i",&s[0],&s[1],&s[2])!=3)
    {
        if (feof(infile) != 0)
        {
            doneloading = 1;
           	PoseChanged(NULL,0);
            return;
        }
    }
    
    
    s[0] -= (float)geoRef[0];
    s[1] -= (float)geoRef[1];
    s[2] -= (float)geoRef[2];
    XYZPoint tmpsounding;
    COPY_POINT(tmpsounding.sounding,s);
    quadtree->AddNode(tmpsounding,s[0],s[1]);

    while (sscanf(tmpstr,"%f %f %f %*i",&s[0],&s[1],&s[2])==3)
    {
        s[0] -= (float)geoRef[0];
        s[1] -= (float)geoRef[1];
        s[2] -= (float)geoRef[2];
        XYZPoint tmpsounding;
        COPY_POINT(tmpsounding.sounding,s);
        quadtree->AddNode(tmpsounding,s[0],s[1]);

        if (!fgets(tmpstr,1000,infile) || feof(infile) != 0)
        {
            doneloading = 1;
           	PoseChanged(NULL,0);
            return;
        }
    }
	PoseChanged(NULL,0);
}

void Reson::DrawLowRes(float degradation, Window3D *win)
{
//    LoadNext();
//    quadtree->DrawGrid();
//    quadtree->DrawValues();
    deg = (int)(5*degradation);
//    cerr << deg << "\n";
//    starttime = clock();
//    cerr << "Overhead time: " << starttime - endtime << "\n";
    glColor3f(1.0,1.0,1.0);
    EnableTexture();
//    quadtree->Draw(deg);
	quadtree->Draw(1);
    DisableTexture();
//    endtime = clock();
//    cerr << "Frame time: " << endtime - starttime << "\n";
}

int Reson::DrawHighRes(int eye, Window3D *win)
{
    int ret;
//    LoadNext();
//    quadtree->DrawGrid();
//    quadtree->DrawValues();
    glColor3f(1.0,1.0,1.0);
    EnableTexture();
    ret = quadtree->DrawLine(eye);
    DisableTexture();
    return ret;
}

