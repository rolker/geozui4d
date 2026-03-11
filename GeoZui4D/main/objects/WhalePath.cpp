// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// WhalePath.cpp
/** 
 *	This Object is created especially for aiding the visualization and research
 *	of whale ehtology.  Certain components of this object may be useful to
 *	implement in others to make them more powerful, but this one exists to
 *	make further development of the whale research visualizations easier to
 *	integrate with GeoZui4D.
 *
 *	Originally created by Colin Ware outside GeoZui (4/2005)
 *	Brought into GeoZui by Matthew Plumlee 5/19/2005
 */
///////////////////////////////////////////////////////////////////////////////
#include "WhalePath.h"
#include "widgets/Sv_Matrix4.h"
#include "GZincludes.h"
#include "engine/ObjectPoseState.h"
#include "utilities/DottedAddress.h"
#include <math.h>


extern MainWindow *mainWindow;

#define NPOINTS 100000
#define CYCLES 5
#define WIDTH 330
#define SMFAC 2

REGISTER_OBJECT(WhalePath, "");

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
WhalePath::WhalePath()
{
	int i;

	arrayOfArrays[aPATHX] = &pathX;
	arrayOfArrays[aPATHY] = &pathY;
	arrayOfArrays[aPATHZ] = &pathZ;
	arrayOfArrays[aAZIM] = &azim;
	arrayOfArrays[aELEV] = &elev;
	arrayOfArrays[aROLL] = &roll;
	arrayOfArrays[aDELTAELEV] = &deltaElev;
	arrayOfArrays[aDDELTAELEV] = &ddeltaElev;
	arrayOfArrays[aDELTAROLL] = &deltaRoll;
	arrayOfArrays[aDDELTAROLL] = &ddeltaRoll;
	//not used right now...
	deltaElev = NULL;
	deltaRoll = NULL;
	ddeltaRoll = NULL;

	for (i = 0; i <= aLastOrient; i++)
		*arrayOfArrays[i] = new float[NPOINTS];

	pLen = NPOINTS;

	trackCenter = 590;

	for(i=0;i<100;++i)
	{
		meanprofile[i]=0.0;
	}

	arrayOfPointArrays[paBox1] = &Box1;
	arrayOfPointArrays[paBox2] = &Box2;
	arrayOfPointArrays[paBox3] = &Box3;
	arrayOfPointArrays[paBox4] = &Box4;
	arrayOfPointArrays[paSideways] = &Sideways;
	arrayOfPointArrays[padNorms] = &dNorms;
	arrayOfPointArrays[panegdNorms] = &negdNorms;
	arrayOfPointArrays[padorsalAcc] = &dorsalAcc;
	arrayOfPointArrays[parotAcc] = &rotAcc;
	arrayOfPointArrays[paLeftFlipper1] = &LeftFlipper1;
	arrayOfPointArrays[paLeftFlipper2] = &LeftFlipper2;
	arrayOfPointArrays[paRightFlipper1] = &RightFlipper1;
	arrayOfPointArrays[paRightFlipper2] = &RightFlipper2;
	arrayOfPointArrays[padorsalColors] = &dorsalColors;

	savewave = 0;

	cursor = new ObjectPoseState(this, "cursor");
}

/*---------------------------------------------------------------------------*/
WhalePath::~WhalePath()
{
	int i;

	for (i = 0; i < numArrays; i++)
	{
		if (*arrayOfArrays[i])
			delete []*arrayOfArrays[i];
	}
	for (i = 0; i < numPointArrays; i++)
	{
		if (*arrayOfPointArrays[i])
			delete []*arrayOfPointArrays[i];
	}
}

/*---------------------------------------------------------------------------*/
void WhalePath::initializingLabel()
{
	pose->getAddress()->addChild(cursor->getAddress());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void WhalePath::average()
{
	int i,j,k,c;

	float wavelength,start,end;
	float avg[100],meandev;

	float peak;
	peak = 0.0;

	// the idea is to find the wavelength with the strongest energy
	for(i=0;i<100;++i)
		meanprofile[i]=0.0;

	for(k=0;k<30;++k)
	{
		//reset avg array
		for(i=0;i<100;++i)
			avg[i] = 0.0;

		//wavelength goes from 6 to 20.5 in increments of 0.5
		wavelength = 6+k*0.5;

		for (c=0;c < CYCLES;++c)
		{
			start = trackCenter + float(c)*wavelength+0.5;
			end = start + wavelength;
			i = 0;
			for (j=int(start);j<int(end);++j)
			{
				avg[i] += ddeltaElev[j];
				++i;
			}
		}

		meandev = 0;
		for(i=0;i<100;++i)
		{
			meandev += fabs(avg[i]);
		}
		meandev = meandev/wavelength;

		if (meandev > peak)
		{
			peak = meandev;
			savewave = wavelength;
			for(i=0;i<wavelength;++i)
			{
				meanprofile[i] = avg[i]/CYCLES;

				//cerr << meanprofile[i] << " ";
			}
			//cerr << "\n";
		}

		//cerr << "Wave = " << wavelength << " Dev " << meandev << "\n";
	
	}
        std::cerr << "Wavelength = " << savewave << " Energy " << peak << "\n";
}


/*---------------------------------------------------------------------------*/
Object * WhalePath::LoadXML(bpt::ptree const &xml, char * path, gz4d::netcdf::File ncfile)
{
    int i, j;
    Point3d pos, orient;

    Object::LoadXML(xml,path, ncfile);

    for(bpt::ptree::const_iterator node = xml.begin(); node != xml.end(); ++node)
        if(node->first == "Position")
        {
            if (i >= NPOINTS)
                break;
            boost::optional<std::string> attribute;
            if(attribute = node->second.get_optional<std::string>("<xmlattr>.position"))
            {
                std::stringstream ss(*attribute);
                ss >> pos[0] >> pos[1] >> pos[2];
            }
            if(attribute = node->second.get_optional<std::string>("<xmlattr>.orientation"))
            {
                std::stringstream ss(*attribute);
                ss >> orient[0] >> orient[1] >> orient[2];
            }

            for (j = X; j <= Z; j++)
            {
                (*arrayOfArrays[j])[i] = pos[j]; //PathX, PathY, PathZ
                (*arrayOfArrays[j + aROLL])[i] = orient[j]; //roll, elev, azim
            }
        }

    initialize();

    return this;
}

/*---------------------------------------------------------------------------*/
void WhalePath::initialize()
{
	int i, j;
	float up, front, side;
	float dTheta, dRot;

	float forward[4];
	float oldforward[4] = {1.0, 0.0, 0.0, 0.0};
	float sideways[4];
	float oldsideways[4] = {0.0, 0.0, 1.0, 0.0};
	float dorsal[4];

//	float dx,dy,dz,len;

	float vent[4] = {0.0, 1.0, 0.0, 0.0};
	float lat[4] = {1.0, 0.0, 0.0, 0.0};
	float dor[4] = {0.0, 0.0, 1.0, 0.0};

	Sv_Matrix4 M1,Maz,Mel,Mro;

	M1.SetIdentity();

	for (i = aPATHX; i <= aLastOrient; i++)
		j = smooth(SMFAC, *arrayOfArrays[i]);
	pLen = j;

	for (i = 0; i < numPointArrays; i++)
		*arrayOfPointArrays[i] = new Point3f[pLen];

	for(j=0;j<pLen-1;++j)
	{
		Maz.SetRotation(-azim[j],'z');
		Mel.SetRotation(-elev[j],'x');
		Mro.SetRotation(roll[j],'y');

		//cerr << "AZIM EL ROLL  " << az << " " << el << " " << ro << "\n";

		M1.SetIdentity();

		M1 = M1*Mro;
		M1 = M1*Mel;
		M1 = M1*Maz;
		
		M1.VecMultMat(vent,forward);
		M1.VecMultMat(lat,sideways);
		M1.VecMultMat(dor,dorsal);
	
		// find the deltapitch in whale_coords
		front = up = side = 0;
		for (i = X; i <= Z; i++)
		{
			front += forward[i]*oldforward[i];
			up += dorsal[i]*oldforward[i];
			side += sideways[i]*oldsideways[i];
		}

		dTheta = atan2f(up,front); 
		dRot = atan2f(up,side);

		if(dTheta > 3)
		{
                    std::cerr << "forward " << forward[0] << " " << forward[1] << " " << forward[2] << "\n";
                    std::cerr << "oldforward " << oldforward[0] << " " << oldforward[1] << " " << oldforward[2] << "\n";	
                    std::cerr << "THETA " << dTheta << " " <<front << " " << up << "\n\n";
		}
		for (i = X; i <= Z; i++)
		{
			oldforward[i] = forward[i];
			oldsideways[i] = sideways[i];
		//	Fwd[j][i] = forward[i];
			Sideways[j][i] = sideways[i];
			dNorms[j][i] = dorsal[i];
			negdNorms[j][i] = -dorsal[i];
		}
			
		// improve this
		if (dTheta > 0.2 || dTheta < -0.2) 
				dTheta = 0.0;
		elev[j] = dTheta*180.0/3.14159;

		if (dRot > 0.3 || dRot < -0.3) 
				dRot = 0.0;
		roll[j] = dRot*180.0/3.14159;
	}

	elev[pLen] = 0.0;
	roll[pLen] = 0.0;

// note: this has been averaged
	differentiate(elev,ddeltaElev);
	//differentiate(elev,deltaElev);	
//	differentiate(deltaElev,ddeltaElev);

	float wid = 2.0;
	float flipperwid = 5.0;
	double rollFactor[3] = {0.2, 0.2, 0.42}; //&&&should this las number really be 0.42?? If not, we can nix rollFactor.
	float dorsalPlus[3] = {1.0, 0.3f, 0.0}; //reddish
	float dorsalMinus[3] = {0.0, 1.0, 1.0}; //cyan

	for(i=0;i<pLen;++i)
	{
		for (j = X; j <= Z; j++)
		{
			Box1[i][j]= (*arrayOfArrays[j])[i] + Sideways[i][j]*wid; //PathX/Y/Z plus sideways
			Box3[i][j] = Box1[i][j] - dNorms[i][j]*0.4;
			Box2[i][j]= (*arrayOfArrays[j])[i] - Sideways[i][j]*wid; //PathX/Y/Z minus sideways
			Box4[i][j] = Box2[i][j] - dNorms[i][j]*0.4;

			dorsalAcc[i][j] = (*arrayOfArrays[j])[i] + dNorms[i][j]*ddeltaElev[i]*0.7;//PathX/Y/Z plus dNorms,etc.
			LeftFlipper1[i][j] = (*arrayOfArrays[j])[i] + Sideways[i][j]*flipperwid - dNorms[i][j]*roll[i]*rollFactor[j];//PathX/Y/Z plus all that jazz
			LeftFlipper2[i][j] = LeftFlipper1[i][j] + dNorms[i][j]*roll[i]*0.4;

			if (ddeltaElev[i] > 0.0)
				dorsalColors[i][j] = dorsalPlus[j];
			else
				dorsalColors[i][j] = dorsalMinus[j];
		}
	}

	//for(i=0;i<10;++i)
		//cerr << "Path X Y Z " << pathX[trackCenter] << " " << pathY[200] << " " <<pathZ[200] << "\n";
}

/*---------------------------------------------------------------------------*/
void WhalePath::DrawLowRes(float degradation, Window3D *win)
{
	int i, j;
//	float emission[] = {0.5,0.0,0.0,0.0};
	float Nemission[] = {0.0,0.0,0.0,0.0};
	Point3f *norms, *vertices1, *vertices2;
	Point3f *normSide[4] = {dNorms, negdNorms, Sideways, Sideways};
	Point3f *v1Side[4] = {Box1, Box3, Box1, Box2};
	Point3f *v2Side[4] = {Box2, Box4, Box3, Box4};

	// this draws the path about a particular track point index;
	glColor3f(1.0,1.0,0.0);

	//dot at trackCenter
	glPushMatrix();
		glTranslatef(pathX[trackCenter], pathY[trackCenter], pathZ[trackCenter]);
		glutSolidSphere(1.5,12,12);
		glRotatef(-win->getPoseInfo()->getOrientationAxis()[Yaw], 0.0, 0.0, 1.0);
		drawProfile();
	glPopMatrix();

	glMaterialfv(GL_FRONT, GL_EMISSION, Nemission);
	glShadeModel(GL_FLAT);

	//centerline
	glBegin(GL_LINE_STRIP);
	for(i=0;i<pLen;++i)
	{
		glVertex3f(pathX[i],pathY[i],pathZ[i]);
	}
	glEnd();

	//outer box
	glEnable(GL_LIGHTING);
	for (j = 0; j < 4; j++)
	{
		norms = normSide[j];
		vertices1 = v1Side[j];
		vertices2 = v2Side[j];

		glBegin(GL_TRIANGLE_STRIP);
		for(i=0;i<pLen-1;++i)
		{
			//striped
			if(i%2==0)
				glColor3f(0.7f, 0.7f, 0.7f);
			else
				glColor3f(0.8f, 0.8f, 0.8f);
			glNormal3fv(norms[i]);
			glVertex3fv(vertices1[i]);
			glVertex3fv(vertices2[i]);
		}
		glEnd();
	}

	//Dorsal acceleration
	//glMaterialfv(GL_FRONT, GL_EMISSION, emission);		
	glDisable(GL_LIGHTING);	
	glShadeModel(GL_SMOOTH);
	glBegin(GL_TRIANGLE_STRIP);
	for(i=0;i<pLen-1;++i)
	{
		glColor3f(0.0,0.0,0.0);
		glNormal3fv(Sideways[i]);
		glVertex3f(pathX[i],pathY[i],pathZ[i]);

		glColor3fv(dorsalColors[i]);
		glVertex3fv(dorsalAcc[i]);
	}
	glEnd();

/*

		glColor3f(0.9,0.9,0.0);

		glBegin(GL_TRIANGLES);
		for(i=0;i<pLen-1;++i)
		{
			glColor3f(0.5,1.0,1.0);
			
			glVertex3f(pathX[i],pathY[i],pathZ[i]);
			glVertex3fv(LeftFlipper1[i]);

			glColor3f(0.0,0.0,0.5);
			glVertex3fv(LeftFlipper2[i]);
		}
		glEnd();



		glColor3f(1.0,1.0,0.2);

		glBegin(GL_LINES);
		for(i=0;i<pLen;++i)
		{
			glVertex3f(pathX[i],pathY[i],pathZ[i]);
			glVertex3f(pathX[i]+Fwd[i][0]*5,pathY[i]+Fwd[i][1]*5,pathZ[i]+Fwd[i][2]*5);
		}
		glEnd();
		*/

	/*
	glColor3f(0.0,1.0,1.0);

	glBegin(GL_LINES);
//	glVertex3f(0.0,0.0,0.0);
//	glVertex3f(forward[0]*19.0,forward[1]*19.0,forward[2]*19.0);

//	glVertex3f(0.0,0.0,0.0);
//	glVertex3f(sideways[0]*19.0,sideways[1]*19.0,sideways[2]*19.0);

	glColor3f(1.0,1.0,0.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(20.0,0.0,0.0);

	glColor3f(1.0,0.0,1.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,20.0,0.0);

	glEnd();
	*/
}

/*---------------------------------------------------------------------------*/
void WhalePath::drawProfile()
{
	int i;
	float x1, y1;
	glColor3f(0.7f, 0.9f, 0.7f);
	//glRectf(2.0,-10.0,100.0,10.0);

	glDisable(GL_LIGHTING);
	//glColor3f(0.0, 0.7f, 0.0);
/*
	for(i=0;i<savewave;++i)
	{
		glRectf(6.0+i*4,-10.0,10.0+i*4,-10.0+meanprofile[i]*5.0);
	}
	*/
	glBegin(GL_TRIANGLE_STRIP);
	for(i=0;i<savewave;++i)
	{
		x1 = 6.0+i*4;
		y1 = -10.0+meanprofile[i]*5.0;
		glVertex3f(x1,0.0,-10.0);
		glVertex3f(x1,0.0,y1);
	}
	glEnd();
	glEnable(GL_LIGHTING);
}

/*---------------------------------------------------------------------------*/
int WhalePath::smooth(int sn, float *&in)
{
	// the purpose of this module is to smooth a vector
	// reducing its lenght by a factor of sn;
	// the new length is returned ;

	int i,j,k;
	float sum;
	int m;
	float *out;

	m = pLen/sn;

	out = new float[m];

	for(i=0;i<m;++i)
	{
		sum = 0.0;
		
		for(j=0;j<sn;++j)
		{
			k = i*sn+j;
			sum += in[k];
		}
		out[i] = sum/sn;
	}

    //for(i=0;i<10;++i)
	//	cerr << i << " " << out[i] << "\n";
	
	delete []in;

	in = out;

	return m;
}

/*---------------------------------------------------------------------------*/
void WhalePath::differentiate(float *in, float *&out)
{
	int i;

	out = new float[pLen];

	out[0] = 0.0;

	for(i=1;i<pLen-1;++i)
	{
		out[i] = (in[i+1] - in[i-1])/2.0;
	}
	out[pLen-1] = 0.0;
}

/*---------------------------------------------------------------------------*/
void WhalePath::advance(int step)
{
	OrientationInfo o;

	trackCenter = trackCenter + step;

	average();
	cursor->AccumulateNotifications();
	cursor->setPosition(pathX[trackCenter], pathY[trackCenter], pathZ[trackCenter]);
	o.axis[Yaw] = azim[trackCenter];
	o.axis[Pitch] = elev[trackCenter];
	o.axis[Roll] = roll[trackCenter];
	cursor->setOrientation(o);
	cursor->FlushNotifications();

	globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/*-------------------------Functions to Interface w/ Tcl---------------------*/
/*---------------------------------------------------------------------------*/
int WhalePath::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	const char *message = argv[0];
	
	if (argc >= 1)
	{
		if (!strcmp(message, "advance"))
		{
			if (argc >= 2)
				advance(atoi(argv[1]));
			else
				advance(10);
			return TCL_OK;
		}
	}
	return Object::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int WhalePath::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "advance ", NULL);
    return Object::list(interp);
}

