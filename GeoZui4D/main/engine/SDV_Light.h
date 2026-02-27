/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef SDV_LIGHT_H
#define SDV_LIGHT_H

#include "PosableStateUser.h"

class SDV_Light : public PosableStateUser
{
public:
    SDV_Light(PosableStateBase *parent = NULL);
    ~SDV_Light();

    void PoseChanged(PosableStateBase *whichPose, long whatChanged);

    void Init();
    void Draw();
    int perform(int argc, const char **argv, Tcl_Interp *interp);
private:
    void update();

    GLfloat lpos[4];  // the light position
    float mat_shininess[1];
    float mat_diffuse[4];  // the texture replaces this
    float mat_specular[4];

    
    float light_ambient[4];	
    float light_diffuse[4];
    float light_specular[4];
    
    float fog_color[4];
    float fog_start;
    float fog_end;
    float fog_density;
    GLuint fog_mode;
};

#endif
