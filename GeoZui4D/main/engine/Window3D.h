/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// Window3D.h
///////////////////////////////////////////////////////////////////////////////


//class ViewManager; //this line connects the above comments with this class.

#ifndef WINDOW3D_H
#define WINDOW3D_H

#include "engine/ToglWindow.h"

#include <gz4d/Point.h>

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

#include <list>
#include <memory>
#include <gz4d/Color.h>
#include "SDV_types.h"

class Scene3D;
class Sv_Widgets;
class ViewNudger;
class Object;
class ViewManager;
class SubWindow;
class SDV_Light;
class DottedAddress;
class CursorObject;
class InputTentacle;
class Notes;
class TextureMap;
class PositionDrag;
class VideoRecorder;
class ZoomInteractor;
class PosableStateBase;

/**
 *  This is the abstract class that encapsulates a 3-D OpenGL window using
 *  Togl for displaying object lists and using Sv_Widgets for navigation.
 *  Child classes are responsible for their own physical instantiation.
 *  \note This class implements the following responsibilities:
 *  \li Facilitate view manipulations
 *  \li Provide vital view information (center of view, size of pixel, etc.)
 *  \li Provide ways to add objects and subwindows to the Window3D's Scene3D
 *  \li Facilitate input directed to the window (inherited)
 *  \li Allocate and manage subwindow views.
 *
 *  \author Matt Plumlee (mdp)
 *  \date 7/14/2000
 *  \image html viewpointSubsystem.jpg
 */
///////////////////////////////////////////////////////////////////////////////

class Window3D : public ToglWindow
{
public:
    /*---Initialization---*/
    Window3D(std::string name, Window3D *myParent = NULL);
    ~Window3D();
    virtual void Init(int argc = 0, char *argv[] = NULL);
    virtual void publishName(std::string name);

    /*---Casting---*/
    static Window3D *cast(ToglWindow *);

    /*---View Manipulation Functions---*/
    void PushTransform(long gl_side, int clearDepth = 0, GLboolean fromScratch = GL_TRUE);
    void PopTransform();
    int EnableClippingPlanes();
    void DisableClippingPlanes();
    void BlankLowResResidue();
    SDV_Light *getLight() {return light;}

    /*---Viewpoint information---*/
    ViewManager *getViewInfo() { return currentView;}
    PosableStateBase *getPoseInfo() { return currentPose;}
    GLboolean getSelectedLocation(InputInfo *info, gz4d::Point<double> &worldPos, double *depth = NULL, GLboolean allowRedisplay = GL_TRUE);
    void updatePosableState();
    void SetScale(GLfloat sval);
    
    /*---Proxy representations---*/
    virtual void DrawIn(Window3D *inWindow);
    virtual void PerformDrawIns();

    /*---Object Bookkeeping Functions---*/
    virtual void setScene3D(Scene3D *s3d);
    Scene3D *getScene3D();
    virtual void addObject(Object *object, int flags = 0);

    /*---Input Management---*/
        virtual void startPickingRound(unsigned char pickType);
    virtual void enablePicking();

    /*---Display management---*/
    virtual void Reshape(int x = 0, int y = 0, int w = 0, int h = 0);
    virtual void Display();
    virtual int getX(GLboolean relativeToMain = GL_FALSE) {return 0;}
    virtual int getY(GLboolean relativeToMain = GL_FALSE) {return 0;}
    short getTransparencyLevel() {return transparencyLevel;} //indicates the current transparency level at which objects are being drawn. Added by rjk 2/5/02 to support semi-transparent objects in haptics.  Put into common use by mdp 1/28/2004.
    void setTransparencyLevel(short l) {transparencyLevel = l;} //allows temporary transparency level changes to let ObjectCollections pass 0 on to children. (mdp 2/12/2004).
    void requestUpdate() {updateRequired = GL_TRUE;}

    /*---Rendering Priority Management---*/
    void setDefaultPriority(float p);
    float getDefaultPriority() {return defaultPriority;}
    void setCurrentPriority(float p);
    void restoreDefaultPriority() {setCurrentPriority(defaultPriority);}
    void setPriorityInteractive();

    /*---Subwindow Management---*/
    Window3D *GetParent() {return parent;}
    void AddSubwindow(SubWindow *subwin = NULL);
    void RemoveSubwindow(SubWindow *subwin);
    void DrawSubwindows(long gl_side);
    int ShowOverviewWindow();
    int RestoreSubwindows();
    static unsigned int windowChangedEvent() { return windowChangedInputType->value;}
    void registerSubWindowVisibility(GLboolean visibility);
    void requestNewSubwindow(Object * creator, Window3D * parentWindow);
    static void setBridgeView(bool on = true);

    /*---Common 3D Cursor---*/
    static CursorObject *linkageCursor;

    /*---Cursor Info---*/
    gz4d::Point<double> getCursorPosOnSurface(GLboolean allowRedisplay = GL_FALSE);
    GLboolean getCursorPosOnPlane(double &x, double &y, double z);
    gz4d::Point<double> getCursorPosOnSurfaceOrPlane(double z, GLboolean allowRedisplay = GL_FALSE);

    /*---Mpeg output---*/
    int StartVideoRecorder(int argc, const char *argv[]);
    int AddFrame(int argc, const char *argv[]);
    int RecordFrame(int argc, const char *argv[]);
    bool isRecording();
    int StopVideoRecorder(int argc, const char *argv[]);

    void ShowWatermark(bool s = true){showWatermark = s;};
    void ShowCompass(bool s = true){showCompass = s;};

    void ScreenShot(int w, int h, bfs::path fname);

    Notes * GetNotes(){return notes;};
    
    bool globalPrintAt(double x, double y, double z, char *string, GLboolean shadow = GL_TRUE, void *font = GLUT_BITMAP_HELVETICA_10, int xOffset = 0, int yOffset = 0, GLboolean atScreenDepth = GL_FALSE);
    
    static void exitNextIdle() {exitFlag = true;}

protected:
    int         FullScreen;
    int         savewidth, saveheight;
    int         xpos, ypos;
    GLfloat     lpos[4];
    short       widgetStatus;
    ViewManager *currentView;
    PosableStateBase *currentPose;
    Scene3D     *scene3d;
    Window3D    *parent, *debugWindow;
    GLdouble    *clippingPlane[2];

    //priority rendering
    float       currentPriority, defaultPriority;
    float       priorityReserve; //currentPriority accumulated over each frame since last render
    static float currentPriorityTotal;
    static int  priorityParticipants, usePriorities;

    // video recording command handling functions
    int VideoRecorderCommand(int argc, const char *argv[]);
    void drawOrtho();
    void drawWatermark();
    void drawCompass(bool resetAlpha = false);
    void DrawSurface();

    //cursor tracking
    int         cursorX, cursorY;
    GLboolean   cursorValid;

    int         trackCursorAtSurface;
    gz4d::Point<double> surfaceCursorPos;
    GLboolean   surfaceCursorPosValid;

    int         trackCursorAtZ;
    gz4d::Point<double> cursorPosAtScreen, cursorPosUnderScreen, cursorPosAtZ;
    double zDiffCursor;
    short   cursorPosAtZValid;

    //Interactors;
    Sv_Widgets  *widgets;
    ZoomInteractor *zoomInteractor;
    ViewNudger  *viewNudger;
    InputTentacle *mouseMotionGrabber;
    PositionDrag *positionDragger;

    //Initialization information
    static int windowToglInitialized;

    //Input
    int HandleInput(unsigned int inputType, InputInfo *info);
    virtual void PostOptionMenu(int x, int y);

    //Display management
    virtual void Idle();
    virtual int SubCommand(int argc, char *argv[]);

    //Rendering
    int _FirstIdleCall;
    int doneDrawingHighRes;
    bool showAsWireframe;
    short transparencyLevel;
    static std::shared_ptr<TextureMap> watermark, compass, compassPoints;
    bool showWatermark, showCompass;
    float waterX, waterY, waterS; //watermark position in 0-to-1 coordinates, and the # of sections to divide the screen (larger waterS => smaller watermark)
    float compassX, compassY, compassS; //compass position in 0-to-1 coordinates, and the # of sections to divide the screen (larger compassS => smaller compass)
    static float innerCompassX[16], innerCompassY[16];
    static float outerCompassX[16], outerCompassY[16];
    static float compassAlpha;

    //Subwindow management
    int updateRequired;
    struct Node
    {
        SubWindow* window;
        Node* next;
        Node* prev;
    };
    Node *subWindows, *subWindowTail;
    SubWindow *overviewWindow;
    int subWindowCount, visibleSubwindowCount;
    static ToglWindow *otherMovieWindow;
    static bool otherMovieWindowIsAbove;
    static bool bridgeView;

    static const EventType *windowChangedInputType;


    Notes * notes;
    
    //video recording
    std::shared_ptr<VideoRecorder> vrec; //changed to a shared_ptr by mdp 2/26/2009 to cut off an annoying include chain
    gz4d::Color surfaceColor;
private:
    //Togl commands
    int LockToCommand(int argc, char *argv[]);

    static bool exitFlag;

    SDV_Light * light;
};

extern DottedAddress *WindowNeighborhood;

#endif
