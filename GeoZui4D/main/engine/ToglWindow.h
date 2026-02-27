/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ToglWindow.h
///////////////////////////////////////////////////////////////////////////////
/**
 *  This is the abstract class that encapsulates a 3-D OpenGL window using
 *  Togl. Child classes are responsible for their own physical instantiation
 *  This class implements the following responsibilities:
 *  *   Initialize Tcl/Tk interpreter, and ToglWindow commands
 *  *   Facilitate input directed to the window (partially private)
 *  *   Provide easy invocation of certain Tcl/Tk functions, with error reports
 *  *   Provide a way to move the cursor to the desired position in this window.
 *
 *  Created by mdp 7/14/2000
 */
///////////////////////////////////////////////////////////////////////////////

class ToglWindow; //this line connects the above comments with this class.

#ifndef TOGLWINDOW_H
#define TOGLWINDOW_H

#include<gz4d/glo.h>

#include "Togl/togl.h"

#include <tcl.h>
#include <tk.h>

// X11/Xlib.h defines 'Complex' as a macro (= 0), which conflicts with
// Boost.Math template parameter names. Undefine it here.
#ifdef Complex
#undef Complex
#endif

#include "engine/InputMediator.h"

class SDV_Cursor;
class DottedAddress;

extern int dominantEye;

#include <GL/glut.h>
#include <gz4d/glo/Context.h>

#ifndef GLUT_BITMAP_HELVETICA_10
#define GLUT_BITMAP_HELVETICA_10    ((void*)6)
#endif

class ToglWindow : public Interactor
{
public:
    /*---Initialization---*/
    ToglWindow(std::string name);
    Togl *GetTogl();
    virtual void Init(int argc = 0, char *argv[] = NULL);

    /*---Context validation---*/
    GLboolean establishValidContext();
    static GLboolean contextIsValid();
    GLboolean isBaseThread();
    
    /*---Input Facilitation---*/
    InputMediator inputMediator;
    virtual void startPickingRound(unsigned char) {};
    int getWidth();
    int getHeight();
    int getMinDimension();

    /*---Tcl Utility Procedures---*/
    static int Tcl_Eval(std::string const &command, std::string &retval);
    static int Tcl_Eval(std::string const &command){std::string s; return Tcl_Eval(command,s);}

    int Tcl_LinkVar(char const *tclVar, void *address, int type = TCL_LINK_INT, GLboolean addToGlobal = GL_TRUE);
    int Tcl_UnlinkVar(char const *tclVar);
    int Tcl_UpdateLinkedVar(char const *tclVar);

    static int Tcl_LinkVar_Root(char *tclVar, void *address, int type = TCL_LINK_INT);
    static int Tcl_UnlinkVar_Root(char *tclVar);
    static int Tcl_UpdateLinkedVar_Root(char *tclVar);

    /*---Move Mouse---*/
    int MoveMouse(int x, int y);
    static GLboolean toggleMouseTracking();
    static GLboolean getMouseTracking();

    /*---Name Lookup---*/
    static ToglWindow *GetToglWindow(Tcl_Interp *interp, const char *name);

    /*---GLobal interp---*/
    static Tcl_Interp * interp;

    /*---Bitmap/Text positioning---*/
    static GLboolean beginRaster2D(float rx, float ry);
    static GLboolean beginRaster2D(float &x, float &y, float &z);
    static void endRaster2D();

    /*---Textual output---*/
    bool printAt(float rx, float ry, std::string text, GLboolean shadow = GL_TRUE, void *font = GLUT_BITMAP_HELVETICA_10); //added by mdp 1/4/2007
    bool printAt(float x, float y, float z, std::string text, GLboolean shadow = GL_TRUE, void *font = GLUT_BITMAP_HELVETICA_10, int xOffset = 0, int yOffset = 0, GLboolean atScreenDepth = GL_FALSE);
    void printOver(float x, float y, float z, std::string text, GLboolean shadow = GL_TRUE, void *font = GLUT_BITMAP_HELVETICA_10, int xOffset = 0, int yOffset = 0) {printAt(x, y, z, text, shadow, font, xOffset, yOffset, GL_TRUE);}
    static int printingWidth(std::string text, void *font = GLUT_BITMAP_HELVETICA_10);

    /*---Window activation---*/
    static void SetTogl(Togl * togl);

    char * GetIORFilename();

    int getSlavePort(){return slavePort;}
    int getMasterPort(){return masterPort;}
    char * getMasterAddress(){return masterAddress;}
    int getIOPort(){return ioPort;};

    //Display management
    virtual void Zap();
    virtual void Reshape(int x = 0, int y = 0, int w = 0, int h = 0);
    virtual void Display() = 0;
    static int SubCommandCallBack(struct Togl *togl, int argc, char *argv[]);
    virtual int SubCommand(int argc, char *argv[]);
    virtual int HandleInput(unsigned int inputType, InputInfo *info);

    /*---Access to flag values and text output in ftvr mode---*/
    GLboolean isHeadtrackingEnabled() const {return (headtracking_flag == 1);}
    GLboolean isFishtankVREnabled() const {return (fishtankVR_flag == 1);}
    void printFtvrAt(float x, float y, float z, std::string text, GLboolean shadow, GLfloat pitch_adj_angle = 0.0f);

    void BackgroundColor(double r, double g, double b);
    char * GetDatadir();
    char * GetAppdir();
    int isRTVideoDisabled();
    gz4d::glo::Context::Ptr getGloContext() const {return context;}
    
protected:
    static int stereo_flag;   //everyone changes if one changes
    static int oneEyedFlag;  //shared with tcl/tk to select sw or hw cursor
    static int ssstereo_flag;   //if in stereo mode, selects side by side stereo
    static int fishtankVR_flag;
    static int headtracking_flag;

    static char * datadir;
    static char * appdir;

    static int masterPort;
    static char * masterAddress;
    static int slavePort;
    
    static int ioPort;

    static int screenWidth_pix;
    static int screenHeight_pix;
    static double screenLeft;
    static double screenRight;
    static double screenTop;
    static double screenBottom;
    static double eyeX;
    static double eyeY;
    static double eyeZ;
    static int mainRight; // is the right screen in a dual screen config the main one? If not, the left one must be main.

    static SDV_Cursor *sdvcursor;
    int width, height;
    int xOffset, yOffset;
    static float bgColor[4];

    Togl * myTogl;
    static GLboolean toglIsValid;
    
    static int disableRTVideo;

    //Initialization information
    static int tkInitialized;
    static ToglWindow *creatingWindow;
    virtual void initTkCallBack(Tcl_Interp *interp) {};

    static std::string setupScript;

    gz4d::glo::Context::Ptr context;
private:
    //One-time initialization
    static int InitTkCallBack(Tcl_Interp *interp) ;

    //Per-instance initialization
    static void InitToglCallBack( struct Togl *togl );
    void InitTogl( struct Togl *togl );

    //Input Event handlers
    static void ZapCallBack( struct Togl *togl );
    static void ReshapeCallBack( struct Togl *togl);
    static void DisplayCallBack(struct Togl *togl);

    int Idle(int argc, char *argv[]);
    int Mouse(int argc, char *argv[]);
    int Motion(int argc, char *argv[]);
    int Keyboard(int argc, char *argv[]);
    int BackgroundColor(int argc, char *argv[]);

    int InputCallback(int inputType, InputInfo *info);

    /*---Name Lookup---*/
    static ToglWindow *pointerForRetrieval;
    int PointerForRetrieval(int argc, char *argv[]);

    /*---Idle Event Recording---*/
    static long idleEventCount;
};

extern DottedAddress *InteractorWindowNeighborhood;

#endif
