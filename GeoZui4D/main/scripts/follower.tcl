#--------------------------------------------------------------------------
# This is a modifyable example script that allows a vehicle to be attached
# to a .gdl object.  The first section sets up variables which can be
# edited for quick modification of the behavior of this script.
#--------------------------------------------------------------------------
global mainWindow subWindowCount toglPath


#---Modify these variables to change what this script acts on.---
set vehicle Object.Sample1_sample;	#What's going to follow the path (must be loaded prior to running script)
set path Object.ROV4_gdl;		#What's the path to be followed (must be loaded prior to running script)
set splines 0;				#How should the path be interpolated? 1 for spline, 0 for simple linear interpolation
set speed 0.015;				#How fast should the vehicle go (relative to the spacing of the points on the path)
set createWindow 1;			#1 to create window, 0 to skip it
set initialScale 100;			#Zoom factor of 100 (screen covers about 15-20 meters)
set wrapAround 1;				#Should the path-following restart when the end is reached? 1 for yes, 0 for no


#---Create a new subwindow---
if {$createWindow} {
	set subwinName "sub$subWindowCount"
	$mainWindow createSubwindow
	set subwin $toglPath($subwinName)

	#---Move the new window's center of workspace to the desired object
	set setstr xyzh
	eval "gz posable set Window.$subwinName $setstr [gz posable get $vehicle $setstr]"; #move subwindow to object position
	$subwin lockTo $vehicle thl; #instantiate desired lock
	gz posable set Window.$subwinName s $initialScale
}

#---Create a wrapper to follow the path.---
gz posable create .Posable wrapper translate_wrapper
gz tellObject wrapper attachTo $path
gz tellObject wrapper index dimensions 2
gz tellObject wrapper index set 0 as Float
if {$splines} {
	gz tellObject wrapper index 1 set cubic_spline as String
}

#---Create the animator and make it move the vehicle along the path---
gz animator create .Animator follow PathAnimator $vehicle wrapper
gz tellObject .Animator.follow effect xyzhp
gz tellObject .Animator.follow speed $speed

#--------------------------------------------------------------
if {$wrapAround} {
	gz event handler create wrapAround for interpolationBounds "wrapAround %V"
	gz tellObject .EventMediator.user push wrapAround
}

proc wrapAround {posable} {
	if {$posable == ".Posable.wrapper"} {
		gz tellObject wrapper index set 0
	}
}
