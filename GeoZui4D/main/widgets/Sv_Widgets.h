///////////////////////////////////////////////////////////////////////////////
// Sv_Widgets.h
///////////////////////////////////////////////////////////////////////////////
/**
 *	This class implements a rotation interactor with 2 degrees of freedom,
 *	for operating on ViewManagers.
 *	This class implements the following responsibilities:
 *	*	Display and maintain rotation widgets.
 *	*	Provide mechanisms for customizing the look of the widgets.
 *
 *	Created by ???
 *	Modified by mdp 7/21/2000 (Removed chaff, integrated ViewManager)
 */
///////////////////////////////////////////////////////////////////////////////

class SV_Widgets; //this line connects the above comments with this class.

#ifndef SV_WIDGETS_H
#define SV_WIDGETS_H

#include<gz4d/glo.h>
#include <math.h>
#include "Sv_Slider3.h"
#include "widgets/Sv_Matrix4.h"
#include "widgets/Sv_Standards.h"
#include "../engine/Interactor.h"
#include "SDV_types.h"
#include <gz4d/Point.h>

class ViewManager;
class Window3D;
class PosableStateBase;
class LightDirector;

const GLshort  SV_WIDGETS_IDENTIFIER = 733;    // an unusual number, unlikely to be used elsewhere

const GLshort	SV_WIDGETS_YROTATION_WIDGET	= 0; 
	    
const GLshort  NUMBEROFWIDGETS = 1;
	    
const GLshort	SV_WIDGETS_ENTER	= 0, 
				SV_WIDGETS_EXIT		= 1, 
				SV_WIDGETS_DRAG		= 2, 
				SV_WIDGETS_LOCK		= 3, 
				SV_WIDGETS_NOACTION	= 4;

class Sv_Widgets : public Interactor
{
   public:
		//Ctor and dtor
		Sv_Widgets(ViewManager *targetView, GLfloat theSensitivity,
				   GLshort theSelectButton = MouseButton1,
				   GLshort theLockButton = MouseButton2,
				   GLshort theVideo = FALSE );

		~Sv_Widgets(void);
		int RegisterWithTcl(ToglWindow *throughWin);

		//Member functions
		void UpdateWidgets( GLint thePosX, GLint thePosY, GLshort but2state );
		void UpdateWidgets();
		void UpdatePose();
		void Resize();
		void CallBackMouseFunc(int button, int state, int x, int y);
		void CallBackMotionFunc(int x, int  y);
		void CallBackIdleFunc();
		int HandleInput(unsigned int inputType, InputInfo *info); //added by mdp 3/2/2000

		GLshort CheckWidgets( GLshort buttonstate, GLint thePosX, GLint thePosY );
		
		void DrawWidgets();
		void DrawCompass(gz4d::Point<double> const &orient);
		void ResetWidgets();
		GLint GetWidgetsLocked( void )	{ return azimuthLock; }   // return # of locked widgets
		GLshort GetWidgetStatus( void )	{ return itsWidgetStatus; }
		virtual void GetWindow( GLint = -1, GLint = -1);
		
		void SetRotate( float angx, float angy, float angz = 0);
		void SetScale( GLfloat sval );
		
		void SetVelocityRatio( GLshort which, GLfloat val );
		void ViewHome( void );
			
		void SetVelocityPointerStyle( GLshort style );	
		void SetPositionPointerType( GLshort type );	
		void SetPositionPointerStyle( GLshort style );	

		void DrawScale(GLfloat width, GLboolean vertical = GL_FALSE, GLfloat adj_angle = 0.0f);

	protected:
		void AdjustWidgets(GLint xdelta, GLint ydelta, GLshort lockButtonState );
		virtual GLint DoneWidgets( void )   { return 1; }
		virtual void SetMouse( void )	    {;}
		virtual void ResetMouse( void )	    {;}
		GLint  PickWidgets( GLuint *buffer, GLint hits );
		GLint  ReleaseWidgets(void);
		//void   CalculateTurnOverAngle();//when the mouse motion should be map in reverse order

		GLint   itsOldMouseX, itsOldMouseY;
		GLint   itsMouseX, itsMouseY;
		GLint   itsWindowHeight;
		GLfloat itsAspectRatio;
		Sv_Slider3 *itsSlider;
		GLshort itsWidgetIdentifier;

		GLshort itsWidgetPart;

		GLshort azimuthLock;// itsLockedWidget[NUMBEROFWIDGETS];
		GLshort itsSelectButton, itsLockButton;
		//GLfloat itsTurnOverAngle;

		GLfloat lpos[4]; //for brightening up the widgets (mdp)

		int HighlightTab;
		
		static bool *isVisible;

		GLshort itsButtonStatus, itsWidgetStatus;
		GLshort itsActiveButton;  // which mouse button is pressed last
		GLshort OrientFlag;
		GLboolean ignoreAdjustment;
		GLboolean picking;

		// extra info for picking
		GLfloat itsCurrentProjectionMatrix[16];
		GLdouble itsCurrentProjectionMatrixd[16];
		GLdouble itsCurrentModelMatrixd[16];
		GLint itsCurrentViewport[4];

		ViewManager *currentView;
		Window3D *parentWin;
		PosableStateBase *currentPose;
		LightDirector *lightDirector;

		float light_position[4];  // the widgets have their own lighting
		float widgetdiffuse[4];
		float widgetambient[4];

		// options
		static int showCompass;
		static int showHorizontalScale;
		static int showVerticalScale;
		static int shallowAngleCompensation;
		static int showLightingControl;
		static double selectionHeight, selectionWidth;

};
#endif
