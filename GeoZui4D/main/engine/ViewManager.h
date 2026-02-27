/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ViewManager.h
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
class ViewManager; //this line connects the above comments with this class.

#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

#include <gz4d/gz4d.h>
#include <gz4d/util/Utilities.h>
#include "SDV_types.h"
#include "engine/Interactor.h"
#include "PosableStateUser.h"
#include "interactors/WindowAnchor.h"
#include <gz4d/Point.h>

class Window3D;
class PosableStateLock;
class Object;

#include "Isotrack.h"

struct ViewIntersection;
/**
 *  This class implements a viewpoint object for use in rendering GL scenes.
 *  A ViewManager object encapsulates position, rotation, and scaling
 *  information, relative to a particular reference point.
 *  This class implements the following responsibilities:
 *  \li Maintain viewing frustum information.
 *  \li Maintain position, rotation, and scaling information.
 *  \li Apply GL tranformations based on the viewpoint information.
 *  \li Retain a modifiable "home" setting for restoring a previous viewpoint.
 *  \li Provide mechanisms for coordinating multiple interactors operating on
 *          a single ViewManager.
 *  \li Provide a way to lock views together in different ways
 *
 *  \author Matt Plumlee (mdp)
 *  \date 7/20/2000
 */
class ViewManager : public Interactor, public PosableStateUser
{
    public:
        /*---Initialization and destruction---*/
        ViewManager(const int *widthPtr, const int *heightPtr, Window3D *parentWin,
            int * ptr_ftVR_flag = NULL, int * ptr_ht_flag = NULL, int * ptr_s_flag = NULL, int * ptr_sss_flag = NULL);
        void reset(); //everything goes back to (0,0,0), etc.
        ~ViewManager(void);

        /*---Basic Mutators---*/
        ViewManager &operator =(const ViewManager &mgr);

        /*---Viewing Frustum Operations---*/
        void resetViewportOffsets(int x = 0, int y = 0) {offX = x; offY = y;}
        void applyFrustum(GLboolean fromScratch = GL_TRUE);
        void setEye(long gl_side, GLboolean fromScratch = GL_TRUE);
        GLint getEye(){return whichEye;};
        static GLboolean viewMovesWithWindow;
        void setDistanceToFarPlane(float distance);
        double getDistanceToFarPlane() {return distanceToFarPlane;}
        void setPerspective(int pflag = GL_TRUE);
        int getPerspective() {return perspectiveFlag;}
        void setPerspectiveWidthRatio(float r);
        float getPerspectiveWidthRatio() {return perspectiveWidthRatio;}
        void setPerspectiveHeightRatio(float r);
        float getPerspectiveHeightRatio() {return perspectiveHeightRatio;}

        /*---Viewing Frustum Locations---*/
        enum {FRONT_CORNERS = 0x00,
              BOTTOM_CORNERS= 0x00,
              LEFT_CORNERS  = 0x00,
              BACK_CORNERS  = 0x01,
              TOP_CORNERS   = 0x02,
              RIGHT_CORNERS = 0x04,
              FRONT_BOTTOM_LEFT = 0x00, //000
              BACK_BOTTOM_LEFT,         //001
              FRONT_TOP_LEFT,           //010
              BACK_TOP_LEFT,            //011
              FRONT_BOTTOM_RIGHT,       //100
              BACK_BOTTOM_RIGHT,        //101
              FRONT_TOP_RIGHT,          //110
              BACK_TOP_RIGHT            //111
        };
        gz4d::Point<double> const &getCorner(int corner);
        double getZComponentOfUnderPlane();
        double getZComponentOfAbovePlane();
        void updateCornerInfo(void);

        /*---Viewing Frustum Intersection---*/ //added by mdp 5/26/2004
        enum {BOTTOM_LEFT = 0,  //Edges.  They are in the following order,
              FRONT_LEFT,       //according to the corner enum above:
              FRONT_BOTTOM,     //Start from corner 000, and generate
              BACK_LEFT,        //edges by flipping least significant 0-bit
              BACK_BOTTOM,      //that hasn't been flipped for this corner yet.
              TOP_LEFT,         //When gotten through all 3 bits, increment and
              FRONT_TOP,        //repeat.  In other words, the edges are in the
              BACK_TOP,         //following order:  000-001, 000-010, 000-100,
              BOTTOM_RIGHT,     //001-011, 001-101, 010-011, 010-110, 011-111,
              FRONT_RIGHT,      //100-101, 100-110, 101-111, 110-111
              BACK_RIGHT,
              TOP_RIGHT
        };
        ViewIntersection *getIntersectionAtPlaneZ(double zBottom, double tolerance, ViewIntersection *vi = NULL); //returns NULL if no intersection
        ViewIntersection *getIntersectionAtPlaneZ(double zBottom, double zTop, double tolerance, ViewIntersection *vi = NULL); //returns NULL if no intersection
        GLboolean getIntersectionAtLineY(ViewIntersection *vi, double yLess, double yMore = std::numeric_limits<double>::quiet_NaN());
	int getScreenWidth() {return *screenWidth_pix;}
	int getScreenHeight() {return *screenHeight_pix;}
	double getScreenLeft() {return *screenLeft;}
	double getScreenRight() {return *screenRight;}
	double getScreenTop() {return *screenTop;}
	double getScreenBottom() {return *screenBottom;}
	double getEyeX() {return *eyeX;}
        double getEyeY() {return *eyeY;}
        double getEyeZ() {return *eyeZ;}

        /*---GL Transformations---*/
        void applyTransforms(GLboolean fromScratch = GL_TRUE);
        void pushView(GLboolean fromScratch = GL_TRUE);
        void popView(void);
        GLfloat getHeightExaggeration() {return (usingLocalExaggeration? localHeightExaggeration : heightExaggeration);}
        void setHeightExaggeration(GLfloat heightEx);
        void setLocalHeightExaggeration(GLfloat heightEx = 0.0f);

        int getWindowPoint(gz4d::Point<double> &screenPos, gz4d::Point<float> &winPosL);
        int getWorldPoint(gz4d::Point<double> &screenPos, gz4d::Point<double> &worldPos);
        int plotWorldPoint(gz4d::Point<double> const &worldPos, gz4d::Point<double> &screenPos);
        GLboolean pointIsOnScreen(gz4d::Point<double> const &worldPos, int bufferPixels = 0, GLdouble *depth = NULL);

        /*---Viewpoint information---*/
        void updateViewInfo(void);
        const GLdouble *getProjectionMatrix(GLboolean UpdateIfNecessary = GL_TRUE);
        const GLdouble *getModelMatrix(GLboolean UpdateIfNecessary = GL_TRUE);
        const GLint *getViewport(GLboolean UpdateIfNecessary = GL_TRUE);
        PosableStateBase *getPerspectivePosable() {return perspectivePosable;}
        GLdouble getDistanceToScreen(void);
        GLdouble getCenterToScreenDistance(void);
        GLdouble getCenterToEyepoint(void);

        /** GetPixelSize returns the size of a pixel (located in the screen plane,
         *  near the center of the screen) in world coordinates.  Updates the information
         *  first if necessary.
         *
         *  @return             Pixel size in world coordinates.
         */
        GLdouble getPixelSize(void);

        /** GetPixelSizeAt returns the size of a pixel located at the specified
         *  location in world coordinates.  Updates the information first if necessary.
         *
         *  @param  worldPos    Point at which the pixel size query is being made.
         *  @return             Pixel size in world coordinates at that point.
         */
        GLdouble getPixelSizeAt(gz4d::Point<double> const &worldPos);

        static const GLdouble radiusOfWorkspaceCenter;

        int             viewMatriciesValid;      //are current viewMatricies valid?

        void viewUpdated() {viewInfoCurrent = extraViewInfoCurrent = false;/* cornerInfoCurrent = GL_FALSE;&&& Need this for movies if can get viewInfo actually working!!?*/}

        /** GetReferencePoint returns a coordinate triple indicating the current
         *  locatoin of the reference point in world coordinates.
         *
         *  @return             Reference point coordinate triple
         */
        gz4d::Point<double> const &getReferencePoint();

        /** GetScreenCenterPoint returns a coordinate triple indicating the current
         *  center of the workspace (crosshairs of the widgets) in screen coordinates.
         *
         *  @return             Centerpoint coordinate triple (screen coords).
        */
        gz4d::Point<double> getScreenCenterPoint();

        /** getEyePoint returns a coordinate triple indicating the current
         *  location of the eyepoint in world coordinates.
         *
         *  @return             Eyepoint coordinate triple
         */
        gz4d::Point<double> getEyePoint();

        int get_ssstereo_flag() {return *ptr_ssstereo_flag;}
        bool usingFishtankVR() const {return *ptr_fishtankVR_flag == 1;}
        void setMeasurments(int *x, int *y, double *l, double *r, double *t, double *b, int *mr);
        void setEyeX(double *eX) {eyeX = eX;}
        void setEyeY(double *eY) {eyeY = eY;}
        void setEyeZ(double *eZ) {eyeZ = eZ;}

        /*---Proxy representations---*/
        void DrawIn(Window3D *inWindow);
        void enablePicking();
        void cancelAnchor(Window3D *inView);
        int showProxy(int show = -2) {return anchor->showProxy(show);}
        GLboolean pickProxy(int pick = -2) {return anchor->pickProxy(pick);}
        GLboolean showTethers(int show = -2) {return anchor->showTethers(show);}
        GLboolean showFlagpole(int show = -2) {return anchor->showFlagpole(show);}
        GLboolean showEyepoint(int show = -2) {return anchor->showEyepoint(show);}
        GLboolean showCenter(int show = -2) {return anchor->showCenter(show);}
        float proxyLineWidth(float width = 1.0f) {return anchor->proxyLineWidth(width);}
        float proxyStrength(float strength = 1.0f) {return anchor->proxyStrength(strength);}
        float tetherStrength(float strength = 1.0f) {return anchor->tetherStrength(strength);}
        float flagpoleStrength(float strength = 1.0f) {return anchor->flagpoleStrength(strength);}
        float centerStrength(float strength = 1.0f) {return anchor->centerStrength(strength);}

        /*---Interaction facilitation---*/
        virtual int HandleInput(unsigned int inputType, InputInfo *info);
        void setVisible(GLboolean vis = GL_TRUE);
        void setMinimized(GLboolean state = GL_TRUE);
        virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);
        virtual void PoseContinuouslyChanging(PosableStateBase *whichPose, GLboolean isChanging = GL_TRUE);
        Window3D *getParentWin() {return parent;} //added by mdp 4/7/2001

        /*---View Locking---*/
        void lockTo(ViewManager *);
        void lock(long lockType);
        void unlock(long lockType);
        long getLockType();
        static unsigned int viewChangedEvent() { return viewChangedInputType->value;}
        static unsigned int viewStabilizedEvent() { return viewStabilizedInputType->value;}
        static unsigned int spaceWarpedEvent() { return spaceWarpedInputType->value;}
        void setView(long lockType);
        void lockTo(Object *, long lockType = -1);
        void unlockFromObject();
        PosableStateBase *getObjectLockedTo();
        PosableStateLock *getLockToParent() {return lockToParent;}
        PosableStateLock *getLockToObject() {return lockToObject;}
        void resetMaxOrientationChange(float newMax = -1.0) {maxOrientationChange = newMax;}
        float getMaxOrientationChange() {return maxOrientationChange;}

        //static const double NaN;

    protected:
        Window3D *parent;
        PosableStateBase *perspectivePosable;
        PosableStateLock *lockToParent, *lockToObject;
        virtual void PoseReplaced(PosableStateBase *oldPose, PosableStateBase *newPose);

        static GLfloat heightExaggeration;
        GLfloat localHeightExaggeration;
        GLboolean usingLocalExaggeration;

        GLfloat perspectiveWidthRatio;
        GLfloat perspectiveHeightRatio;

        GLfloat maxOrientationChange;
        GLfloat oldHeading;

        /*---Viewing Frustum information, operations---*/
        //Note: if someone wants to get to these, just derive a class that accesses them.
        GLdouble        centerToNearPlaneOffset;
        GLdouble        centerToEyepoint;
        GLdouble        centerToScreen;
        GLdouble        nearPlane;
        GLdouble        distanceToFarPlane;
        GLdouble        minimumFrustumSize;
        GLdouble        stereoOffset;
        const int       *windowWidth, *windowHeight; //necessary because we need to know them before the matrices are set up!
        int             offX, offY; //offset values for rendering to movies (mdp 11/10/2006)
        void setViewpoint(GLdouble x, GLdouble y, GLdouble z, GLboolean fromScratch = GL_TRUE);
        GLboolean       cornerInfoCurrent;         //0 = need to call updateCornerInfo
        gz4d::Point<double> corners[8]; //front/back, which corner, which dimension
        gz4d::Point<double> cornerMin, cornerMax;
        GLdouble        zComponentOfUnderPlane;
        GLdouble        zComponentOfAbovePlane;
        int             perspectiveFlag;

        /*---Transformations about the reference point---*/
        bool       referenceIsCurrent;      //Do we need to update the referencePoint?
        gz4d::Point<double> referencePoint;       //world coordinates derived from the translation.

        /*---Viewpoint information from GL---*/
        bool        viewInfoCurrent;         //false = need to call updateViewInfo
        GLdouble        projectionMatrix[16];    //last updated projection matrix
        GLdouble        modelMatrix[16];         //last updated model matrix
        GLint           viewport[4];             //last updated viewport matrix
        GLint           whichEye;                //what was the last eye set for projection? (added by mdp 8/2/2000)
        GLint           whichBuffer;             //which front buffer does this correspond to? (added by mdp 8/3/2000)

        /*---Headtracking information---*/
        Isotrack *isotrack;

        /*---Pointers to haptic vr related flags---*/
        int * ptr_fishtankVR_flag;
        int * ptr_headtracking_flag;

        /*---Derived viewpoint information---*/
        GLdouble        distanceToScreen;        //wc of eyePoint to screen-center distance
        GLdouble        pixelSize;               //length of the side of a pixel in wc
        ViewIntersection *tempVi;

        /*---Derived viewpoint information with extra work involved---*/
        int             updateExtraViewInfo();
        bool                extraViewInfoCurrent;    //false = need to call UpdateExtraViewInfo

        gz4d::Point<double> screenWorldPos;       //world coordinates of screen center
        gz4d::Point<double> eyePoint;             //world coordinates of eyePoint

        /*---Proxy representations---*/
        WindowAnchor    *anchor;

        /*---Interaction facilitation---*/
        static const EventType *viewChangedInputType;
        static const EventType *viewStabilizedInputType;
        static const EventType *spaceWarpedInputType;

        /*---Pointers to stereo related flags---*/
        int * ptr_stereo_flag;
        int * ptr_ssstereo_flag;

        // measurments
        int *screenWidth_pix;
        int *screenHeight_pix;
        double *screenLeft;
        double *screenRight;
        double *screenTop;
        double *screenBottom;
	double *eyeX;
        double *eyeY;
        double *eyeZ;

        int * mainRight;
        
};

struct ViewIntersection
{
    GLboolean perspectiveFlag, usingBothZs, usingBothYs, fullyEnclosed;
    GLdouble tolerance;
    GLdouble zBottom, zTop;         //bottom, top z's chosen by user
    GLdouble edgeIntersect[2][12][2];   //bottom = 0, top = 1, (points calc'd) but...
    GLboolean edgeValid[2][12];     //...only 8 intersection points are valid in each
    GLdouble yLess, yMore;          //min and max y's chosen by user for line intersection

    //The following are calculated results intended to be queried by the user.
    GLdouble yMin, yMax;            //Min and max Y's that intersect either z plane
    GLdouble xMin, xMax;            //min and max x's that intersect view at given z or (y and z)
};

#endif
