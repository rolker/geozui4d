# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#package require -exact snack 2.0
global mainWindow

#set some important variables.
set normal_mode 1
set placeClickedText ""

#lets see if the dirs got set properly...
if {$appdir == ""} {set appdir [pwd]}
puts "appdir:  $appdir"
puts "datadir: $datadir"
#lappend auto_path [file join $appdir "BWidget-1.4.1"]
lappend auto_path [file join $appdir "BWidget-1.7.0"]
package require BWidget

#load the linkage to C++
source [file join $appdir "tclLinkage.h"]

#load the remaining components
source [file join $appdir "visualizer_procs.tcl"]
source [file join $appdir "clutEditor.tcl"]
source [file join $appdir "subtogl.tcl"]
source [file join $appdir "scripts/cutaway.tcl"]
source [file join $appdir "scripts/attributeWindow.tcl"]

#lappend auto_path [file join $appdir "Tktable2.9"]
#package require Tktable;

#if {$tcl_platform(platform) == "windows" && $tcl_patchLevel != "8.4.12"} {
#	puts "tcl-warning" 1 "####Warning: This version of GeoZui4D is built for Tcl/Tk 8.4.12,###\n ####but is trying to use $tcl_patchLevel dll's!###"
#}

#specify variables to be recorded in GeoZui3D.conf
global configVariables

set configVariables {screenWidth_pix screenHeight_pix screenLeft screenRight screenTop screenBottom}
set configReadOnlyVariables {appdir iorFilename}

#---timezone stuff---
array  set timezones [list {ACDT} {10.5}\
                           {ACST} {9.5}\
									{ADT} {-3}\
									{AEDT} {11}\
									{AEST} {10}\
									{AKDT} {-8}\
									{AKST} {-9}\
									{AST} {-4}\
									{AWST} {8}\
									{BST} {1}\
									{CDT} {-5}\
									{CEST} {2}\
									{CET} {1}\
									{CST} {-6}\
									{CXT} {7}\
									{EDT} {-4}\
									{EEST} {3}\
									{EET} {2}\
									{EST} {-5}\
									{GMT} {0}\
									{HAA} {-3}\
									{HAC} {-5}\
									{HADT} {-9}\
									{HAE} {-4}\
									{HAP} {-7}\
									{HAR} {-6}\
									{HAST} {-10}\
									{HAT} {-2.5}\
									{HAY} {-8}\
									{HNA} {-4}\
									{HNC} {-6}\
									{HNE} {-5}\
									{HNP} {-8}\
									{HNR} {-7}\
									{HNT} {-3.5}\
									{HNY} {-9}\
									{IST} {1}\
									{MDT} {-6}\
									{MESZ} {2}\
									{MEZ} {1}\
									{MST} {-7}\
									{NDT} {-2.5}\
									{NFT} {11.5}\
									{NST} {-3.5}\
									{PDT} {-7}\
									{PST} {-8}\
									{UTC} {0}\
									{WEST} {1}\
									{WET} {0}\
									{WST} {8}\
									{UTC-12} {-12}\
									{UTC-11} {-11}\
									{UTC-10} {-10}\
									{UTC-9} {-9}\
									{UTC-8} {-8}\
									{UTC-7} {-7}\
									{UTC-6} {-6}\
									{UTC-5} {-5}\
									{UTC-4} {-4}\
									{UTC-3} {-3}\
									{UTC-2} {-2}\
									{UTC-1} {-1}\
									{UTC+1} {1}\
									{UTC+2} {2}\
									{UTC+3} {3}\
									{UTC+4} {4}\
									{UTC+5} {5}\
									{UTC+6} {6}\
									{UTC+7} {7}\
									{UTC+8} {8}\
									{UTC+9} {9}\
									{UTC+9:30} {9.5}\
									{UTC+10} {10}\
									{UTC+10:30} {10.5}\
									{UTC+11} {11}\
									{UTC+12} {12}\
									]


if {![info exists timezone]} {
    global timezone
	set timezone UTC
}


#-----------------------------------------------------------------------------
# GUI setup
#-----------------------------------------------------------------------------
global guiMode
if {![info exists guiMode]} {
	set guiMode "zui"
}
#set guiMode wall
if {$guiMode == "nav"} {
	wm title . "GeoNav3D"
	set mainWindow .mainPanel.glwindow
	initSubtogl
	source "GeoNav.tcl"
} elseif {$guiMode == "cp"} {
	wm title . "GeoCoastPilot"
	PanedWindow .pane -side left
	global mwframe
	global pane
	set pane .pane
	set mwframe [.pane add -weight 1]
	set mainWindow $mwframe.glwindow
	global menuWin
	set menuWin .
	initSubtogl
	source [file join $appdir "GeoCoastPilot.tcl"]
} elseif {$guiMode == "wall"} {
	wm title . "GeoZui4D Extreme GeoWall2 Edition"
	toplevel .backscreen

	PanedWindow .backscreen.pane
	global pane
	set pane .backscreen.pane
	
	set mwframe [.backscreen.pane add -weight 1]
	#wm overrideredirect . yes
#	wm attributes . -topmost yes   
	set mainWindow .glwindow
	initSubtogl
	
	global menuWin
	set menuWin .backscreen
#	source [file join $appdir "GeoWall2.tcl"]
	source [file join $appdir "GeoZui.tcl"]
	source [file join $appdir "notes.tcl"]
	
	set mainWidth [winfo screenwidth .]
	set mainHeight [expr [winfo screenheight .] / 2]
	wm geometry . [winfo screenwidth .]x[winfo screenheight .]+0+0
	pack forget .timecontrol

	#---hack!!
	pack $mainWindow -side top
	global toglPath useStereo

	set procBody [info body subtogl]
	set procArgs [info args subtogl]

	#re-direct subtogl
	set name child
	set parent .
	proc subtogl {windowPath args} "
		global absoluteWindowPath windowContents toglPath tetherEvent parentName

		set iname \[string range \$windowPath \[expr 1 + \[string last . \$windowPath\]\] end\]
		eval \"togl .$name -double true -depth true -stereo $useStereo -sharecontext main -ident \$iname -width $mainWidth -height $mainHeight -stencil true\"
		set absoluteWindowPath($name) .$name
		set windowContents($name) 3D
		set toglPath($name) .$name
		set parentName($name) top
		set tetherEvent($name) \"\"
		subwindowEnableBindings $name 0
		.$name positionUpdated 50 50 200 200
		.$name inParent tethers show 0
		.$name inParent proxy show 0
		.$name inParent flagpole show 0
		.$name inParent center show 0
		pack .$name -side bottom
 	"

	set parent $toglPath(main)

	#create a window to stuff in a pane
	$parent createSubwindow $name

	#re-enable old subtogl
	proc subtogl $procArgs $procBody

	bind .$name <Unmap> ".$name priority 0.0"
	bind .$name <Map> ".$name priority restore"
	
	gz posable set Window.child s [gz posable get Window.main s]
    $mainWindow lockTo Window.child THPRS+
        global screenWidth_pix screenHeight_pix screenLeftChild screenRightChild screenTopChild screenBottomChild
        $toglPath(child) screenSpecs $screenWidth_pix $screenHeight_pix $screenLeftChild $screenRightChild $screenTopChild $screenBottomChild
} else {
	wm title . "GeoZui4D"
	PanedWindow .pane
	global mwframe
	global pane
	set pane .pane
	set mwframe [.pane add -weight 1]
	set mainWindow $mwframe.glwindow
	global menuWin
	set menuWin .
	initSubtogl
	source [file join $appdir "GeoZui.tcl"]
	source [file join $appdir "notes.tcl"]
}


puts "Timezone: $timezone"
gztime timezone $timezones($timezone)

#----Time control---
#togl .timecontrol -depth true -double true -width 100 -height 100 -ident timecontrol
#pack .timecontrol -side bottom -fill x


#----Time control---
frame .timeframe

frame .timeframe.buttons

image create photo x10_pic -file [file join $appdir "bitmaps/x10.ppm"]
button .timeframe.buttons.x10button -image x10_pic -command "gztime multscale 10"
pack .timeframe.buttons.x10button -side right

image create photo x2_pic -file [file join $appdir "bitmaps/x2.ppm"]
button .timeframe.buttons.x2button -image x2_pic -command "gztime multscale 2"
pack .timeframe.buttons.x2button -side right

image create photo x0_5_pic -file [file join $appdir "bitmaps/x0.5.ppm"]
button .timeframe.buttons.x0_5button -image x0_5_pic -command "gztime multscale 0.5"
pack .timeframe.buttons.x0_5button -side right

image create photo x0_1_pic -file [file join $appdir "bitmaps/x0.1.ppm"]
button .timeframe.buttons.x0_1button -image x0_1_pic -command "gztime multscale 0.1"
pack .timeframe.buttons.x0_1button -side right

image create photo rev_pic -file [file join $appdir "bitmaps/rev.ppm"]
button .timeframe.buttons.revbutton -image rev_pic -command "gztime rev"
pack .timeframe.buttons.revbutton -side right

image create photo rt_pic -file [file join $appdir "bitmaps/rt.ppm"]
button .timeframe.buttons.rtbutton -image rt_pic -command "gztime rt"
pack .timeframe.buttons.rtbutton -side right

image create photo play_pic -file [file join $appdir "bitmaps/play.ppm"]
button .timeframe.buttons.playbutton -image play_pic -command "gztime play"
pack .timeframe.buttons.playbutton -side right

image create photo stop_pic -file [file join $appdir "bitmaps/stop.ppm"]
button .timeframe.buttons.stopbutton -image stop_pic -command "gztime stop"
pack .timeframe.buttons.stopbutton -side right

image create photo end_loop_pic -file [file join $appdir "bitmaps/endLoop.ppm"]
button .timeframe.buttons.endloopbutton -image end_loop_pic -command "gztime endloop"
pack .timeframe.buttons.endloopbutton -side right

image create photo loop_pic -file [file join $appdir "bitmaps/Loop.ppm"]
button .timeframe.buttons.loopbutton -image loop_pic -command "gztime loop"
pack .timeframe.buttons.loopbutton -side right

image create photo dont_loop_pic -file [file join $appdir "bitmaps/dontLoop.ppm"]
button .timeframe.buttons.dontloopbutton -image dont_loop_pic -command "gztime dontloop"
pack .timeframe.buttons.dontloopbutton -side right

image create photo start_loop_pic -file [file join $appdir "bitmaps/startLoop.ppm"]
button .timeframe.buttons.startloopbutton -image start_loop_pic -command "gztime startloop"
pack .timeframe.buttons.startloopbutton -side right

label .timeframe.buttons.timeDisplay -textvariable GZtimestr -width 24 -justify right -anchor e
pack .timeframe.buttons.timeDisplay -side right -expand yes -fill x -pady 2 -padx 1

scale .timeframe.timescale -orient horizontal -from 0.0 -to 1.0 -resolution 0.0000001 -showvalue 0 -variable GZtime -command "gztime timescale"
pack .timeframe.timescale -fill x -expand yes -side top
pack .timeframe.buttons -side top
#pack .timeframe -side bottom -fill x


#---Allow user interaction---
enableBindings

#---Set up the idle events---
after idle idleProc

#---Source user commands---
if {[file isfile "user.tcl"]} {
	source "user.tcl"
}

