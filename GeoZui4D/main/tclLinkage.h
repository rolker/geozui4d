#if 0
set comment {
#endif
// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// tclLinkage.h
///////////////////////////////////////////////////////////////////////////////
/** \file
 *	This file defines some strings that are used in both the C++ and Tcl
 *	code. This allows the text to be changed without changing both sets of code.
 *	The \#if 0 portion allows Tcl code to be mixed into the header file,
 *	since this file is also sourced by Tcl, so be careful what you change.
 *	If you don't know tcl, just keep to the \#define list in the table below!
 *
 *	\NOTE THIS FILE SHOULD NOT BE CHANGED BY PEOPLE ONLY EDITING THE TCL SIDE.
 *		  BOTH TCL AND C++ NEED TO SEE THE CHANGES.
 *
 *	Created by mdp 8/14/2000
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TCL_LINKAGE_H
#define TCL_LINKAGE_H

#if 0
}

set variables {
#endif	/*--VARIABLE--*/            /*-------------VALUE-------------*/
#define	LABEL_OVERVIEW                "Show Overview"
#define	LABEL_RESTORE_SUBS            "Restore Subwindows"
#define	LABEL_MINIMIZE                "Minimize"
#define	LABEL_LOCK_LIGHTING           "Lock Lighting"
#define	LABEL_UNLOCK_LIGHTING         "Unlock Lighting"
#define	LABEL_STATIC_TETHERS          "Static Tethers"
#define	LABEL_DYNAMIC_TETHERS         "Dynamic Tethers"
#define	LABEL_CLOSE                   "Close"
#define	SUBWINDOWS_SEPARATE           0
#define	SUBWINDOWS_FOLLOW_PARENT      1
#define	SUBWINDOWS_CLIPPED_BY_PARENT  3
#define	SUBWINDOW_HEIGHT              300
#define	SUBWINDOW_WIDTH               400
#define	RECORDER_STOPPED              0
#define	RECORDER_RECORDING            1
#define	RECORDER_PLAYING              2
#define	RECORDER_PAUSED               4
#define	RECORDER_PAUSED_RECORDING     5
#define	RECORDER_PAUSED_PLAYING       6
#define	TEXTURE_COORD_USER            0
#define	TEXTURE_COORD_POSABLE         1
#define	TEXTURE_COORD_GEOREF          2
#define	TEXTURE_COORD_NONE            3
#if 0	/*-------------------------------------------------------*/
}

foreach {junk variable value} $variables {
	if {[string compare $junk "#define"] == 0} {
		set $variable $value
	}
}
unset variables
#endif

#endif
        
