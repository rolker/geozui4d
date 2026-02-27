/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifdef WIN32

//#include <time.h>
#include <iostream>
#include "Isis.h"
#include "engine/PosableStateBase.h"

REGISTER_OBJECT(Isis, "xyz");

Isis::Isis()
{
    quadtree = new SDV_Quadtree(5);
}

int Isis::Load(Point3d georef_p)
{
	int retval;

	retval = Object::Load();
    
	if (retval)
		return retval;

	infile = fopen(GetFilename().string().c_str(), "r");

    if (infile == NULL)
	{
    	printf("\nCannot open %s\n",GetFilename().string().c_str());
        return(1);
    }
	return retval;
}

// documented in Object
int Isis::readFrom(FILE *infile)
{
	fscanf(infile,"%f %f %f ",&s[0],&s[1],&s[2]);
    
	fp = ftell(infile);
    geoRef[0] = s[0];
    geoRef[1] = s[1];
    geoRef[2] = 0;

    doneloading = 0;
    donechunk = 1;

	return 0;
}

void Isis::LoadNext()
{
    Point3f s;
    SoundingsAvg * tmpsoundings;
	int count;
	const GLfloat *offset = pose->getPosition();
//	double fstarttime, fendtime;
//	double starttime, endtime;
	

    if(doneloading == 1)
    {
        return;
    }
    
	if(donechunk == 1)
	{

		_fstat(infile->_file,&infilestat);

		cerr << "fp: " << fp << "\tsize: " << infilestat.st_size << "\n";
		if (infilestat.st_size > fp+INBUFSIZE)
		{
			fread(inbuf,1,INBUFSIZE,infile);
			bufptr=inbuf;
			fp = ftell(infile);
			donechunk = 0;
//			cerr << "file read time: " << fendtime-fstarttime << "\n";
		}
	}	
	
	if(donechunk == 0)
	{
		count = 0;
		while(donechunk == 0 && count < 1000)
		{
			if (sscanf(bufptr," %f %f %f ",&s[0],&s[1],&s[2])==3)
			{
				s[0] = s[0] - geoRef[0] +offset[0];
				s[1] = s[1] - geoRef[1] +offset[1];
				s[2] = s[2] - geoRef[2] +offset[2];
				
				//	cerr << "Sounding: " << s[0] << ", " << s[1] << ", " << s[2] << "\n";
				
				tmpsoundings = new SoundingsAvg;
				tmpsoundings->Add(s);
				quadtree->AddNode(tmpsoundings,s[0],s[1]);
				bufptr = &bufptr[1];
				while(bufptr[0]!= 0 && bufptr[0] != '\n')
				{
					bufptr = &bufptr[1];
				}
			}
			else
			{
				donechunk = 1;
			}
			count++;
		}
	}
//	cerr << "Load next: " << endtime - starttime << "\n";
	
}

void Isis::DrawLowRes(float degradation, Window3D *win)
{
    LoadNext();
//    quadtree->DrawGrid();
//    quadtree->DrawValues();
    glColor3f(1.0,1.0,1.0);
    EnableTexture();
    quadtree->Draw(1);
    DisableTexture();
}

int Isis::DrawHighRes(int eye, Window3D *win)
{
    LoadNext();
//    quadtree->DrawGrid();
//    quadtree->DrawValues();
    glColor3f(1.0,1.0,1.0);
    EnableTexture();
	quadtree->Draw();
    DisableTexture();
    return 1;
}

#endif  // WIN32
