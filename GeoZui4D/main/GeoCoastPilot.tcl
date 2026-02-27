#-----------------------------------------------------------------------------
# GUI setup
#-----------------------------------------------------------------------------

global pane
global menuWin
global objectFrame
global minLat maxLat minLon maxLon
global coastPilot
global maxScale minScale
global interactivityBase interactivityMotion
global interactivityMotionRequests
global speedFactor standardEmphasisFactor
global northUpMode
global configVariables
global keelDepth cautionDepth

#---set reasonable bounds on entry box
proc PortsmouthBounds {} {
	global minLat maxLat minLon maxLon maxScale minScale
	
	set minLat 43.0
	set maxLat 43.15
	set minLon -70.85
	set maxLon -70.65
	set maxScale 0.045
	set minScale 0.00002
}

proc BostonBounds {} {
	global minLat maxLat minLon maxLon maxScale minScale
	
	set minLat 42.2
	set maxLat 42.45
	set minLon -71.1
	set maxLon -70.8
	set maxScale 0.045
	set minScale 0.00002
}

if {$cautionDepth <= $keelDepth} {
	set keelDepth 10
	set cautionDepth 20
}

if {$speedFactor <= 0.0} {
	set speedFactor 1.0
}
if {$standardEmphasisFactor < 0.01} {
	set standardEmphasisFactor 0.55
}
set northUpMode 0

set GeoCoastPilotVersion "2.0"
set defaultFont "{Time New Roman} 10"

if {$GeoCoastPilotVersion < 2.0} {
	PortsmouthBounds
} else {
	BostonBounds
}

proc showMenu {} {
	global menuWin

	$menuWin config -menu .menubar
}

proc powerUser {} {
	global menuWin

	$menuWin config -menu .menubar
	pack .pane.f1.frame.objnb -fill both -expand true
	pack .timecontrol -side bottom -fill x
}

proc user {} {
	global menuWin
	
	$menuWin config -menu .cpMenu

	pack forget .pane.f1.frame.objnb
	pack forget .timecontrol
}

#Stand-in for later proc
proc rangeReached {} {
}

#--Remove object menu posting for right-clicks.
proc postObjectMenu {name classname objectname localX localY args} {}

#-----------------------------------------------------------------------------
# GeoCoastPilot menu
#-----------------------------------------------------------------------------
menu .cpMenu

#----File menu----
menu .cpMenu.file -tearoff false
.cpMenu add cascade -label File -menu .cpMenu.file
.cpMenu.file add command -label Exit -command {gz_exit menu}

#----Edit menu----
menu .cpMenu.edit -tearoff false
.cpMenu add cascade -label Edit -menu .cpMenu.edit
.cpMenu.edit add command -label Preferences... -command {wm deiconify .preferences}

#----Help menu----
menu .cpMenu.help -tearoff false
.cpMenu add cascade -label Help -menu .cpMenu.help
.cpMenu.help add command -label "GeoCoastPilot Help" -command {start "GeoCoastPilot Help" [file join $datadir ../../documentation "GeoCoastPilot User Manual.pdf"] &}
.cpMenu.help add separator
.cpMenu.help add command -label "About GeoCoastPilot" -command {wm deiconify .splash}

proc getSysInfo {} {
	#Start up a mail application to paste results in
	start test.msg
	#Open the system info panel and take a screen shot
	start %SystemRoot%/system32/sysdm.cpl; after 2000; gz sendkey Alt PrintScreen; gz sendkey Escape
	after 1000; gz sendkey Ctrl v; #DOESN'T WORK--Pastes, but doesn't send right!
	#Open the desktop info panel and take a screen shot
	start %SystemRoot%/system32/desk.cpl; after 2000; gz sendkey Shift Tab; foreach i "1 2 3 4 5" "after 1000; gz sendkey Right"; after 200; gz sendkey Alt PrintScreen; gz sendkey Escape
	#Paste the result
	after 1000; gz sendkey Ctrl v; #DOESN'T WORK--Pastes, but doesn't send right!

	#Another way?  Paint...
	start %SystemRoot%/system32/mspaint.exe
}

#-----------------------------------------------------------------------------
#Redefine bindings
#-----------------------------------------------------------------------------
set interactivityBase 0.5
set interactivityMotion 1.0
set interactivityMotionRequests 0

proc setInteractivity {which} {
	global interactivityHint interactivityBase interactivityMotion
	global interactivityMotionRequests
	
	if {$which == "motion"} {
		rangeReached
		incr interactivityMotionRequests -1
		if {$interactivityMotionRequests <= 0} {
			set interactivityHint $interactivityMotion
			set interactivityMotionRequests 0
		}
	} else {
		incr interactivityMotionRequests
		set interactivityHint $interactivityBase
	}
}

proc enableBindings {} {
	global mainWindow clutEditor

	bind $mainWindow <KeyPress>      "$mainWindow keyboard %x %y %s %A 0 %N"
	bind $mainWindow <KeyRelease>    "$mainWindow keyboard %x %y %s %A 1 %N"
	bind $mainWindow <Motion>        "$mainWindow motion %x %y %s 0"
	bind $mainWindow <ButtonPress>   "setInteractivity motion; focus $mainWindow; $mainWindow mouse %x %y %s %b 0"
	bind $mainWindow <ButtonRelease> "focus $mainWindow; $mainWindow mouse %x %y %s %b 1; setInteractivity base"
	bind $mainWindow <MouseWheel>    "focus $mainWindow; if {%D > 0} {$mainWindow mouse %x %y %s 4 0} else {$mainWindow mouse %x %y %s 5 0}"
	bind $mainWindow <Enter>         "$mainWindow motion %x %y %s 1"
	bind $mainWindow <Leave>         "$mainWindow motion %x %y %s -1"
	bind $clutEditor <Motion>        "$clutEditor motion %x %y %s 0"
	bind $clutEditor <ButtonPress>   "$clutEditor mouse %x %y %s %b 0"
	bind $clutEditor <ButtonRelease> "$clutEditor mouse %x %y %s %b 1"
	bind $clutEditor <Enter>         "$clutEditor motion %x %y %s 1"
	bind $clutEditor <Leave>         "$clutEditor motion %x %y %s -1"

	foreach name [subwindowVerifyList] {
		subwindowEnableBindings $name
	}
}

#---Set up full menu, but hide it for now---
source [file join $appdir "fullmenu.tcl"]
user; #hide menu
source [file join $appdir scripts "GeoCoastPilot_preferences.tcl"]

#---Set up power user stuff, but don't show---
source [file join $appdir scripts "GeoCoastPilot_extras.tcl"]

#----------------------------------------------------------------------
#--------togl widget for main display--------
#togl $mainWindow -stereo true -depth true -double true -width 100 -height 100 -ident main
set useStereo false
if {[catch "togl $mainWindow -stereo $useStereo -depth true -double true -width 100 -height 100 -ident main -stencil true"]} {
	puts "Disabling stereo"
	set useStereo false
	if {[catch "togl $mainWindow -depth true -double true -width 100 -height 100 -ident main -stencil true"]} {
		puts "Problem initializing Togl widget"
		set savedInfo $errorInfo
		error "Problem initializing Togl widget" $savedInfo
	}
}
$mainWindow configure -cursor hand2

createOptionMenu $mainWindow main

pack $mainWindow -expand true -fill both -side left

pack $pane -expand true -fill both -side top
#bind $pane <Double-1> {recordAndExecMaybe {global pane; $pane.f1 configure -width 300}}

#----------------------------------------------------------------------
#---tidbits---

#---Scale slider---
set scaleScrollFrame [frame .pane.f0.scaleScrollFrame]
image create bitmap bitmap_plus_scale -file [file join $appdir "bitmaps/plus_scale.bmp"]
set l [button $scaleScrollFrame.zoomIn -image bitmap_plus_scale -command "setScale scroll -1 pages"]
pack $l -side top
set scaleScroll [scrollbar $scaleScrollFrame.scaleScroll -command "setScale"]
pack $scaleScroll -fill y -expand yes -side top
image create bitmap bitmap_minus_scale -file [file join $appdir "bitmaps/minus_scale.bmp"]
set l [button $scaleScrollFrame.zoomOut -image bitmap_minus_scale -command "setScale scroll +1 pages"]
pack $l -side top
pack $scaleScrollFrame -fill y -expand no -side right

proc setScale {command value {units "none"}} {
	global minScale maxScale settingScale

	if {$command == "scroll"} {
		set scale [gz posable get Window.main s]
		if {$units == "units"} {
			set scale [expr pow($scale, pow(1.01, $value))]
		} else {
			set scale [expr pow($scale, pow(1.08, $value))]
		}
		if {$scale < $minScale} {
			set scale $minScale
		} elseif {$scale > $maxScale} {
			set scale $maxScale
		}
		gz posable set Window.main s $scale
	} else {
		if {$value > 0.9} {
			set value 0.9
		}
		set value [expr $value / 0.9]
		gz posable set Window.main s [expr $minScale * pow($maxScale/$minScale, 1.0-$value)]
	}
}

proc updateScale {} {
	global scaleScroll minScale maxScale

	set scale [gz posable get Window.main s]
	set location [expr 1.0 - log($scale / $minScale) / log($maxScale / $minScale)]
	set location [expr $location * 0.9]
	$scaleScroll set $location [expr $location + 0.1]
	update
}

gz event handler create trackScale for viewChanged {
	#scale has value 8388671
	if {%iM == 8388671} {
		updateScale
	}
}
gz tellObject .EventMediator.Window.main push trackScale

#---Attribute editor---
attributeWindow .editPoint .Interactor.pointHandle.editPoint -destroyable 0 -wrapper 2 -showHandler {gz tellObject .EventMediator.user push editPointSelection} -hideHandler {gz tellObject .EventMediator.user remove editPointSelection}
gz event handler create editPointSelection for objectSelected {
	global attribFullObject
	if {%iM < 0} {
		set attribFullObject(editPoint) %V
		set localstr ""
	} else {
		set attribFullObject(editPoint) .Interactor.pointHandle.editPoint
		set localstr "wrapper around "
	}
	subwindowTitle editPoint "Attributes for Edit-Target ($localstr[gz tellObject %V objectType])"
	attributeUpdate editPoint 1 1
}
gz tellObject .EventMediator.user push editPointSelection
subwindowMinimize editPoint

#----------------------------------------------------------------------
#---GUI parts---
source [file join $appdir scripts GeoCoastPilot_control_panel.tcl]
source [file join $appdir scripts CFR_procs.tcl]
source [file join $appdir scripts GeoCoastPilot_text_and_procs.tcl]

#----------------------------------------------------------------------
#---Main window setup---
#---this can't be put in visualizer.tcl without getting the mainWindow in front of the splashscreen
wm geometry . 1024x768
wm protocol . WM_DELETE_WINDOW {gz_exit wm}
focus $coastPilot
pack forget .timecontrol
update
$pane.f1 configure -height 288

#------------------Set up the color key----------------------
proc setupColorKey {} {
	global colorKey mainWindow
	
	$colorKey editclut 1
	subwindowTitle key "Depth (feet)"
	$colorKey keydisplay units f
	$colorKey keydisplay buffer 45
	refreshColorKey
 	subwindowRestore key
	subwindowResizeAndMove key 70 130 0 332
	bind $mainWindow <Configure> "updateColorKeyFor %w %h"
}

proc updateColorKeyFor {w h} {
	subwindowResizeAndMove key 70 130 0 [expr $h - 126]
}

proc refreshColorKey {{name1 {}} {name2 {}} {op w}} {
	global colorKey keelDepth cautionDepth
	
	set bottom [expr $cautionDepth / -3.2808399]
	set mid [expr $keelDepth / -3.2808399]
	if {$mid <= $bottom} {
		puts "Not updating the color key: keelDepth must be less than cautionDepth!"
		return
	}
	$colorKey clutelement 0 setHeight $bottom
	$colorKey clutelement 1 setHeight $mid
	$colorKey keydisplay max 0
	$colorKey keydisplay min $bottom
	$colorKey keydisplay step 0.3048
	redisplay
}

trace variable cautionDepth w refreshColorKey
trace variable keelDepth w refreshColorKey

#---Splash Screen---
toplevel .splash
wm title .splash "GeoCoastPilot"
wm protocol .splash WM_DELETE_WINDOW {wm withdraw .splash}
canvas .splash.canvas -height 300 -width 400
image create photo splashPhoto -file [file join $appdir "GeoCoastPilot_splash.gif"] -height 300 -width 400
.splash.canvas create image 200 150 -image splashPhoto
#label .splash.versionLabel -text "$GeoZuiDescriptor"
label .splash.versionLabel -text "GeoCoastPilot Version $GeoCoastPilotVersion"
label .splash.copyright -text "Copyright (c) 2008.  All rights reserved.
 Not to be copied or distributed without written agreement.
 Contact:  briana@ccom.unh.edu or schwehr@ccom.unh.edu"
button .splash.okbutton -text "OK" -command "wm withdraw .splash" -takefocus 1
pack .splash.canvas -expand yes -fill both
pack .splash.versionLabel
pack .splash.copyright
pack .splash.okbutton -pady 10
bind .splash <KeyRelease-Return> ".splash.okbutton invoke"
wm deiconify .splash
after 8000 {wm withdraw .splash}
update

#---Allow user interaction---
bind . <KeyPress-Q> {gz_exit Q}
#$mainWindow keyboard -1 -1 0 w 0 w
$mainWindow showWatermark X 0.074
$mainWindow showCompass X 0.95 Y 0.05 S 4

#---Ground the common functions from other interfaces---
proc gnAttachToVessel {args} {}

#---Set the current projection---
#set projection UTM.ZONE19
#gz projection set $projection
gz projection setCurrent "+proj=utm +ellps=WGS84 +zone=19"
gz posable set Window.main.perspective s 3
foreach {minX minY} [gz projection apply $projection $minLon $minLat] {}
foreach {maxX maxY} [gz projection apply $projection $maxLon $maxLat] {}

#---set reasonable bounds on where Window.main will actually go (matches entry box ones pretty close)
#Note: if you change these, be sure to change them for .Animator.direction, too!
gz posable set Window.main Mpxyzs 90 $maxX $maxY 30.0 $maxScale
gz posable set Window.main mpxyzs 8 $minX $minY 4.0 $minScale
gz posable set Window.main.perspective s 1.633

#---Set up additional user interaction---
gz animator create .Animator mover ImitatingAnimator Window.main; #to move the window toward features
gz posable create .Animator direction window
gz posable set .Animator.direction Mxyz $maxX $maxY 30.0
gz posable set .Animator.direction mxyz $minX $minY 4.0
gz animator create .Animator turner ImitatingAnimator Window.main .Animator.direction
gz tellObject .Animator.turner effect h
gz tellObject .Animator.turner speed [expr $speedFactor * 15]
gz tellObject .Animator.mover activate 0
gz tellObject .Animator.turner activate 0

gz newobject ProximityHighlighter rangeFinder; #to stop the window before it gets to something
gz tellObject rangeFinder manage Window.main
gz tellObject rangeFinder hide
gz event handler create rangeReachedHandler for inProximity rangeReached
gz tellObject .EventMediator.user push rangeReachedHandler

gz newobject ProximityHighlighter encounterChecker; #to stop the window if it gets all the way to something
gz tellObject encounterChecker manage Window.main
gz tellObject encounterChecker manage .Animator.direction
gz tellObject encounterChecker hide
gz tellObject encounterChecker threshold 1.0
gz event handler create encounterHandler for inProximity rangeReached
gz event handler create encounterHandler2 for inCloseProximity rangeReached
gz tellObject .EventMediator.user push encounterHandler

gz event handler create objectClicked for objectClicked "objectClicked %V"
gz tellObject .EventMediator.user push objectClicked

gz event handler create doubleClick for doubleClick "doubleClicked %iIX %iIY"
gz tellObject .EventMediator.internal push doubleClick
