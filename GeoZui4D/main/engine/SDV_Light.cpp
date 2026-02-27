/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "engine/Scene3D.h"
#include "engine/SDV_Light.h"
#include <stdlib.h>
#include "engine/TranslatableState.h"
#include "widgets/Sv_Standards.h"
#include "engine/InputMediator.h"
#include "utilities/DottedAddress.h"

#define EXPERIMENTAL_SHADING


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
SDV_Light::SDV_Light(PosableStateBase *parent)
{
    // position will be used to specify the vector of the direction of the light source
    pose = new TranslatableState(this, "Light");

    if (parent)
        parent->getAddress()->addChild(pose->getAddress());

    //(mdp 8/4/2008) These should stay put--you can change these in Tcl if need be (gz posable set Window.main.Light xyz -0.5 -1.0 1.5 and gz posable set Window.main.Light hxyz -0.5 -1.0 1.5)
    pose->setPosition(-1.0, -1.0, 1.0);
    pose->setPosition(-1.0, -1.0, 1.0, ChangeHomeTranslate);
    lpos[3] = 0.0;

    mat_shininess[0]  = 128.0;

    mat_diffuse[0] = 1.0;
    mat_diffuse[1] = 1.0;
    mat_diffuse[2] = 1.0;
    mat_diffuse[3] = 1.0;

    mat_specular[0] = 0.5;
    mat_specular[1] = 0.5;
    mat_specular[2] = 0.5;
    mat_specular[3] = 1.0;

    //(mdp 8/4/2008) If you really need more ambient light, we should add options to change it.  Values above these destroy reasonable lighting for other applications/demos.
    light_ambient[0]  = 0.1;
    light_ambient[1]  = 0.1;
    light_ambient[2]  = 0.1;
    light_ambient[3]  = 1.0;

    light_diffuse[0]  = 0.8;
    light_diffuse[1]  = 0.8;
    light_diffuse[2]  = 0.8;
    light_diffuse[3]  = 1.0;

    light_specular[0] = 0.8;
    light_specular[1] = 0.8;
    light_specular[2] = 0.8;
    light_specular[3] = 1.0;

    fog_mode = GL_LINEAR;

    fog_color[0] = 0.0;
    fog_color[1] = 0.0;
    fog_color[2] = 0.0;
    fog_color[3] = 0.0;
    fog_start = 0.0;
    fog_end = 1.0;
    fog_density = 1.0;
}

/*---------------------------------------------------------------------------*/
SDV_Light::~SDV_Light()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void SDV_Light::Init()
{
    glFogi(GL_FOG_COORD_SRC,GL_FOG_COORD);

    update();
    gz4d::Point<double> lp = pose->getPosition();
    COPY_POINT(lpos, lp);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
}

/*---------------------------------------------------------------------------*/
void SDV_Light::Draw()
{
    gz4d::Point<double> lp = pose->getPosition();
    COPY_POINT(lpos, lp);
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);

}

/*---------------------------------------------------------------------------*/
void SDV_Light::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
    globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
}

int SDV_Light::perform(int argc, const char **argv, Tcl_Interp *interp)
{

    std::stringstream retString;
    if (argc)
    {
        std::string cmd(argv[0]);
        switch (argc)
        {
            case 1:
                if (cmd == "ambient")
                {
                    retString << light_ambient[0] << " ";
                    retString << light_ambient[1] << " ";
                    retString << light_ambient[2] << " ";
                    retString << light_ambient[3];
                } else if (cmd == "diffuse")
                {
                    retString << light_diffuse[0] << " ";
                    retString << light_diffuse[1] << " ";
                    retString << light_diffuse[2] << " ";
                    retString << light_diffuse[3];
                } else if (cmd == "specular")
                {
                    retString << light_specular[0] << " ";
                    retString << light_specular[1] << " ";
                    retString << light_specular[2] << " ";
                    retString << light_specular[3];
                }
                else
                {
                    break;
                }
                Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                return TCL_OK;
            case 2:
                if (cmd == "fog")
                {
                    if(std::string(argv[1]) == "color")
                    {
                        retString << fog_color[0] << " ";
                        retString << fog_color[1] << " ";
                        retString << fog_color[2] << " ";
                        retString << fog_color[3];
                    }
                    else if(std::string(argv[1]) == "start")
                    {
                        retString << fog_start;
                    }
                    else if(std::string(argv[1]) == "end")
                    {
                        retString << fog_end;
                    }
                    else if(std::string(argv[1]) == "density")
                    {
                        retString << fog_density;
                    }
                    else if(std::string(argv[1]) == "mode")
                    {
                        retString << fog_mode;
                    }
                }
                else
                    break;
                Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                return TCL_OK;
            case 3:
                if (cmd == "fog")
                {
                    if(std::string(argv[1]) == "start")
                    {
                        fog_start = atof(argv[2]);
                    }
                    else if(std::string(argv[1]) == "end")
                    {
                        fog_end = atof(argv[2]);
                    }
                    else if(std::string(argv[1]) == "density")
                    {
                        fog_density = atof(argv[2]);
                    }
                    else if(std::string(argv[1]) == "mode")
                    {
                        fog_mode = atoi(argv[2]);
                    }
                }
                else
                    break;
                update();
                globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
                Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                return TCL_OK;
            case 5:
                if (cmd == "ambient")
                {
                    light_ambient[0] = atof(argv[1]);
                    light_ambient[1] = atof(argv[2]);
                    light_ambient[2] = atof(argv[3]);
                    light_ambient[3] = atof(argv[4]);
                } else if (cmd == "diffuse")
                {
                    light_diffuse[0] = atof(argv[1]);
                    light_diffuse[1] = atof(argv[2]);
                    light_diffuse[2] = atof(argv[3]);
                    light_diffuse[3] = atof(argv[4]);
                } else if (cmd == "specular")
                {
                    light_specular[0] = atof(argv[1]);
                    light_specular[1] = atof(argv[2]);
                    light_specular[2] = atof(argv[3]);
                    light_specular[3] = atof(argv[4]);
                }
                else
                {
                    break;
                }
                update();
                globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
                Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                return TCL_OK;
            case 6:
                if (cmd == "fog" && std::string(argv[1]) == "color")
                {
                    fog_color[0] = atof(argv[2]);
                    fog_color[1] = atof(argv[3]);
                    fog_color[2] = atof(argv[4]);
                    fog_color[3] = atof(argv[5]);
                }
                else
                {
                    break;
                }
                update();
                globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
                return TCL_OK;
            default:
                break;
        }
    }
    retString << "sub commands:\n  ambient [r g b a]\n  diffuse [r g b a]\n  specular [r g b a]\n";
    retString << "  fog mode [m]\n      start [s]\n      end [e]\n      density [d]\n      color [r g b a]";
    Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
    return TCL_ERROR;
}

void SDV_Light::update()
{
     glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
     glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
     glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

     glLightfv(GL_LIGHT0,GL_POSITION,lpos);
     glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
     glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
     glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);

     glFogi(GL_FOG_MODE, fog_mode);
     glFogfv(GL_FOG_COLOR,fog_color);     // Set the fog color 
     glFogf(GL_FOG_START, fog_start);
     glFogf(GL_FOG_END, fog_end);
     glFogf(GL_FOG_DENSITY, fog_density);
}

