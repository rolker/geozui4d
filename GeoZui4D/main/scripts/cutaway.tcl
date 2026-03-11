# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#-----------------------------------------------------------------------------
# Cutaway subwindow procedures
#
# Supports profile and cross-track views through use of clipping planes.
#
# Created by mdp 1/17/2003
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Procedure to create a new cutaway subwindow.
#
# "type" may be one of "profile" or "cross-track"
# possible "args" are as follows:
#	-lockTo <posable>
#	-cutWidth <width of cutaway>
#	-cutAngle <angle of cutaway for profile> 
#	-trackOffset <distance ahead>
#	-name <name>
#-----------------------------------------------------------------------------
proc createCutaway {parent type args} {
	global toglPath subWindowCount

	set lockTo {}
	set cutWidth 10
	set cutAngle 10
	set name "sub$subWindowCount"

	if {$type == "profile"} {
		set trackOffset 0
	} else {
		set trackOffset 50
	}

	set parentPath $toglPath($parent)

	#---Get the cutWidth and lockTo arguments, if any---
	foreach switch {cutWidth cutAngle lockTo trackOffset name} {
		set searchPattern "-$switch\[\t \]*(\[^\t \n\]*)"
		regexp -- $searchPattern $args entireSwitch $switch
		regsub -- $searchPattern $args "" args
	}

	#---Create the window---
	set cutaway $name ;# I don't know why the following line works sometimes and not others!
	set cutawayRootPath [$parentPath createSubwindow $name]

	#---Get the subwindow names out of the paths
	#set cutaway [string range $cutawayRootPath [expr 1 + [string last . $cutawayRootPath]] end]
	set cutawayPath $toglPath($cutaway)

	#---Attach the window to the passed posable---
	if {[string length $lockTo] > 0} {
		foreach {x y z h} [gz posable get $lockTo xyzh] {}
		if {$type == "profile"} {
			set heading [expr $h - 90]
		} else {
			set heading $h
		}
		set rh [expr 3.14159 * $h / 180]
		gz posable set Window.$cutaway pxyzh 0 [expr $x + sin($rh) * $trackOffset] [expr $y + cos($rh) * $trackOffset] $z $heading
		$cutawayPath lockTo $lockTo lht
	}

	#---Set up clipping planes---
	set width [expr $cutWidth / 2]
	if {$type == "profile"} {
		set rangle [expr 3.14159 * $cutAngle / 180]
		set x [expr sin($rangle)]
		set y [expr cos($rangle)]
		$cutawayPath clippingPlane 0 $x $y $width
		$cutawayPath clippingPlane 1 $x -$y $width
	} else {
		$cutawayPath clippingPlane 0 0 1 $width
		$cutawayPath clippingPlane 1 0 -1 $width
	}

	return $cutawayRootPath
}