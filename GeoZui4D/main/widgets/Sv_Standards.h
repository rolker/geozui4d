#ifndef SV_STANDARDS_H
#define SV_STANDARDS_H

/****************************** Sv_Standards.h *******************************/

//#include "glui_v2/glui.h" 
#include "SDV_types.h"
#include <math.h>

/*********************************/
/* Monitor display global values */
/*********************************/

const GLint YSTEREOMAX	= 491;
const GLint YOFFSETSTEREO = 532;
const GLint XSCREENMAX   = 1279;
const GLint XSCREENMAX_2 = (int)(XSCREENMAX/2.0);
const GLint YSCREENMAX   = 1023;
const GLint YSCREENMAX_2 = (int)(YSCREENMAX/2.0);
const GLint ZBUFMAX      = 8388607;
const GLint ZBUFMIN      = -8388608;

/*********************************/
/* display types (for stereo/HC) */
/*********************************/

const GLshort SV_DISPLAY_19INCH		 = 0, 
	      SV_DISPLAY_LARGE_MONITOR	 = 0, 
	      SV_DISPLAY_15INCH		 = 1, 
	      SV_DISPLAY_SMALL_MONITOR	 = 1, 
	      SV_DISPLAY_PROJECTION	 = 2, 
	      SV_DISPLAY_PROJECTOR	 = 2, 
	      SV_DISPLAY_BIG_SCREEN	 = 2, 
	      SV_DISPLAY_REAR_PROJECTION = 3, 
	      SV_DISPLAY_REAR_PROJECTOR	 = 3, 
	      SV_DISPLAY_REAR_BIG_SCREEN = 3;
	    
/**********************************************/
/* unit conversion constants, for I/O devices */
/**********************************************/

const GLfloat CMTOPIXEL   = (GLfloat)37.647060;
const GLfloat INCHTOCM    = (GLfloat)2.54;
#ifndef M_PI //This has to be here because Windows does not have M_PI defined
const GLfloat M_PI        = (GLfloat)3.14159265359;
#endif
const GLfloat INCHTOPIXEL = INCHTOCM*CMTOPIXEL;

const GLfloat SV_CMTOPIXEL   = CMTOPIXEL;
const GLfloat SV_INCHTOCM    = INCHTOCM;
const GLfloat SV_INCHTOPIXEL = INCHTOPIXEL;

/**************************************************/
/* math functions not already defined in <math.h> */
/**************************************************/

#define sgn(a) ((a) >= 0 ? 1 : -1)
#define sgnf(a) ((a) >= 0.0 ? (GLfloat)1.0 : (GLfloat)-1.0)

/****************************/
/* device hierarchy-related */
/****************************/

const GLshort	SV_DEVICE_LOGITECH	= 100, 
		SV_DEVICE_LOGIHANDLE	= 101,
		SV_DEVICE_BAT		= 200, 
		SV_DEVICE_ISOTRAKII	= 201, 
		SV_DEVICE_ISOTRAKII_1	= 201, 
		SV_DEVICE_ISOTRAKII_2	= 202, 
		SV_DEVICE_FASTRAK	= 300, 
		SV_DEVICE_FASTRAK_1	= 300, 
		SV_DEVICE_FASTRAK_2	= 301, 
		SV_DEVICE_FASTRAK_3	= 302, 
		SV_DEVICE_FASTRAK_4	= 303, 
		SV_DEVICE_ADL		= 400, 
		SV_DEVICE_NULL		= 0;

const GLshort SV_DEVICE_MAXIMUM_COMMANDS = 32;   // 32 char. array is maximum command length
const GLshort SV_DEVICE_CMD_MODE	= 50,
	      SV_DEVICE_CMD_UNITS	= 51,
              SV_DEVICE_CMD_FLUSH	= 52,   // For flushing the device
              SV_DEVICE_CMD_CMD		= 99;   // signal a general command
	    
const GLshort SV_DEVICE_MODE_DEFAULT     = 100,
              SV_DEVICE_MODE_FLYING      = 101,
              SV_DEVICE_MODE_ORIENTATION = 102;
	    
const GLshort SV_DEVICE_UNITS_CM	= 100, 
	      SV_DEVICE_UNITS_INCHES	= 101, 
	      SV_DEVICE_UNITS_DU	= 102,  // device units
	      SV_DEVICE_UNITS_PIXELS	= 103;

const GLshort SV_DEVICE_QUAT_W = 3, 
	      SV_DEVICE_QUAT_X = 4, 
	      SV_DEVICE_QUAT_Y = 5, 
	      SV_DEVICE_QUAT_Z = 6;
	    
const GLshort SV_DEVICE_LEFTBUTTON   = 7,     // Values correspond to button
              SV_DEVICE_MIDDLEBUTTON = 8,     // locations in shared memory
              SV_DEVICE_RIGHTBUTTON  = 9,     // buffer.
              SV_DEVICE_SIDEBUTTON   = 10;


const GLshort SV_DEVICE_ERR1	= 11, 
              SV_DEVICE_ERR2	= 12, 
	      SV_DEVICE_CMDTYPE	= 13,
	      SV_DEVICE_CMDVAL	= 14, 
	      SV_DEVICE_CMDDATA	= 15; 

const GLint   SV_DEVICE_ERROR	= -1, 
	      SV_DEVICE_NOERROR	=  0;
	    
const GLint   SV_DEVICE_IPC_SERVER = -2,
              SV_DEVICE_IPC_CLIENT = -3;


    // number of floating-point numbers

const GLint SV_SHMEM_STRUCTURE_SIZE = 15+SV_DEVICE_MAXIMUM_COMMANDS;
const GLint SV_SHMEM_SEGMENT_SIZE   = SV_SHMEM_STRUCTURE_SIZE*sizeof(float);

const GLshort SV_LOGITECH_GET_RECEIVER_TYPE = 300;

/**************************/
/* other useful functions */
/**************************/

/************************/
/* other global defines */
/************************/

const short X = 0,
	    Y = 1, 
	    Z = 2;

const short Yaw = Y,
		Pitch = X,
		Roll = Z;

const double DEGTORAD = M_PI/180.0;
const double RADTODEG = 180.0/M_PI;

#define SV_TRUE  1
#define SV_FALSE 0

#define SV_EPSILON 1.0E-15
	    
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif
