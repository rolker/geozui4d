# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#-----------------------------------------------------------------------------
# GeoNav.tcl -- Interface script for the GeoNav GUI of GeoZui4D.
#-----------------------------------------------------------------------------
source [file join $appdir "pane_procs.tcl"]

#-----------------------------------------------------------------------------
# GUI setup
#-----------------------------------------------------------------------------
set defaultFont "{Time New Roman} 12"

#-----------------------
#--------menubar--------
menu .menubar
. config -menu .menubar

#----file menu----
menu .menubar.file -tearoff false
.menubar add cascade -label File -menu .menubar.file

set objectTypeAlias(gutm) "Gridded UTM"
set objectTypeAlias(dem) "USGS DEM"
set objectTypeAlias(strip) "Polygon Strip"
set objectTypeAlias(sdv) "Scene files"
set objectTypeAlias(img) "ROV Images"

.menubar.file add command -label "Open Object" -command {loadobj [tk_getOpenFile -initialdir $datadir -multiple 1 -filetypes [getObjectFilterList]]}
.menubar.file add separator
.menubar.file add command -label Exit -command {gz_exit menu}

#----view menu----
menu .menubar.view -tearoff false
.menubar add cascade -label View -menu .menubar.view
.menubar.view add checkbutton -label Stereo -onvalue 1 -offvalue -1 -variable stereoflag -command {redisplay; recordMaybe "set stereoflag $stereoflag"}
.menubar.view add separator
.menubar.view add command -label Background -command {changebg $mainWindow}
.menubar.view add separator
.menubar.view add command -label "Profile" -command {createSideview 5}
.menubar.view add command -label "Crosstrack" -command {createRearview 6}

#----options menu----
menu .menubar.options -tearoff false
.menubar add cascade -label "3D Options" -menu .menubar.options
proc addOption {name {type {}} {commandOrItemValuePairs {}} {variable {}} {onvalue 1} {offvalue 0} {menupath {}}} {

	if {[string length $menupath] == 0} {
		set path .menubar.options
	} else {
		set path .menubar.options.$menupath
	}

	if {$name == ""} {
		$path add separator
		return
	}

	if {[string compare $type command] == 0} {
		$path add command -command "recordAndExecMaybe \{$commandOrItemValuePairs\}" -label $name
		return
	}
	if {[string compare $type checkbutton] == 0} {
		$path add checkbutton -command "recordMaybe \{if \{\$$variable == $offvalue\} \{set $variable $onvalue\} else \{set $variable $offvalue\}\}; recordAndExecMaybe \{$commandOrItemValuePairs\}" \
			-label $name -onvalue $onvalue -offvalue $offvalue -variable $variable
		global $variable ;#Tcl doesn't seem to get it right all the time without this
		return
	}

	menu $path.$variable -tearoff false
	$path add cascade -label $name -menu $path.$variable
	foreach {item value} $commandOrItemValuePairs {
		$path.$variable add radiobutton -label $item -value $value -variable $variable -command "recordMaybe \{set $variable $value\}"
		global $variable ;#Tcl doesn't seem to get it right all the time without this
	}
}

proc removeOption {nameOrIndex} {
	.menubar.options delete $nameOrIndex
}

#----tools menu----
#menu .menubar.tools -tearoff false
#.menubar add cascade -label Tools -menu .menubar.tools
proc addTool {{name {}} {hotKey {}}} {
#	global mainWindow
#
#	if {[string length "$name"] <= 0} {
#		.menubar.tools add separator
#	} else {
#		.menubar.tools add command -label $name -accelerator $hotKey\
#				-command "after 50 \"$mainWindow keyboard -1 -1 0 $hotKey 0 $hotKey\""
#	}
}

#----commands menu----
menu .menubar.commands -tearoff false
.menubar add cascade -label Commands -menu .menubar.commands
.menubar.commands add command -label "Return to Home Position" -accelerator h -command "$mainWindow keyboard -1 -1 0 h 0 h"
.menubar.commands add command -label "Fuse Gridded Data" -accelerator f -command "$mainWindow keyboard -1 -1 0 f 0 f"
.menubar.commands add command -label "Hide/Show Rotation Widgets" -accelerator w -command "$mainWindow keyboard -1 -1 0 w 0 w"
.menubar.commands add separator
.menubar.commands add command -label "Run Script" -command {
	set scriptFile [tk_getOpenFile -initialdir "$appdir/scripts" -filetypes {{{Script Files} {.tcl}} {{All Files} {.*}}}]
	if {$scriptFile != ""} {
		set oldDir [pwd]
		cd [file dirname $scriptFile]
		source $scriptFile
		cd $oldDir
	}
}

# mdp - Dec 5, 2006  Added from GeoZui.tcl for movie creation...
# rja - Oct 20, 2005     GZConsole Launch button
# I promoted the GZConsole launch menu to acting like a button on the 
# menu bar since I seem to use it a lot. Hopfully more will use it as 
# the interface to adhoc scripting as it gain features
#
# TODO:
# - See if an instance of GZConsole is already running, make it visible
#   if so istead of launchin another one... Gotta think about this one,
#   a user might want multiple GZConsoles...

if {$tcl_platform(platform) == "windows"} {
#	if {![info exists python]} {
#		set python "python.exe"
#	}
#	if {[file isfile "$python.exe"]} {
		.menubar add command -label "GZConsole" -command {exec python.exe [file join $appdir gzmod/console.py] localhost:$ioPort &}
#	} else {
#		.menubar add command -label "GZConsole" -state disabled
#		puts "Python not found; GZConsole disabled. Check the python setting in GeoZui4D.conf"
#	}
} else {
	.menubar add command -label "GZConsole" -command {exec [file join $appdir gzmod/console.py] localhost:$ioPort &}

	.menubar add command -label "Mission Planner" -command {exec [file join $appdir gzmod/missionPlanner.py] localhost:$ioPort &}
}

#----help menu----
menu .menubar.help -tearoff false
.menubar add cascade -label Help -menu .menubar.help
.menubar.help add command -label About -command {wm deiconify .splash}

trace variable oneEyedFlag w oneEyedCursor
addOption "One-eyed Cursor" checkbutton {} oneEyedFlag
addOption "Right-Eye Dominant" checkbutton {} dominantEye


#---------------------------
#--------sash layout--------
#NOTE: Pane layout must happen before anything that goes in the panes!

paneLayout toplevelSash h {mainSash v 1 {bottomSash h 2 3}} {sideSash v 4 5 6 7}
paneUsePolicy toplevelSash percent 82
paneUsePolicy mainSash percent 22
paneUsePolicy bottomSash percent 100; #formula {$h + 120}
paneUsePolicy sideSash formula 130 formula {$h * 0.5 - 130} formula {$h * 0.75 - 130}

#----------------------------
#--------legacy lists--------

#--------texture list--------
proc comment {} {
frame .textureframe -relief groove -borderwidth 2
set textureFrame .textureframe

label $textureFrame.label -text "Textures" -relief sunken -background "#DDDDDD" -borderwidth 2
pack $textureFrame.label -side top -fill x -padx 2 -pady 2

frame $textureFrame.listframe
set textureListFrame $textureFrame.listframe
listbox $textureListFrame.texturelist -height 3 -selectmode browse -yscrollcommand "$textureListFrame.scroll set" -exportselection no
scrollbar $textureListFrame.scroll -command "$textureListFrame.texturelist yview"
pack $textureListFrame -padx 2 -fill both -expand yes
pack $textureListFrame.texturelist -side left -expand y -fill both
pack $textureListFrame.scroll -side right -fill y
bind $textureListFrame.texturelist <Double-Button-1> {setdefaulttexture $clutEditor}
bind $textureListFrame.texturelist <Return> {setdefaulttexture $clutEditor}

set textureList $textureListFrame.texturelist
set defaultTextureLabel $textureFrame.defaulttexturelabel

label $textureFrame.defaulttexturelabel -text "Default:" -anchor w -justify left
pack $textureFrame.defaulttexturelabel -side top -fill x
}

#----------------------------------------------------------
#--------main display panel (w/ overview 3D window)--------
global mainPanel
set mWidth 250
set mHeight 200
set mainPanel [frame .mainPanel -width $mWidth -height $mHeight]

#--------Layer & scale window for main display--------
global mainScaleLog mainScale mainPanel
set mainScale 5000
set mainScaleLog [expr log10($mainScale)]


set layerFrame [frame $mainPanel.layerWindow]
#					   radar		"Radar"
#					   weather		"Weather"
#					   wind			"Wind"
#					   currents		"Currents"			0
foreach {button name state} {northup		"North-up"			1
					   oblique		"Oblique View"		0
					   seastate		"Sea State"			0
					   shading		"Shading"			1
					   tides		"Apply Tides"		0
					   nav			"Navigation Aids"	1
					   bathymetry	"Bathymetry"		1
					   land			"Land"		        1
					   panorama		"Panoram"			1
					   soundings	"Soundings"			0
					   ais			"AIS"		    	0
					  } {
	set displayMain_$button $state
	checkbutton $layerFrame.$button -text $name -indicatoron false -variable displayMain_$button -anchor w -command "gnButton $button"
	pack $layerFrame.$button -side top -fill x
}

set textureFrame [frame $mainPanel.textureFrame]
label $textureFrame.color -text "Color:"
global textureList
set textureList [ComboBox $textureFrame.texturemap -width 5 -modifycmd {setdefaulttexture $clutEditor}]
pack $textureFrame.color -side left
pack $textureFrame.texturemap -side left -expand true -fill x

#-----------------------------------------------------------------------------
#Procedure to change which texture is the default
#-----------------------------------------------------------------------------
proc setdefaulttexture {win} {
	global textureList
# defaultTextureLabel

	scan [$textureList get] {%[0-9]} textureid
	$win defaulttexture $textureid
#	$defaultTextureLabel configure -text "Default: [$textureList get [$textureList curselection]]"
#	puts "yup: [$textureList get [$textureList curselection]]"
	redisplay
}

set scaleWindow [frame $mainPanel.scaleWindow -relief raised -borderwidth 2]
set scaleFrame [frame $scaleWindow.scaleFrame]
label $scaleFrame.label -text "Scale 1:"
entry $scaleFrame.entry -textvariable mainScale -width 9
bind $scaleFrame.entry <Return> {mainScaleSelect $mainScale}
pack $scaleFrame.label -side left
pack $scaleFrame.entry -side left -fill x
pack $scaleFrame -side top -fill x

scale $scaleWindow.scale -orient vertical -length 100 -from 7 -to 2 -resolution 0.01 \
	-command "mainScaleMoved" -tickinterval 0 -variable mainScaleLog -showvalue false -width 10
pack $scaleWindow.scale -side left -fill both
gz posable create .Posable.Window.main target object
gz animator create .Posable.Window.main targetAnimator ImitatingAnimator .Posable.Window.main .Posable.Window.main.target
gz tellObject .Posable.Window.main.targetAnimator : effect s & speed 5

proc mainScaleMoved {newVal} {
	global mainScale

	set mainScale [expr int(pow(10, $newVal))]
	gz posable set .Posable.Window.main.target s [expr 1.0 / $mainScale]
}

set buttonFrame [frame $scaleWindow.buttonFrame]
#---order matters:  what appears last shows up in front, they always show up in same x,y though---
foreach {value text} {100 "100"
					  200000 "200,000"
					  2000000 "2,000,000"
					  10000 "10,000"
					  5000 "5,000"
					  40000 "40,000"
					  20000 "20,000"
					  80000 "80,000"
					  1000 "1,000"
					  10000000 "10,000,000"} {
	button $buttonFrame.v$value -text "1:$text" -command "mainScaleSelect $value" -width 11 -justify right -anchor e
	place $buttonFrame.v$value -x 1 -rely [expr (7.0 - log10($value)) / 5.3]
}

proc mainScaleSelect {newVal} {
	global mainScaleLog mainScale mainPanel

	regsub -all {[^0-9\.]} $newVal "" newVal
	set mainScale $newVal
	set mainScaleLog [expr log10($newVal)]
	gz posable set .Posable.Window.main.target s [expr 1.0 / $mainScale]
}

#--------togl widget for main display--------
if {[catch "togl $mainWindow -stereo $useStereo -depth true -double true -ident main"]} {
	puts "Disabling stereo"
	togl $mainWindow -depth true -double true -ident main
}

#---Packings---
pack $mainWindow -side right -expand true -fill both
$mainWindow perspective 0
pack $mainPanel.layerWindow -side top -expand false -fill x
pack $textureFrame -side top -expand false -fill x
pack $buttonFrame -side right -expand true -fill both
pack $scaleWindow -side left -expand false -fill y

#---------------------------------------
#----------Vital status window----------
set vitalsWindow [frame .vitals -relief groove -borderwidth 2]
foreach {name labelval default} {latitude "Latitude" " 00\xb0\x30\x30'00.00\"N"
								 longitude "Longitude" " 00\xb0\x30\x30'00.00\"E"
								 sog "Speed Over Ground" " 0.00 Kts"
								 cog "Course Over Ground" " 000.0\xb0T"
								 heading "Heading" " 000.0\xb0T"
								} {
	frame $vitalsWindow.$name
	label $vitalsWindow.$name.label -anchor w -justify left -text $labelval -font $defaultFont
	label $vitalsWindow.$name.value -anchor e -justify right -text $default -font "$defaultFont bold"
	pack $vitalsWindow.$name.value -side right -expand false -fill x
	pack $vitalsWindow.$name.label -side right -expand true -fill x
	pack $vitalsWindow.$name -side top -expand true -fill x
}

#-------------------------------------------------
#----------Dynamic info bar (mouse-over)----------
set dynamicInfoBar [frame .dynamic -relief sunken -borderwidth 4]
label $dynamicInfoBar.label -anchor w -justify left -text "Cursor Info: "
pack $dynamicInfoBar.label -side left -expand false
foreach {name labelval default} {latitude " Lat" " 00\xb0\x30\x30'00.00\"N"
								 longitude " Lon" " 00\xb0\x30\x30'00.00\"E"
								 depth " Depth" " 0000.0m"
								 distance " Distance" " 000.0nm"
								 bearing " Bearing" " 000\xb0"
								 rBearing " Rel. Bearing" " 000\xb0"
								 etr " ETR" " 0:00:00"
								} {
	frame $dynamicInfoBar.$name
	label $dynamicInfoBar.$name.label -anchor e -justify left -text $labelval -font $defaultFont
	label $dynamicInfoBar.$name.value -anchor w -justify left -text $default -font "$defaultFont bold"
	pack $dynamicInfoBar.$name.label -side left -expand false
	pack $dynamicInfoBar.$name.value -side left -expand true
	pack $dynamicInfoBar.$name -side left -expand true -fill x
}
pack $dynamicInfoBar -side top -expand false -fill x

#------------------------------------------------------
# This section ties in info bar items with cursor movement
#------------------------------------------------------

# - bs 3/29/05
#proc updateLatLon {latD latM latS latDir lonD lonM lonS lonDir} {
proc updateLatLon {latD latStr lonD lonStr} {

	#set newLat [format "%02d%02d'%05.2f\"%c" $latD $latM $latS $latDir]
	#set newLon [format "%03d%02d'%05.2f\"%c" $lonD $lonM $lonS $lonDir]
	set newLat [format "%02d\xb0%s" $latD $latStr]
	set newLon [format "%03d\xb0%s" $lonD $lonStr]

	.dynamic.latitude.value configure -text $newLat
	.dynamic.longitude.value configure -text $newLon
}

#proc updateDynamicInfoBar {lat lon depth distance bearing rBearing etr} {
#	.dynamic.latitude.value configure -text $lat
#	.dynamic.longitude.value configure -text $lon
#	.dynamic.depth.value configure -text $depth
#	.dynamic.distance.value configure -text $distance
#	.dynamic.bearing.value configure -text $bearing
#	.dynamic.rBearing.value configure -text $rBearing
#	.dynamic.etr.value configure -text $etr
#} 

# - bs 4/21/05
proc updateDepth {depth} {
	.dynamic.depth.value configure -text $depth
}

# - bs 4/20/05
proc updateDistance {distance} {
	.dynamic.distance.value configure -text $distance
}

# - bs 4/21/05
proc updateBearing {bearing} {
	.dynamic.bearing.value configure -text "$bearing\xb0"
}

# - bs 4/21/05
proc updaterBearing {rBearing} {
	.dynamic.rBearing.value configure -text "$rBearing\xb0"
}

# - bs 4/21/05
proc updateETR {etr} {
	.dynamic.etr.value configure -text $etr
}


#--------------------------------------
#----------Various 3D windows----------
#set overviewWindow [paneToglWindow overview main overview]
#gz posable set Window.mainOverview s [expr [gz posable get Window.main s] * 0.15]
#$overviewWindow lockTo Window.main s
#$overviewWindow perspective 0
#set bWidth [expr int($screenWidth_pix * 0.2 + 42.2)]
set bWidth 300
set bridgeWindow [paneToglWindow bridgeview main normal -width $bWidth]
set bridgewinHeight 0.6
$toglPath(bridgeview) screenSpecs $screenWidth_pix [expr $screenHeight_pix * $bridgewinHeight] $screenLeft $screenRight $screenTop [expr $screenBottom * ($bridgewinHeight - 0.5)]
global windowStackLevel
set windowStackLevel(bridgeview) 0
#$bridgeWindow perspectiveWidthRatio 2
#$bridgeWindow perspectiveHeightRatio 2
$bridgeWindow inParent proxy show 3

gz animator create .Posable.Window.bridgeview rotationAnimator ImitatingAnimator .Posable.Window.bridgeview
gz posable create .Posable.Window.bridgeview offset object
gz tellObject .Posable.Window.bridgeview.rotationAnimator : effect h & speed 50 & offset .Posable.Window.bridgeview.offset

#----------------------------------------
#----------Create GPS posables-----------
global gnVessel
set gnVessel {}

#----------------------------------------
proc gnAttachToVessel {vessel} {
#	global mainWindow sideviewWindow rearviewWindow bridgeWindow gnVessel
	global mainWindow bridgeWindow gnVessel

	set gps $gnVessel.GPS
	set stern $gnVessel.Stern
	set bridge $gnVessel.Bridge

#	catch "gz posable decouple $sideviewWindow and $stern"
#	catch "gz posable decouple $rearviewWindow and $stern"
	catch "gz posable decouple $bridgeWindow and $bridge"
	catch "gz event handler delete vesselMonitor"

	set gnVessel [gz tellObject $vessel getName]

	set gps $vessel.GPS
	set stern $vessel.Stern
	set bridge $vessel.Bridge

	foreach var {h p r x y z} val [gz posable get $gps hprxyz] {
		set $var $val
	}
	gz posable set $gps hprxyz 0 0 0 0 0 0

	gz posable set Window.main h 0
	$mainWindow lockTo $vessel dlt

	gz posable set Window.bridgeview ps 4.3 0.05
	gz tellObject .Posable.Window.bridgeview.rotationAnimator base $bridge
	$bridgeWindow lockTo $bridge dT

	gz posable set $gps hprxyz $h $p $r $x $y $z

	gz event handler create vesselMonitor for objectPoseChanged "if {\"%V\" == \"$gnVessel\"} gnUpdateVesselGPS"
	gz tellObject .EventMediator.user push vesselMonitor
}

#----------------------------------------
proc createSideview {pane} {
	global sideviewWindow gnVessel

	set gps $gnVessel.GPS
	set stern $gnVessel.Stern

	foreach var {h p r x y z} val [gz posable get $gps hprxyz] {
		set $var $val
	}
	gz posable set $gps hprxyz 0 0 0 0 0 0

	set sideviewWindow [paneToglWindow sideview main profile]
	gz event sink create sink_sideviewWindow for mouse2Down
	gz tellObject .EventMediator.Window.sideview push sink_sideviewWindow

	gz posable set Window.sideview hsp 90 0.001 0
	gz posable set Window.sideview yz 100 -10
	$sideviewWindow lockTo $stern lth

	gz posable set $gps hprxyz $h $p $r $x $y $z
	panePack $sideviewWindow $pane
}

#----------------------------------------
proc createRearview {pane} {
	global rearviewWindow gnVessel

	set gps $gnVessel.GPS
	set stern $gnVessel.Stern

	foreach var {h p r x y z} val [gz posable get $gps hprxyz] {
		set $var $val
	}
	gz posable set $gps hprxyz 0 0 0 0 0 0

	set rearviewWindow [paneToglWindow rearview main cross-track]
	gz event sink create sink_rearviewWindow for mouse2Down
	gz tellObject .EventMediator.Window.rearview push sink_rearviewWindow

	gz posable set Window.rearview sp 0.002 0
	gz posable offset Window.rearview yz 10 -10
	$rearviewWindow lockTo $stern ltH

	gz posable set $gps hprxyz $h $p $r $x $y $z
	panePack $rearviewWindow $pane
}

#----------------------------------------
proc gnUpdateVesselGPS {} {
	global gnVessel vitalsWindow

	foreach statistic {latitude longitude sog cog heading} {
		set val [gz tellObject $gnVessel getGPSinfo $statistic dms]
		$vitalsWindow.$statistic.value configure -text $val
	}
}

#----------------------------------------
proc gnButton {button} {
	global displayMain_$button
	global layerFrame displayMain_northup

	upvar #0 displayMain_$button state

	if {$button == "shading"} {
		foreach object "[gz address list .Posable.Object.bathymetry_objects] [gz address list .Posable.Object.land_objects]" {
			catch "gz tellObject $object shadeExaggeration [expr $state * 5 + 0.001]"
		}
		return
	}
	if {$button == "tides"} {
		foreach object "[gz address list .Posable.Object.bathymetry_objects] [gz address list .Posable.Object.land_objects]" {
			if {$state == 1} {
				catch "gz tellObject $object tides tide_objects.tides"
			} else {
				catch "gz tellObject $object tides {}"
			}
		}
		return
	}
	if {$button == "seastate"} {
		set button "tide"
	}
	if {$button == "northup"} {
		if {$state == 0} {
			#heads up
			gz posable couple Window.main to Window.bridgeview H
		} else {
			gz posable decouple Window.main and Window.bridgeview H
			gz posable set Window.main h 0
		}
		return
	}
	if {$button == "oblique"} {
		global mainWindow
		global mainScale
		if {$state == 1} {
			#oblique
			gz posable set Window.main p 24.99; #less than 25 so that the buoys don't always show up in front
			$mainWindow perspective 1
			if {$displayMain_northup} {
				$layerFrame.northup invoke
			}
			#mainScaleSelect [expr int($mainScale / 2.2)]
			mainScaleSelect 1000
		} else {
		    #plan view
			gz posable set Window.main p 90
			$mainWindow perspective 0
			mainScaleSelect [expr int($mainScale * 2.2)]
		}
		return
	}
	if {$button == "soundings"} {
		foreach object "[gz address list .Posable.Object.nav_objects]" {
			if {$state == 1} {
				catch "gz tellObject $object.Sounding show"
			} else {
				catch "gz tellObject $object.Sounding hide"
			}
		}
		return
	}
	if {$state == 1} {
		gz tellObject $button\_objects show
	} else {
		gz tellObject $button\_objects hide
	}
}

#----------------------------------------
proc gnNull {args} {
	return 1
}

#----------------------------------------------
# Validates input into the path planning table
# used with the -validate and -vcmd attributes
# added - 2/24/05 briana
#----------------------------------------------
proc validate {c val w r} {
	global navPath
	if {$c==1 || $c==10} {
		## Anything
		set expr {.*$}
	} elseif {$c>1} {
		## Real only
		set expr {^[-+]?[0-9]*\.?[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
    } 

	if {[string index $val end] == "\n"} {
		$w set "$r,$c" [string trimright $val]
		after idle {gz tellObject $navPath refresh}
		$w activate "[expr $r + 1],$c"
		return 0
	}
    if {[regexp $expr $val]} {
		return 1
    } else {
		bell
		return 0
    }
}

#----------------------------------------------
# Sets the tag to be used for the given row
# on initialization of the table.  Added by
# mdp 12/22/2006
#----------------------------------------------
proc tableRowTag {row} {
	if {[expr $row & 1]} {
		return "oddRow"
	} else {
		return "evenRow"
	}
}

#----info frame (Done here to ensure that it always shows)----
frame .infoframe -relief groove -borderwidth 2
set infoFrame .infoframe
frame $infoFrame.spacer
pack $infoFrame.spacer -side top -pady 2
label $infoFrame.label -text "Information" -relief sunken -background "#DDDDDD" -borderwidth 2
pack $infoFrame.label -side top -fill x -pady 2 -padx 2
set objectVFrame $infoFrame.vertical
set objectHFrame $infoFrame.horizontal
set objectInfo $objectVFrame.text
frame $objectVFrame
frame $objectHFrame
scrollbar $objectVFrame.yscroll -command "$objectInfo yview" -width 16
scrollbar $objectHFrame.xscroll -command "$objectInfo xview" -orient horizontal
text $objectVFrame.text -height 4 -width 10 -wrap none \
 	-yscrollcommand "$objectVFrame.yscroll set" \
	-xscrollcommand "$objectHFrame.xscroll set" \
	-font "courier 8"
pack $objectVFrame.text -side left -expand yes -fill both
pack $objectVFrame.yscroll -side top -expand yes -fill y
pack $objectVFrame -side top -expand yes -fill both -padx 3

pack $objectHFrame.xscroll -side left -expand yes -fill x
label $objectHFrame.filler -width 2
pack $objectHFrame.filler -side left
pack $objectHFrame -side top -fill x -padx 2

#--------Path Planning window--------
# this is for easy changing of table values - added 2/24/05 briana
array set tableAttrib {
	rows 1
	cols 11
	ptable .table
}

# somehow need to configure this array before it is used to store values from the table
# not sure if this is the best way - added 2/24/05 briana
global ptable
for {set i 0} {$i<$tableAttrib(rows)} {incr i} {
    for {set j 0} {$j<$tableAttrib(cols)} {incr j} {
	set ptable($i,$j) ""
    }
}

set pathPlanWindow [frame .pathPlanning -relief groove -borderwidth 2]
global navPath
global pathTable
set pathTable [table $pathPlanWindow.table -cache 1 -titlecols 1 -titlerows 1 \
			   -multiline 0 -cols $tableAttrib(cols) -rows $tableAttrib(rows) -colstretchmode last \
			   -yscrollcommand {.pathPlanning.sy set} \
			   -xscrollcommand {.pathPlanning.sx set} \
			   -variable ptable \
			   -sparsearray no \
			   -rowtagcommand tableRowTag \
			   -validate yes \
			   -vcmd {if {![%W tag includes title %C]} { validate %c %S %W %r} } \
			   -selecttype row -font {{Times Roman} 11 bold} \
			  ]
set pathYscroll [scrollbar .pathPlanning.sy -command "$pathTable yview"]
set pathXscroll [scrollbar .pathPlanning.sx -command "$pathTable xview" -orient horizontal]
$pathTable tag config JustLeft -anchor w
$pathTable tag config JustRight -anchor e
$pathTable tag config JustCenter -anchor center
$pathTable tag col JustCenter 1
$pathTable tag col JustLeft 9
$pathTable tag col JustRight 2 3 4 5 6 7 8
$pathTable tag row JustCenter 0
$pathTable tag raise JustCenter
$pathTable width 0 3  1 10  2 6  3 8  4 11  5 8  6 22 7 14 8 16 9 10
$pathTable tag configure evenRow -bg "#aaaaaa"
$pathTable tag configure oddRow -bg "#bbbbbb"
#$pathTable tag configure totalRow -bg "#888888" -fg "#dddddd"
pack $pathYscroll -side right -fill y
pack $pathXscroll -side bottom -fill x
pack $pathTable -side left -fill both -expand yes

#----------------------------------------
proc newNavPath {pathName varName} {
	global pathTable
	global $varName
	global timezone
	global navPath

	set navPath $pathName
	$pathTable configure -variable $varName
	#Replaced DTG with DTNW for now ("go" replaced with "next waypoint")
	$pathTable set row 0,0 {"#" Label SMG DTNW TTG Heading ETA Lat Lon Cost Notes}
	$pathTable set 0,6 "ETA ($timezone)"
	$pathTable set col 1,0 {1 2 3 4 5 6 7 8 9 10 11 12 13 14 15}
	trace variable $varName w "gz tellObject $pathName updateFromTable"; #automatically gets "<varname> <idx> w" appended to it
	paneForget 1
}

#----------------------------------------
#----------put things in places----------
panePack $mainPanel 2; #1
panePack $bridgeWindow 1
#paneForget 1
#panePack $pathPlanWindow 1
panePack $vitalsWindow 4
##panePack $overviewWindow 2; #5
##panePack $sideviewWindow 3
##panePack $rearviewWindow 6
panePack $infoFrame 7

removeOption Subwindow*
removeOption Zooming*
removeOption Widget*
#really need to resize thing to fit whatever screen they have, don't worry about smaller window version for now...

#---Main window setup---
#---this can't be put in visualizer.tcl without getting the mainWindow in front of the splashscreen
wm geometry . 1024x768
wm protocol . WM_DELETE_WINDOW {gz_exit wm}
focus $mainWindow

$mainWindow bgcolor "#6B6B6B"

update
.toplevelSash sash place 0 800 0
.toplevelSash.mainSash sash place 0 0 600

#---GeoZui parameter changes---
#gz gutmBaseColor 0.5 0.5 0.5 1.0

#---Splash Screen---
toplevel .splash
wm title .splash "GeoNav3D"
wm protocol .splash WM_DELETE_WINDOW {wm withdraw .splash}
canvas .splash.canvas -height 300 -width 400
image create photo splashPhoto -file [file join $appdir "GeoNav.gif"] -height 300 -width 400
.splash.canvas create image 200 150 -image splashPhoto
#label .splash.versionLabel -text "GeoNav3D Version $GeoZuiVersion $GeoZuiDescriptor"
label .splash.versionLabel -text "$GeoZuiDescriptor"
label .splash.copyright -text "Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
 Licensed under the Apache License, Version 2.0"
button .splash.okbutton -text "OK" -command "wm withdraw .splash" -takefocus 1
pack .splash.canvas -expand yes -fill both
pack .splash.versionLabel
pack .splash.copyright
pack .splash.okbutton -pady 10
bind .splash <KeyRelease-Return> ".splash.okbutton invoke"
wm deiconify .splash
after 8000 {wm withdraw .splash}
#update ;# uncomment this line to make it so the splash screen works.

#---Allow user interaction---
bind . <KeyPress-q> {gz_exit q}

#gz posable set Window.main ps 25 0.006
gz posable set Window.main ps 90 0.0001
gz posable set Window.bridgeview ps 4.3 0.05

#---Ground variables/procedures from other guis---
global objectTree
set objectTree gnNull

global vesselFile
set vesselFile "./NF.gzx"

