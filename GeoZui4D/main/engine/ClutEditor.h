// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// ClutEditor.h
///////////////////////////////////////////////////////////////////////////////

class ClutEditor; //this line connects the above comments with this class.

#ifndef CLUTEDITOR_H
#define CLUTEDITOR_H

#include "engine/TextureMaps/Clut.h"
#include "engine/ToglWindow.h"

class Scene3D;
/**
 *  This class implements a Color Look-Up Table editor as a ToglWindow.
 *  This class implements the following responsibilities:
 *  \li Initialize Tcl/Tk clut commands
 *  \li Allow interactive modification of cluts
 *
 *  Derived from ToglWindow
 *  \author (commented by) Matt Plumlee (mdp)
 *  \date 7/14/2000
 */
///////////////////////////////////////////////////////////////////////////////
class ClutEditor : public ToglWindow
{
public:
    /*---Initialization/Deinitialization---*/
    ClutEditor(std::string name, Tcl_Interp *interp, std::shared_ptr<Clut> &c, GLboolean asColorKey = GL_FALSE);
    virtual void Init(int argc = 0, char *argv[] = NULL);

    ~ClutEditor();

    //Clut editing
    void setScene3D(Scene3D *s3d);
    void SetTargetToglPtr(Togl * togl);
    void SetClut(Tcl_Interp *interp, std::shared_ptr<Clut> &c);
    void GetRGB(Point4ub color, float * hsv);
    void GetHSV(Point4ub color, float * hsv);
    virtual int SubCommand(int argc, char *argv[]);
    void AddElement(Tcl_Interp *interp, float height);
    void DeleteElement(Tcl_Interp *interp);

    //Display management
    void Update(Tcl_Interp *interp, float height);
    void Display();
    void Reshape(int x = 0, int y = 0, int w = 0, int h = 0);
    virtual int HandleInput(unsigned int inputType, InputInfo *info);
    void Mouse(int button, int state, int x, int y);
    void Motion(int x, int y);

    //Hack to change the font
    static void *displayFont;

protected:
    float zoommin, zoommax;
    float min, max, range;
    clutnode * element;
    std::shared_ptr<Clut> clut;

    //color-key vars
    float keyStart, keyStop, keyStep, currentKey, keyNumSteps;
    int currentKind;
    float bgStrength, fgStrength;
    GLboolean aligned;
    enum {NoUnitLabels, MeterLabels, FeetLabels};
    int unitLabels;
    float widthBuffer;

    float elementheight, elementLength;

private:
    //Togl Commands & variables
    static int clutToglInitialized; // have we registered the Togl commands yet?
    Togl *targetTogl;
    Scene3D *scene3d;

    int InitTexture(int argc, char *argv[]);
    int LoadTexture(int argc, char *argv[]);
    int SaveTexture(int argc, char *argv[]);
    int DeleteTexture(int argc, char *argv[]);
    int DefaultTexture(int argc, char *argv[]);
    int UseDefaultTexture(int argc, char *argv[]);
    int UseCustomTexture(int argc, char *argv[]);
    int DisplayTexture(int argc, char *argv[]);
    int EditClut(int argc, char *argv[]);
    int AddClutElement(int argc, char *argv[]);
    int SetClutElement(int argc, char *argv[]);
    int DeleteClutElement(int argc, char *argv[]);
    int ModifyClutExtents(int argc, char *argv[]);
    int CreateClutRange(int argc, char *argv[]);
    int ClearClut(int argc, char *argv[]);
    int KeyDisplay(int argc, char *argv[]);
    int ClutElement(int argc, char *argv[]);
    
    //Togl support commands
    void SetClutEdTogl();
    void SetTargetTogl();

    void SetClutEdTogl(Tcl_Interp *interp);
    void SetTargetTogl(Tcl_Interp *interp);

    //Internal variables
    int zoom1,zoom2;
    int zooming;
    int ract, gact, bact;  // rgb sliders active (only one at a time!)
    int hsact, vact; //hs and v active
    unsigned char rcb,gcb,bcb; // clip board values
    float hsv[3];
    GLboolean colorKeyMode;
};

#endif
