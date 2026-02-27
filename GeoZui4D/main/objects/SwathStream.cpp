/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "GZincludes.h"
//#include "MainWindow.h"
//#include <time.h>
#include "SwathStream.h"
#include "engine/ObjectPoseState.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "objects/nodeTypes/XYZPoint.h"
#include "objects/nodeTypes/XYZPointAvg.h"
#include "objects/nodeTypes/XYZPointWA.h"
//#include "engine/Scene3D.h"
#include "engine/InputMediator.h"
#include "engine/ViewManager.h"

extern MainWindow *mainWindow;

REGISTER_OBJECT(SwathStream, "sws");

SDV_Quadtree * SwathStream::quadtree = NULL;
//SDV_Tiles * SoundingStream::quadtree = NULL;
char * SwathStream::soundingtype = NULL;
int SwathStream::masterpresent = 0;



extern MainWindow *mainWindow;

SwathStream::SwathStream()
: Object(GL_FALSE, PICK_CLICK1)
{
    char *tempname = new char[20];

    sprintf(tempname, "SwathStream_%d", id);
    pose = new ObjectPoseState(this, tempname);
    delete tempname;

    soundingcount = 0;
    swathcount = 0;
    if (masterpresent == 0)
    {
        masterpresent = 1;
        ismaster = 1;
    }
    else
    {
        ismaster = 0;
    }
    swathbuffer = NULL;
    swsfile = NULL;
    asciiIn = NULL;
#ifndef WIN32
//    rawmb = NULL;
#endif

    mainWindow->requestNewSubwindow(this, mainWindow);

    std::cerr << "SwathStream::Constructor " << id << "\n";
}

void SwathStream::DrawLowRes(float degradation, Window3D *win)
{
    float heightExaggeration = win->getViewInfo()->getHeightExaggeration();
        
    DrawShip();
    if(quadtree != NULL && ismaster == 1)
    {
//        degres = (unsigned char)(degradation*10);
        if(degradation == 1.0 && degres < quadtree->GetTopLevelResolution()-2)
        {
            degcount += 1;
        }
        else if(degradation < 0.5 && degres > 1)
        {
            degcount -= 1;
        }
        else if(degcount > 0)
        {
            degcount--;
        }
        
        if (degcount >= 5)
        {
            degres++;
            degcount = 0;
        }
        
        if (degcount <= -5)
        {
            degres--;
            degcount = 0;
        }

    //    quadtree->DrawGrid();
    //    quadtree->DrawValues();
        glPopMatrix();
        glColor3f(1.0,1.0,1.0);
        EnableTexture();
//        quadtree->Draw(degres);
        quadtree->Draw(2,heightExaggeration);
        DisableTexture();
        
        glPushMatrix();
    }

    swathbuffer->Draw(degradation);
}

int SwathStream::DrawHighRes(int eye, Window3D *win)
{
    int ret;
    ret = 1;
    glPopMatrix();
    if(quadtree != NULL && ismaster == 1)
    {
    //   quadtree->DrawGrid();
    //    quadtree->DrawValues();
        glColor3f(1.0,1.0,1.0);
        EnableTexture();
        ret = quadtree->DrawLine(eye);
        DisableTexture();
    }
    if(ret == 1)
    {
        if (swathbuffer != NULL) swathbuffer->Draw();
        pose->applyTransforms();
        DrawShip();
    }
    glPushMatrix();
    return ret;
}

void SwathStream::DrawShip()
{
    glPushMatrix();
    {
        glEnable(GL_LINE_SMOOTH); // Anti-alias lines...
//		glDisable(GL_LIGHTING);
        
        glColor3f(0.8f, 0.8f, 1.0f);
        glScalef(0.3f, 0.3f, 0.3f);

//		pose->applyTransforms();

        glRotatef(heading,0.0,0.0,-1.0);
        glRotatef(pitch,1.0,0.0,0.0);
        glRotatef(roll,0.0,1.0,0.0);

        glBegin(GL_POLYGON);
        glNormal3f(0.0,0.0,-1.0);
        glVertex3f(5.0,-25,0.0); // bottom
        glVertex3f(5.0,25,0.0);
        glVertex3f(0.0,35,0.0);
        glVertex3f(-5.0,25,0.0);
        glVertex3f(-5.0,-25,0.0);
        glEnd();        

        glBegin(GL_POLYGON);
        glNormal3f(0.0,0.0,1.0);
        glVertex3f(5.0,-25,5.0); // top
        glVertex3f(5.0,25,5.0);
        glVertex3f(0.0,35,5.0);
        glVertex3f(-5.0,25,5.0);
        glVertex3f(-5.0,-25,5.0);
        glEnd();        

        glBegin(GL_POLYGON);
        glNormal3f(1.0,0.0,0.0);
        glVertex3f(5.0,-25,0.0); // right
        glVertex3f(5.0,25,0.0);
        glVertex3f(5.0,25,5.0); 
        glVertex3f(5.0,-25,5.0);
        glEnd();        

        glBegin(GL_POLYGON);
        glNormal3f(-1.0,0.0,0.0);
        glVertex3f(-5.0,-25,0.0); // left
        glVertex3f(-5.0,25,0.0);
        glVertex3f(-5.0,25,5.0);
        glVertex3f(-5.0,-25,5.0);
        glEnd();        

        glBegin(GL_POLYGON);
        glNormal3f(0.0,-1.0,0.0);
        glVertex3f(-5.0,-25,0.0); // transom
        glVertex3f(5.0,-25,0.0);
        glVertex3f(5.0,-25,5.0);
        glVertex3f(-5.0,-25,5.0);
        glEnd();        
                        
        glBegin(GL_POLYGON);
        glNormal3f(-0.89443f,0.44721f,0.0);
        glVertex3f(-5.0,25,0.0); // left bow
        glVertex3f(0.0,35,0.0);
        glVertex3f(0.0,35,5.0);
        glVertex3f(-5.0,25,5.0);
        glEnd();        

        glBegin(GL_POLYGON);
        glNormal3f(0.89443f,0.44721f,0.0);
        glVertex3f(5.0,25,0.0); // right bow
        glVertex3f(0.0,35,0.0);
        glVertex3f(0.0,35,5.0);
        glVertex3f(5.0,25,5.0);
        glEnd();        

        glBegin(GL_POLYGON);
        glNormal3f(0.0,0.0,1.0);
        glVertex3f(-4.0,0.0,10.0);  // cabin top
        glVertex3f(4.0,0.0,10.0);
        glVertex3f(4.0,25.0,10.0);
        glVertex3f(-4.0,25.0,10.0);
        glEnd();
        
                
        glBegin(GL_LINES);

        glVertex3f(-4.0,0.0,5.0);  // cabin verticals
        glVertex3f(-4.0,0.0,10.0);  // cabin verticals
        glVertex3f(4.0,0.0,5.0);
        glVertex3f(4.0,0.0,10.0);
        glVertex3f(-4.0,25.0,5.0);
        glVertex3f(-4.0,25.0,10.0);
        glVertex3f(4.0,25.0,5.0);
        glVertex3f(4.0,25.0,10.0);

        glEnd();
        
        glEnable(GL_LIGHTING);        
        glDisable(GL_LINE_SMOOTH);
    }
    glPopMatrix();
}

// documented in Object
int SwathStream::readFrom(FILE * infile)
{
    char instring[500];
    char fname[500] = "";
    double res = 1.0;
    double rad = 0.0;
    char type[100];

    char devicename[100];
    double lat,lon; // used to determine UTM zone for projection
    Point3d gr;
    
    while(fgets(instring, 500, infile) && strstr(instring,"BEGIN SWATH_STREAM\n")== NULL)
    {
    }
    while(!feof(infile))
    {
        int dummy = sscanf(instring,"FNAME %s ",fname);
        dummy = sscanf(instring,"FTYPE %s ",ftype);
        dummy = sscanf(instring,"RESOLUTION %lf ",&res);
        dummy = sscanf(instring,"RADIUS %lf ",&rad);
        dummy = sscanf(instring,"HANDLER %s ",type);
        dummy = sscanf(instring,"DEVICE %s ",devicename);
        dummy = sscanf(instring,"LAT %lf ",&lat);
        dummy = sscanf(instring,"LON %lf ",&lon);
        
        if(!fgets(instring, 500, infile))
            break;
    }
    
    if (strlen(fname)!=0)
    {
    
        if(strcmp(type,"swa")==0)
        {
            InitializeWA(rad);    
        }
        
        Initialize(res,type);

#ifndef WIN32        
//        if(strcmp(ftype,"raw")==0)
//        {
//            rawmb = new RawMultibeam(fname,devicename,lat,lon);
//            rawmb->GetGeoRef(geoRef);
//        }
#endif
        if(strcmp(ftype,"ascii")==0)
        {
            asciiIn = fopen(fname,"r");
            if(asciiIn)
            {
                if(fscanf(asciiIn,"Pos: (easting northing) %lf %lf ",&gr[0],&gr[1]) == 2)
                {
                    gr[2] = 0;
                    SetPosition(gr);
                }
                fseek(asciiIn,0,SEEK_SET); // resets the file
            }
            else
            {
                return 0;
            }
        }
    }
    return 0;
}

void SwathStream::Initialize(float resolution, const char * type)
{
    std::cerr << "Initializing SwathStream " << id << "\n";
    
    if(soundingtype == NULL)
    {
        soundingtype = new char[strlen(type)+1];
        strcpy(soundingtype,type);
    }

    std::cerr << "Res: " << resolution << " Type: " << soundingtype << "\n";
    
    if(quadtree == NULL)
    {
        quadtree = new SDV_Quadtree(resolution);
//        quadtree = new SDV_Tiles(resolution);
    }

    swathbuffer = new SwathBuffer(pose->getPosition(),100);
    degres = 1;
    degcount = 0;
    
    std::cerr << "Done SwathStream initialization\n";
}

void SwathStream::InitializeWA(double radius)
{
    SwathStream::radius = radius;
}

void SwathStream::Idle(double time)
{

    Object::Idle(time);

#ifndef WIN32
//    if(rawmb != NULL)
//    {
//	    LoadNext();
//    }
#endif
    if(asciiIn != NULL)
    {
        LoadNext();
    }
}

void SwathStream::LoadNext()
{
    Point3d p;
    int i;

    int scount;
    
    if(strcmp(ftype,"ascii")==0)
    {
        if(asciiIn != NULL)
        {
            if(!feof(asciiIn))
            {
                
                int dummy = fscanf(asciiIn,"Pos: (easting northing) %lf %lf ",&p[0],&p[1]);
                dummy = fscanf(asciiIn,"Attitude: (roll pitch heading heave) %lf %lf %lf %lf ",&roll,&pitch,&heading,&p[2]);
                
                pose->setPosition(p);
                
                dummy = fscanf(asciiIn,"Soundings: %d ",&scount);
                for (i=0; i<scount; i++)
                {
                    dummy = fscanf(asciiIn,"%lf %lf %lf ",&p[0],&p[1],&p[2]);
                    if(strcmp(soundingtype,"s")==0)
                    {
                        XYZPoint s;
                        COPY_POINT(s.sounding,p);
                        quadtree->AddNode(s,p[0],p[1]);
                    }
                    else if(strcmp(soundingtype,"sa")==0)
                    {
                        XYZPointAvg s;
                        COPY_POINT(s.sounding,p);
                        quadtree->AddNode(s,p[0],p[1]);
                    }        
                    else if(strcmp(soundingtype,"swa")==0)
                    {
                        XYZPointWA s;
                        COPY_POINT(s.sounding,p);
                        s.radius = radius;
                        quadtree->AddNodeToArea(s,p[0]-radius,p[1]-radius,p[0]+radius,p[1]+radius);
                    }        
                    
                    soundingcount++;
                }   
                swathcount++;
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            else
            {
                fclose(asciiIn);
                asciiIn = NULL;
            }
        }
    }

#ifndef WIN32    
/*   	if(strcmp(ftype,"raw")==0)
    {
        if(rawmb->GetNext() == 0)
        {
            for(i=0; i<rawmb->soundingCount;i++)
            {
                p[0] = rawmb->depths[i][0] - (*sGeoRef)[0];
                p[1] = rawmb->depths[i][1] - (*sGeoRef)[1];
                p[2] = rawmb->depths[i][2] - (*sGeoRef)[2];    
                
                if(strcmp(soundingtype,"s")==0)
                {
                    s = new XYZPoint;
                    s->sounding[0] = p[0];
                    s->sounding[1] = p[1];
                    s->sounding[2] = p[2];
                    quadtree->AddNode(s,p[0],p[1]);
                }
                else if(strcmp(soundingtype,"sa")==0)
                {
                    sa = new XYZPointAvg;
                    sa->sounding[0] = p[0];
                    sa->sounding[1] = p[1];
                    sa->sounding[2] = p[2];
                    quadtree->AddNode(sa,p[0],p[1]);
                }        
                else if(strcmp(soundingtype,"swa")==0)
                {
                    swa = new XYZPointWA;
                    swa->sounding[0] = p[0];
                    swa->sounding[1] = p[1];
                    swa->sounding[2] = p[2];
                    swa->radius = radius;
                    quadtree->AddNodeToArea(swa,p[0]-radius,p[1]-radius,p[0]+radius,p[1]+radius);
                }        

                soundingcount++;
                
            }
            swathcount++;

            p[0] = rawmb->shipEast - (*sGeoRef)[0];
            p[1] = rawmb->shipNorth - (*sGeoRef)[1];
            p[2] = rawmb->heave - (*sGeoRef)[2];
            
            roll = rawmb->roll;
            pitch = rawmb->pitch;
            heading = rawmb->heading;

            pose->setPosition(p);

            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
        }
    }*/
#endif
}

void SwathStream::ShowInfo(GLuint *names, int numNames)
{
    Scene3D *scene;
    
    char countstr[100];

    scene = mainWindow->getScene3D();

    sprintf(countstr,"Sounding count: %li",soundingcount);
    scene->appendInfo(countstr, GL_TRUE);
    
    sprintf(countstr,"Swath count: %li",swathcount);
    scene->appendInfo(countstr, GL_TRUE);

    scene->appendInfo("--- Generic Info:---\n");

    Object::ShowInfo(names, numNames);
}
