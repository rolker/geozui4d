// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

///////////////////////////////////////////////////////////////////////////////
// Sv_Widgets.cpp
// 
//
//  This class implements a rotation interactor with 2 degrees of freedom,
//  for operating on ViewManagers.
///////////////////////////////////////////////////////////////////////////////

#include "widgets/Sv_Widgets.h"
#include "engine/PosableStateBase.h"
#include<gz4d/glo.h>
#include <GL/glut.h>
#include <math.h>
#include "engine/ViewManager.h"
#include "engine/Window3D.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "interactors/ScaleDrag.h"
#include "LightDirector.h"
#include "utilities/DottedAddress.h"
#include "engine/Scene3D.h"


#include "MainWindow.h"
extern MainWindow * mainWindow;

#define ELEVATION_WIDGET_HEIGHT_FACTOR 11.0

#define SLIDER_RATIO 37.6f
#define SLIDER_SIZE_NUMERATOR 25
#define SCALE_MAX_POWER 5
#define SCALE_MIN_POWER -2
static char scaleLabel[][10] = {"100km", "10 km", "1 km", "100 m", "10 m", "1 m", ".1 m", ".01 m"};
#define H_SCALE_NAME 5
#define V_SCALE_NAME 6
#define WID_HEIGHT  11.0  // was 12

#define SHALLOW_ANGLE_BOTTOM 5
#define SHALLOW_ANGLE_TOP 20

#define FTVR_ANGLE 45.0

int Sv_Widgets::showCompass = GL_TRUE;
int Sv_Widgets::showHorizontalScale = GL_TRUE;
int Sv_Widgets::showVerticalScale = GL_TRUE;
int Sv_Widgets::shallowAngleCompensation = GL_FALSE;
int Sv_Widgets::showLightingControl = GL_FALSE;
bool *Sv_Widgets::isVisible = NULL;
double Sv_Widgets::selectionHeight = 1.5;
double Sv_Widgets::selectionWidth = 1.5;

/*---------------------------------------------------------------------------*/
/** Constructor.  Sets up the widgets in the home position (all 0's).
 *  
 */
Sv_Widgets::Sv_Widgets(ViewManager *targetView, GLfloat theSensitivity,
                       GLshort theSelectButton, 
                       GLshort theLockButton,GLshort theVideo )
:Interactor("widgets")
{
    GLfloat sliderratio;

    /*---Initialize interaction values---*/
    itsSelectButton = theSelectButton;
    itsLockButton = theLockButton;
    itsWidgetStatus = SV_WIDGETS_NOACTION;
    itsButtonStatus = MouseButtonUp;
    ignoreAdjustment = GL_FALSE;

    currentView = targetView; //set the target ViewManager
    parentWin = currentView->getParentWin();
    currentPose = currentView->getPosable();
    picking = GL_FALSE;

    Point3d lightLocation = {400.0, 400.0, 0.0};
    lightDirector = new LightDirector(parentWin, lightLocation);
    lightDirector->getAddress()->reparent(getAddress());

    /*---Initialize Display information---*/
    itsAspectRatio = -1.0;

    if (!isVisible)
        isVisible = parentWin->getScene3D()->GetWidgetsVisibleFlag();

    // Set the position of the sliders so that they are always at the same
    // relative positions on the screen, regardless of the size of the monitor.
    sliderratio = (GLfloat)SLIDER_RATIO;//CMTOPIXEL;
    if (theVideo)
        sliderratio *= (GLfloat)0.93;

    itsSlider = new Sv_Slider3(0.0, ELEVATION_WIDGET_HEIGHT_FACTOR * sliderratio, 0.0,
                                            0, 0, -180, 3.0f*sliderratio);

    itsSlider->setIdentifier(SV_WIDGETS_IDENTIFIER, 0);
    itsWidgetIdentifier = -1;
    itsWidgetPart = -1;
    azimuthLock = false;

    
    GetWindow();

    itsSlider->setAzimuth(-currentPose->getOrientationAxis()[Y]);

}

/*---------------------------------------------------------------------------*/
/** Destructor.  The only thing to clean up is the sliders.
 */
Sv_Widgets::~Sv_Widgets( void )
{
    ReleaseWidgets();

    delete itsSlider;
    delete lightDirector;
}

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
void Sv_Widgets::UpdatePose()
{
    currentPose = currentView->getPosable();
}

/*---------------------------------------------------------------------------*/
/** Adds the "Widget Options" menu to
 *  allow the Tcl/Tk interface some control on the interaction.
 */
int Sv_Widgets::RegisterWithTcl(ToglWindow *throughWin)
{
    throughWin->Tcl_Eval("addOption {Widget Options} cascade {} widgets");
    throughWin->Tcl_Eval("addOption { Compass} checkbutton {redisplay} widgetCompass 1 0 widgets");
    throughWin->Tcl_Eval("addOption { Horizontal Scale} checkbutton {redisplay} widgetHScale 1 0 widgets");
    throughWin->Tcl_Eval("addOption { Vertical Scale} checkbutton {redisplay} widgetVScale 1 0 widgets");
    throughWin->Tcl_Eval("addOption { Lighting Control} checkbutton {redisplay} widgetLighting 1 0 widgets");
    throughWin->Tcl_Eval("addOption { Shallow Angle Compensation} checkbutton {redisplay} widgetShallow 1 0 widgets");

    throughWin->Tcl_LinkVar("widgetCompass", &showCompass);
    throughWin->Tcl_LinkVar("widgetHScale", &showHorizontalScale);
    throughWin->Tcl_LinkVar("widgetVScale", &showVerticalScale);
    throughWin->Tcl_LinkVar("widgetLighting", &showLightingControl);
//  throughWin->Tcl_LinkVar("widgetVisible",isVisible);
    throughWin->Tcl_LinkVar("widgetPickHeight", &selectionHeight, TCL_LINK_DOUBLE);
    throughWin->Tcl_LinkVar("widgetPickWidth", &selectionWidth, TCL_LINK_DOUBLE);

    return throughWin->Tcl_LinkVar("widgetShallow", &shallowAngleCompensation);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** HandleInput handles button 1 & 3 events, as well as mouse motion events.
 */
int Sv_Widgets::HandleInput(unsigned int inputType, InputInfo *info)
{
    int previousStatus;

    if (inputType == NoInput) //Idle event
    {
        CallBackIdleFunc();
        return PASS_EVENT_THROUGH;
    }

    if (inputType & EnumeratedAction)
    {
        if (inputType == BreakAction)
            CheckWidgets(MouseButtonUp, -1, -1);

        if (!ignoreAdjustment && inputType == ViewManager::viewChangedEvent() && info->val.p == currentView)
        {
            gz4d::Point<double> orient = currentPose->getOrientationAxis();

            if (info->modifier.l & ChangeRotateYMask)
                itsSlider->setAzimuth(-orient[Y]);
            if (info->modifier.l & ChangeRotateXMask)
                itsSlider->setElevation(orient[X]);
        }
        return PASS_EVENT_THROUGH;
    }

    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

    if (inputType & MouseButtons) //Is it a button event?
    {   if (inputType & MouseButton2)
            return PASS_EVENT_THROUGH; //don't use MouseButton2
        previousStatus = itsWidgetStatus;
        CallBackMouseFunc(inputType & MouseButtons,
                          inputType & MouseState,
                          info->x.i,
                          info->y.i);
        if (itsWidgetStatus == SV_WIDGETS_NOACTION ||
            (itsWidgetStatus == SV_WIDGETS_LOCK && previousStatus == SV_WIDGETS_LOCK) ||
            itsWidgetStatus == SV_WIDGETS_EXIT ||
            (itsWidgetStatus == SV_WIDGETS_ENTER && itsWidgetIdentifier == -1))
            return PASS_EVENT_THROUGH;
        else
        {
//          info->window->inputMediator.Trigger(Window3D::windowChangedEvent());
            return SINK_EVENT;
        }
    }
    else if (inputType & MouseMotion) // is it a mouse motion event?
    {   if (itsWidgetStatus == SV_WIDGETS_NOACTION ||
            itsWidgetStatus == SV_WIDGETS_LOCK ||
            itsWidgetStatus == SV_WIDGETS_EXIT ||
            (itsWidgetStatus == SV_WIDGETS_ENTER && itsWidgetIdentifier == -1))
            return PASS_EVENT_THROUGH;
        CallBackMotionFunc(info->x.i, info->y.i);
//      info->window->inputMediator.Trigger(Window3D::windowChangedEvent());
        return SINK_EVENT;
    }

    return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/** This function handles a mouse button press or release event.  It assumes
 *  that only a used button has been pressed (itsLockButton or itsSelectButton).
 *
 *  @param  button      Which button (from the InputType enum)
 *  @param  state       Up or down? (also from the InputType enum)
 *  @param  x           Mouse X position
 *  @param  y           Mouse Y position
 */
void Sv_Widgets::CallBackMouseFunc(int button, int state, int x, int y)
{
    itsMouseX = x;
    itsMouseY = y;
    itsActiveButton = button;

    if (state == MouseButtonDown && button == itsLockButton)
        UpdateWidgets(x, y, MouseButtonDown);

    CheckWidgets(state, x, y);
}

/*---------------------------------------------------------------------------*/
/** This function handles a mouse motion event.
 *
 *  @param  x           Mouse X position
 *  @param  y           Mouse Y position
 */
void Sv_Widgets::CallBackMotionFunc(int x, int  y)
{

    itsMouseX = x;
    itsMouseY = y;
}



/*---------------------------------------------------------------------------*/
/** This function handles an idle event.
 *
 *  
 */
void Sv_Widgets::CallBackIdleFunc()
{
    if ((itsButtonStatus == MouseButtonDown &&
         (itsWidgetStatus == SV_WIDGETS_DRAG || itsWidgetIdentifier != -1) ) ||
         azimuthLock)
        UpdateWidgets(itsMouseX, itsMouseY, MouseButtonUp);

    //cerr << "idle \n";
}

/*---------------------------------------------------------------------------*/
/** See the other UpdateWidgets.
 */
void Sv_Widgets::UpdateWidgets()
{
    UpdateWidgets(itsMouseX, itsMouseY, MouseButtonUp);
}

/*---------------------------------------------------------------------------*/
/** This function updates the widgets based on the passage of time or
 *  user mouse movements.
 *
 *  @param  thePosX         Cursor X position.
 *  @param  thePosY         Cursor Y position.
 *  @param  but2state       State of the "lock" button (from the InputType enum)
 */
void Sv_Widgets::UpdateWidgets( GLint thePosX, GLint thePosY, GLshort but2state )
{
    // dummy variables to store the returned position
    GLint xdelta;
    GLint ydelta;
    
    thePosY = itsWindowHeight-thePosY;
    xdelta = thePosX - itsOldMouseX;
    ydelta = thePosY - itsOldMouseY;
    AdjustWidgets(xdelta, ydelta, but2state );

    itsOldMouseX = thePosX;
    itsOldMouseY = thePosY;
}

/*---------------------------------------------------------------------------*/
/** This function resizes portions of the widgets to work well regardless of
 *  the enveloping window size.
 */
void Sv_Widgets::Resize()
{
    ;
}

/*---------------------------------------------------------------------------*/
/** This function is called when a pick returns a widget identifier. It stores
 *  internally which widget and which part were hit, and returns a value, 
 *  1 if there was a valid widget hit, 0 if not.
 */
GLint Sv_Widgets::PickWidgets( GLuint *buffer, GLint hits )
{
    GLint i;
    GLint indx = 0;
    itsWidgetIdentifier = -1;
    itsWidgetPart = -1;
    
    // reset view perspective
    for (i=0; i<hits; i++)
    {
        GLint items = buffer[indx++];
        indx += 2;  // skip over min & max Z-values

        if (buffer[indx] == (unsigned) SV_WIDGETS_IDENTIFIER)
        {
            if (itsWidgetPart == 0)
            {
                indx += items;  //added by mdp 10/11/2000
                continue; //return TRUE; //changed by mdp 10/11/2000
            }

            itsWidgetIdentifier = buffer[indx+1];
            itsWidgetPart = buffer[indx+2];

//          cerr << ":   H " << itsWidgetIdentifier << " " << itsWidgetPart;

            if (itsWidgetPart == H_SCALE_NAME || itsWidgetPart == V_SCALE_NAME)
            {
                if ((MainWindow::getInterfaceType() != GeoCoastPilot) || itsWidgetPart == H_SCALE_NAME)
                {
                    parentWin->inputMediator.PushInteractor(new ScaleDrag(GL_FALSE, itsWidgetPart - H_SCALE_NAME));
                    itsWidgetPart = 0;
                }
            }
            //When we clicked on widget but no action intended (e.g., click on the widget bar)
            if (itsWidgetPart >= 1)
            {
    
                currentPose->setContinuousChange(TRUE);
                            
                if (azimuthLock)
                {
                    itsSlider->resetAzimuth();
                    itsSlider->HighlightOff();
                    
                    azimuthLock=0;  // unset the lock
                    currentPose->setContinuousChange(FALSE);
                }               
                return TRUE;
            }
            else
            {
            itsWidgetIdentifier = -1;
                itsWidgetPart = -1;
                indx += items;
            }
        }
        else
            indx += items;
    }

//  cerr <<  "\n";
    
    return FALSE;
}

/*---------------------------------------------------------------------------*/
/** This is the routine that handle the moving of the widgets on the screen;
 *  thus, if this routine is not called, the widgets will change color, but
 *  they _will_not_move_.
 *
 *  @param  xdelta          change in mouse X position since the last call.
 *  @param  ydelta          change in mouse Y position since the last call.
 *  @param  lockButtonState state of the button used to lock the widgets.
 */
void Sv_Widgets::AdjustWidgets(GLint xdelta, GLint ydelta, GLshort lockButtonState )
{

//  GLshort lwpos = 0;    // position in array of locked widgets

    GLfloat azimuth;
    GLfloat elevation;
    float factor = 500.0 / parentWin->getMinDimension();

    azimuth = itsSlider->updateAzimuth();
    elevation = itsSlider->getCurrentElevation();

    if (itsWidgetPart == AZIMUTH_TAB)
    {
        itsSlider->setAzimuthVel(xdelta * factor);  // setAzimuth
        itsSlider->HighlightOn();
    }

    if (itsWidgetPart == ELEVATION_CONTROL)  // the other parts of the widget
    {
        elevation = itsSlider->getElevation(ydelta * factor); 
        itsSlider->HighlightOff();
    }
        
    SetRotate(elevation, azimuth);

    if ((itsWidgetStatus == SV_WIDGETS_ENTER ||
        itsWidgetStatus == SV_WIDGETS_DRAG) &&
        lockButtonState == MouseButtonDown)
    {
        if (itsWidgetPart == AZIMUTH_TAB)
        {       
            azimuthLock = true;  // set the lock
            currentPose->setContinuousChange(TRUE);
            itsSlider->HighlightOn();
        }
    }
}

/*---------------------------------------------------------------------------*/
/** This function is called automatically when the condition of the
 *  widgets is SV_WIDGETS_EXIT to restore the proper colors to the widgets
 *  and set the proper values internally.
 */
GLint Sv_Widgets::ReleaseWidgets( void )
{

    if (!azimuthLock) 
    {
        itsSlider->resetAzimuth();
        itsSlider->HighlightOff();
    }
    currentPose->setContinuousChange(FALSE);
    itsWidgetIdentifier = -1;
    itsWidgetPart  = -1;

    return azimuthLock;
}

/*---------------------------------------------------------------------------*/
/** Returns all the widgets to their inital (0) positions, and resets
 *  their colors.
 */
void Sv_Widgets::ResetWidgets()
{

    itsSlider->resetAzimuth();
    itsSlider->HighlightOff();

    currentPose->setContinuousChange(FALSE);
    azimuthLock = false;

    currentView->reset();
}

/*---------------------------------------------------------------------------*/
/** This is the main widget interaction routine. It should be called once
 *  each time through the main loop of the user program. It will check if
 *  there has been any interaction with the widgets, and will adjust the
 *  appearance of the widgets appropriately. The value returned from this
 *  function indicates the current status of the widgets. Valid return
 *  values are:
 *      SV_WIDGETS_ENTER : A widget has just been clicked on; it may or
 *      may not have been previously locked, but regardless, while
 *      the 'active' button remains down, mouse movement controls the
 *      widget action. This value is returned only once when a widget
 *      has been activated. The next value returned will always be
 *      SV_WIDGETS_DRAG or SV_WIDGETS_EXIT, depending on whether the
 *      'active' button is down on the next pass or has been released, 
 *      respectively.
 *          The cursor will disappear at this point, and the widget will
 *      highlight.
 *      SV_WIDGETS_EXIT : A widget was active, but now 'active' button has
 *      been released. Note that there may or may not be locked widgets;
 *      that will not be distinguished until the next value returned.
 *          The cursor will now reappear, and the widget will either
 *      return to its normal color (if it was not locked) or change to
 *      a 'locked' color (if it was locked).
 *      SV_WIDGETS_DRAG : A widget has been activated, and the 'active' mouse
 *      button is still down. All mouse movement will affect the widget
 *      until the button is released.
 *          The widget color will be a highlight, and the cursor will
 *      still be invisible. Note that any other locked widgets will
 *      continue to change during the drag time.
 *      SV_WIDGETS_LOCK = 3 : No widgets are currently activated by the
 *      mouse, although at least one remains locked in an active position.
 *          The mouse is visible and moves freely, and all unlocked
 *      widgets are their respective normal colors.
 *      SV_WIDGETS_NOACTION = 4 : No widgets are crrently activated by the
 *      mouse, and, in addition, no widgets are locked. The cursor is
 *      visible and free, and all widgets are their normal colors.
 *
 *  @param  widgetButtonStatus  MouseButtonUp / MouseButtonDown
 *  @param  thePosX             X position of the mouse
 *  @param  thePosY             Y position of the mouse
 */
GLshort Sv_Widgets::CheckWidgets( GLshort widgetButtonStatus, GLint thePosX, GLint thePosY )
{
    GLint hits;
    GLint aTempY = thePosY;
    GLint vp[4];        // the current viewport
    GLuint buffer[1000];// name buffer

    if (widgetButtonStatus == MouseButtonDown)
    {
        if (itsButtonStatus == MouseButtonUp)//was up ?
        {
            // pick
            glMatrixMode( GL_PROJECTION );
            glPushMatrix();
            glMatrixMode( GL_MODELVIEW );
            glPushMatrix();
            
            glGetIntegerv( GL_VIEWPORT, vp );// get the viewport to pass to picking parameters
            thePosY = vp[3]-thePosY;
            
            glMatrixMode(GL_PROJECTION);     // projection matrix mode    
            glLoadIdentity();
            
            glDrawBuffer( GL_BACK_LEFT );
            
            gluPickMatrix( (GLdouble)thePosX, (GLdouble)thePosY, selectionWidth, selectionHeight, vp );
            glMultMatrixf( itsCurrentProjectionMatrix );
            glMatrixMode(GL_MODELVIEW); 
            
            // pick the widgets
            glSelectBuffer( 1000, buffer);   // set up picking buffer
            glRenderMode(GL_SELECT);         // picking mode
            glInitNames();                   // Clear the name stack (mdp 9/13/2000)
            picking = GL_TRUE;
            
            glPushMatrix();                  // save a copy of the PROJECTION matrix
            glLoadIdentity();
            DrawWidgets();                   //Draw widgets for picking,

            //those component that are drawn here are to be tested
            //with the mouse pointer to see if they are picked.
            
            glPopMatrix();                   // restore MODELVIEW matrix
            glFlush();
            picking = GL_FALSE;
            hits = glRenderMode(GL_RENDER);  // finish picking
            
            glMatrixMode( GL_PROJECTION );
            glPopMatrix();
            glMatrixMode( GL_MODELVIEW );
            glPopMatrix();
            
            if (PickWidgets( buffer, hits ))
            {
                itsWidgetStatus = SV_WIDGETS_ENTER;
            }
            else
            {
                if (itsWidgetStatus != SV_WIDGETS_LOCK)
                    itsWidgetStatus = SV_WIDGETS_NOACTION;
            }
        }
        else
        {
            if (itsWidgetStatus == SV_WIDGETS_EXIT)
            {
                if (azimuthLock)
                    itsWidgetStatus = SV_WIDGETS_LOCK;
                else
                    itsWidgetStatus = SV_WIDGETS_NOACTION;
            }
            else if (itsWidgetStatus == SV_WIDGETS_ENTER)
                itsWidgetStatus = SV_WIDGETS_DRAG;
        }
        itsButtonStatus = MouseButtonDown;
    }
    else    // button is up
    {
        if (itsButtonStatus == MouseButtonDown)
        {
            if ((itsWidgetStatus == SV_WIDGETS_ENTER) ||
                (itsWidgetStatus == SV_WIDGETS_DRAG))
            {
                itsWidgetStatus = SV_WIDGETS_EXIT;
                ReleaseWidgets();
            }
            else if (itsWidgetStatus == SV_WIDGETS_EXIT)
            {
                if (azimuthLock)
                    itsWidgetStatus = SV_WIDGETS_LOCK;
                else
                    itsWidgetStatus = SV_WIDGETS_NOACTION;
            }
        }
        else    // button was up
        {
            if (itsWidgetStatus == SV_WIDGETS_EXIT)
            {
                if (azimuthLock)
                    itsWidgetStatus = SV_WIDGETS_LOCK;
                else
                    itsWidgetStatus = SV_WIDGETS_NOACTION;
            }
        }
        itsButtonStatus = MouseButtonUp;
    }
    
    // last cleanup act
    if (itsWidgetStatus == SV_WIDGETS_ENTER )
    {
        thePosY = itsWindowHeight-aTempY;//the real y coordinate for OpenGL
        itsOldMouseX = thePosX;
        itsOldMouseY = thePosY;
    }
    return itsWidgetStatus;
}

/*---------------------------------------------------------------------------*/
void Sv_Widgets::GetWindow( GLint xsize, GLint ysize )
{
    GLint vp[4];
    
    if ((xsize == -1) || (ysize == -1))
    {
        glGetIntegerv( GL_VIEWPORT, vp );
        itsAspectRatio = (float)vp[2]/(float)vp[3];
        itsWindowHeight = vp[3];
    }
}

/*---------------------------------------------------------------------------*/
void Sv_Widgets::SetRotate( float angx, float angy, float angz)
{
//  RelationshipQueryResult *locksOnWindow;
//  PosableStateRelationship *currentRel;
//  PosableStateLock *currentLock;
//  long changeMask = PosableStateLock::LockDeltaRotate;

//  locksOnWindow = relationshipManager->getThoseThatUpdate(currentPose, changeMask, "PosableStateLock");

    //if there are delta locks, unlock temporarily.
//  for (currentRel = locksOnWindow->first(); currentRel; currentRel = locksOnWindow->next())
//  {
//      currentLock = (PosableStateLock *)currentRel;
//      currentLock->unlock(changeMask);
//  }

    if( mainWindow->isFishtankVREnabled() )
    {
        if (angx > 45.0)
            angx = 45.0;
        if (angx < -60.0)
            angx = -60.0;
    }
    else
    {
        if (angx > 45.0+FTVR_ANGLE)
            angx = 45.0+FTVR_ANGLE;
        if (angx < -60.0+FTVR_ANGLE)
            angx = -60.0+FTVR_ANGLE;
    }

    currentPose->setOrientation(angx, -angy);

    //if there are delta locks, relock them.
//  for (currentRel = locksOnWindow->first(); currentRel; currentRel = locksOnWindow->next())
//  {
//      currentLock = (PosableStateLock *)currentRel;
//      currentLock->restorePreviousLock();
//  }
}

/*---------------------------------------------------------------------------*/
void Sv_Widgets::SetScale( GLfloat sval )
{
    currentPose->setScale(sval);
}

/*---------------------------------------------------------------------------*/
/** This function is provided so that the user can override the default
 *  mapping of the velocity controls, so that smaller movements result in 
 *  larger overall movements.
 *
 *  The parameter "which" specifies which widgets is to be
 *  affected by the change; this can be a number from 0-6, or a
 *  symbolic constant as speicified at the beginning of this file
 *  (of the the format SV_WIDGETS_x_WIDGET, where 'x' is the type of
 *  widget; XROTATION, YTRANSLATION, SCALE, etc.).
 *
 *  The parameter "val" is the power that the
 *  position of the velocity control will be mapped to on the position control.
 *  By default, it is 2, so we are just dealing with the square of the value.
 */
void Sv_Widgets::SetVelocityRatio( GLshort which, GLfloat val )
{
    
    ;
}

/*---------------------------------------------------------------------------*/
/** Reset the widgets to the home view.
 */
void Sv_Widgets::ViewHome( void )
{
    currentPose->setPoseInfoFromTo(ChangeHomePose);
    itsSlider->setAzimuth(-currentPose->getOrientationAxis()[Y]);
}

/*---------------------------------------------------------------------------*/
/** This function draws the navigation sliders and the crosshairs to the
 *  GL display.
 */
void Sv_Widgets::DrawWidgets( void )
{
    GLint aValue;
    GLfloat sliderratio = SLIDER_RATIO;

    if (!(*isVisible))
        return;

    // Save the current projection matrix for use in picking the widgets
    if (!picking) //check added by mdp 9/13/2000
        glGetFloatv(GL_PROJECTION_MATRIX, itsCurrentProjectionMatrix);
    
    if (itsAspectRatio < 0.0)
        GetWindow(-1, -1);

    glPushMatrix();
    glScalef(0.0003f,0.0003f,0.0003f);
    gz4d::Point<double> orient = currentPose->getOrientationAxis();
    glRotatef(orient[Z], 0.0, 0.0, 1.0);
    glDisable(GL_LIGHTING);

    glPushMatrix();
        if (!shallowAngleCompensation || (orient[X] > SHALLOW_ANGLE_TOP || orient[X] <= SHALLOW_ANGLE_BOTTOM))
            glRotatef(orient[X], 1.0, 0.0, 0.0);
        else
            glRotatef(SHALLOW_ANGLE_TOP, 1.0, 0.0, 0.0);

        itsSlider->Draw();
    glPopMatrix();                                                          

    glGetIntegerv(GL_DEPTH_FUNC, &aValue);
    glDepthFunc(GL_LESS);
    glFlush();
    
    glLineWidth(1.0f);

    if( mainWindow->isFishtankVREnabled() )
        glColor3ub(255, 255, 255);
    else
        glColor4ub( 200, 200, 200, 255 ); //chaned to grey to compensate for lack of lighting.

    if (showHorizontalScale)
    {
        if( mainWindow->isFishtankVREnabled() )
        {
            glPushMatrix();
            glRotatef(-45.0, 1.0, 0.0, 0.0);
            DrawScale(ELEVATION_WIDGET_HEIGHT_FACTOR*sliderratio);
            glPopMatrix();
        }
        else
            DrawScale(ELEVATION_WIDGET_HEIGHT_FACTOR*sliderratio);

        // Possibly this might be wanted at some point?  (Horizontal scale in orthogonal direction)
        if( mainWindow->isFishtankVREnabled() && orient[X] == 45.0 )
        {
            glPushMatrix();
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glRotatef(orient[X], 1.0, 0.0, 0.0);
            DrawScale(ELEVATION_WIDGET_HEIGHT_FACTOR*sliderratio, GL_TRUE, 90.0f );
            glPopMatrix();
        }
        else if( !mainWindow->isFishtankVREnabled() && orient[X] > 85.0 && orient[X] < 95.0 )
        {
            glPushMatrix();
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glRotatef(orient[X], 1.0, 0.0, 0.0);
            DrawScale(ELEVATION_WIDGET_HEIGHT_FACTOR*sliderratio, GL_TRUE);
            glPopMatrix();
        }
    }

    if (showVerticalScale && (orient[X] < 85.0 || orient[X] > 95.0))
    {
        glPushMatrix();
        glRotatef(orient[X], 1.0, 0.0, 0.0);
        DrawScale(ELEVATION_WIDGET_HEIGHT_FACTOR*sliderratio, GL_TRUE);
        glPopMatrix();
    }

    //draw the rotation axis
    glPushName(0);//0 -> part of widget but should not do anything

    glBegin(GL_LINES);//on x
        glVertex3f(-ELEVATION_WIDGET_HEIGHT_FACTOR*sliderratio,0.0,0.0);
        glVertex3f( ELEVATION_WIDGET_HEIGHT_FACTOR*sliderratio, 0.0, 0.0 );
    glEnd();
        
    
    glPushMatrix();
    glRotatef(orient[X], 1.0, 0.0, 0.0);
    glColor4ub( 200, 200, 200, 255 ); //chaned to grey to compensate for lack of lighting.
    glBegin(GL_LINES);//on y
    glVertex3f(0.0, ELEVATION_WIDGET_HEIGHT_FACTOR*sliderratio, 0.0);
    glVertex3f(0.0, -ELEVATION_WIDGET_HEIGHT_FACTOR* sliderratio, 0.0 );
    glEnd();
    glPopMatrix();

    glPushMatrix();
    if (!shallowAngleCompensation || (orient[X] > SHALLOW_ANGLE_TOP || orient[X] <= SHALLOW_ANGLE_BOTTOM))
        glRotatef(orient[X]-90.0f, 1.0, 0.0, 0.0);
    else
        glRotatef(SHALLOW_ANGLE_TOP - 90.0f, 1.0, 0.0, 0.0);

    glTranslatef(0.0, 0.0, (ELEVATION_WIDGET_HEIGHT_FACTOR-1)*sliderratio);  
    if (showCompass)
        DrawCompass(orient);
    glPopMatrix();
    
    glEnable(GL_LIGHTING);
    glPopMatrix();
    
    glDepthFunc(aValue);

    if (showLightingControl)
    {
        lightDirector->setVisible();
        lightDirector->DrawIn(parentWin);
    }
    else
        lightDirector->setVisible(GL_FALSE);
}

void Sv_Widgets::DrawCompass(gz4d::Point<double> const &orient)
{
    GLint viewport[4];
    int i, width;

    glPushMatrix();
    glRotated(orient[Y], 0.0, 0.0, 1.0);

    //what size screen are we rendering into?
    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    if (width > viewport[3])
        width = viewport[3];

    //given this, how thick should we make the arrow?
    if (width > 600)
        width = 4;
    else
        width = 3;

    //draw it.
    for (i = 1; i >= 0; i--)
    {
        glLineWidth(width - 2 * i);
        glColor4ub(200 * i, 200 * i, 200 * i, 255 );
        glBegin(GL_LINES);
        //North arrow
        glVertex3f(0.0, -113.0, 38.0);
        glVertex3f(0.0, 113.0, 38.0);
        glVertex3f(-15.0, 80.0, 38.0);
        glVertex3f(0.0, 113.0, 38.0);
        glVertex3f(15.0, 80.0, 38.0);
        glVertex3f(0.0, 113.0, 38.0);
        //North "N"
        glVertex3f(-5.0, 130.0, 38.0);
        glVertex3f(-5.0, 147.0, 38.0);
        glVertex3f(-5.0, 147.0, 38.0);
        glVertex3f(5.0, 130.0, 38.0);
        glVertex3f(5.0, 130.0, 38.0);
        glVertex3f(5.0, 148.0, 38.0);
        //East
        glVertex3f(113.0, 0.0, 38.0);
        glVertex3f(97.0, 0.0, 38.0);
        //West
        glVertex3f(-113.0, 0.0, 38.0);
        glVertex3f(-97.0, 0.0, 38.0);
        glEnd();
    }
    glLineWidth(1.0);
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/
/** Draw scale bars to indicate scale.  We get to do this without conversion
 *  since the widgets are defined in meters when scale is 1.
 */
void Sv_Widgets::DrawScale(GLfloat width, GLboolean vertical, GLfloat adj_angle)
{
    GLfloat scale, labelOffset;
    int powerOf10 = SCALE_MAX_POWER, fiveCounter, i;
    GLfloat currentPowerOf10 = pow(10.0, (double)powerOf10);
    GLfloat tick, tickWidth, tickHeight, pixWidth;

    scale = currentPose->getScale()/0.0003;
    pixWidth = currentView->getPixelSize();//use pixel size at screen since atPos is more expensive...scale numbers by roughly 1.325
    if (vertical)
    {
        glPushMatrix();
        glRotatef(-90.0, 0.0, 0.0, 1.0);
        if( mainWindow->isFishtankVREnabled() )
            labelOffset = 0.0;
        else
            labelOffset = -7 * pixWidth * scale; //about 5 * 1.325
        scale *= currentView->getHeightExaggeration();
        glPushName(V_SCALE_NAME);
    }
    else
    {
        labelOffset = 14 * pixWidth * scale;   //about 10 * 1.325
        glPushName(H_SCALE_NAME);
    }
    //loop through the different powers of ten to draw scale ticks.
    for (; powerOf10 >= SCALE_MIN_POWER; powerOf10--)
    {
        tickWidth = scale * currentPowerOf10;
        if (tickWidth < 3)
            break;

        fiveCounter = 1;
        //work across the widget within this scale
        for (tick = tickWidth; tick <= width; fiveCounter++, tick += tickWidth)
        {
            if (fiveCounter == 10) //scale the ticks to the proper height
            {
                fiveCounter = 0;
                if (powerOf10 == SCALE_MAX_POWER)
                    tickHeight = tickWidth * 3.0;
                else
                    continue;
            }
            else if (fiveCounter == 5)
                tickHeight = tickWidth * 2.0;
            else
                tickHeight = tickWidth * 0.4;

            //Designate the unit ticks (one for each scale on left & right)
            if (tick == tickWidth && tickWidth >= 30)
            {
                if (MainWindow::getInterfaceType() == GeoCoastPilot)
                    glColor4ub( 200, 200, 200, 255 ); //chaned to grey to compensate for lack of lighting.
                if( mainWindow->isFishtankVREnabled() )
                {
                    if( vertical )
                    {
                        gz4d::Point<double> orient = currentPose->getOrientationAxis();
                        glPushMatrix();
                        glRotatef(90.0, 0.0, 0.0, 1.0);
                        parentWin->printFtvrAt(tickHeight * 1.2, tick - labelOffset, 0.0, scaleLabel[SCALE_MAX_POWER - powerOf10], GL_FALSE, -orient[X]-45.0-adj_angle);
                        parentWin->printFtvrAt(tickHeight * 1.2, -tick - labelOffset, 0.0, scaleLabel[SCALE_MAX_POWER - powerOf10], GL_FALSE, -orient[X]-45.0-adj_angle);
                        glPopMatrix();
                    }
                    else
                    {
                        parentWin->printFtvrAt(tick - labelOffset, tickHeight * 1.2, 0.0, scaleLabel[SCALE_MAX_POWER - powerOf10], GL_FALSE);
                        parentWin->printFtvrAt(-tick - labelOffset, tickHeight * 1.2, 0.0, scaleLabel[SCALE_MAX_POWER - powerOf10], GL_FALSE);
                    }
                }
                else
                {
                    parentWin->printAt(tick - labelOffset, tickHeight * 1.2, 0.0, scaleLabel[SCALE_MAX_POWER - powerOf10]);
                    parentWin->printAt(-tick - labelOffset, tickHeight * 1.2, 0.0, scaleLabel[SCALE_MAX_POWER - powerOf10]);
                }
                parentWin->printAt(tick - labelOffset, tickHeight * 1.2, 0.0, scaleLabel[SCALE_MAX_POWER - powerOf10]);
                parentWin->printAt(-tick - labelOffset, tickHeight * 1.2, 0.0, scaleLabel[SCALE_MAX_POWER - powerOf10]);
            }
            if (!vertical && MainWindow::getInterfaceType() == GeoCoastPilot)
                glColor4ub( 0, 0, 0, 255 ); //changed to black for GeoCoastPilot
            //draw the actual tick (make it triple wide if we're picking)
            for (i = -picking * 2; i <= picking * 2; i++)
            {
                glBegin(GL_LINES);
                    glVertex3f(tick + 2 * i, -tickHeight, 0.0);
                    glVertex3f(tick + 2 * i, tickHeight, 0.0);
                    glVertex3f(-tick + 2 * i, -tickHeight, 0.0);
                    glVertex3f(-tick + 2 * i, tickHeight, 0.0);
                glEnd(); // this seems to cause an "Conditional jump or move depends on uninitialised value(s)" error in valgrind
            }
        }
        currentPowerOf10 /= 10.0;
    }
    glPopName();
    if (vertical)
    {
        glPopMatrix();
    }

    glColor4ub( 200, 200, 200, 255 ); //chaned to grey to compensate for lack of lighting.

}




