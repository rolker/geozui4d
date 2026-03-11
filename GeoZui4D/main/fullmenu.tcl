# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#--------menubar--------
menu .menubar
$menuWin config -menu .menubar

#----file menu----
menu .menubar.file -tearoff false
.menubar add cascade -label File -menu .menubar.file

set objectTypeAlias(gutm) "Gridded UTM"
set objectTypeAlias(dem) "USGS DEM"
set objectTypeAlias(strip) "Polygon Strip"
set objectTypeAlias(sdv) "Scene files"
set objectTypeAlias(img) "ROV Images"

set texturetypes {
    {{Textures} {.clut .grid .tfw .jgw .tif .jpg .lat .mgw .nc}}
    {{Color Table} {.clut}}
    {{Grid} {.grid}}
    {{Image} {.tfw .jgw .tif .jpg .nc}}
    {{Lattice} {.lat}}
    {{Movie} {.mgw}}
}
#menu .menubar.file.new -tearoff false
#.menubar.file add cascade -label "New" -menu .menubar.file.new
#.menubar.file.new add command -label "AUV Mission" -command {gz newobject MissionPlan mission posSelfInit}

.menubar.file add command -label "Open Object" -command {loadobj [tk_getOpenFile -initialdir $datadir -multiple 1 -filetypes [getObjectFilterList]]}
if {[file exists [file join $appdir "base2gutm"]]} {
.menubar.file add command -label "Open Base Grid" -command {loadBaseGrid [tk_getOpenFile -initialdir $datadir -multiple 1 -filetypes {{{Base Grids} hns}}]}
}
.menubar.file add command -label "Open Texture" -command {loadtexture [tk_getOpenFile -initialdir $datadir -multiple 1 -filetypes $texturetypes]}
.menubar.file add separator
.menubar.file add command -label "Save Screenshot" -command {$mainWindow screenShot 4000 4000 [tk_getSaveFile -initialdir $datadir]}
.menubar.file add separator
.menubar.file add command -label "Clear Scene" -command {clearScene}
.menubar.file add separator
.menubar.file add command -label Exit -command {gz_exit menu}

#----view menu----
menu .menubar.view -tearoff false
.menubar add cascade -label View -menu .menubar.view
.menubar.view add checkbutton -label Stereo -onvalue 1 -offvalue -1 -variable stereoflag -command {redisplay; recordMaybe "set stereoflag $stereoflag"}
.menubar.view add checkbutton -label "Side by side stereo" -onvalue 1 -offvalue -1 -variable ssstereoflag -command {redisplay; recordMaybe "set ssstereoflag $ssstereoflag"; wm geometry . 2048x1024+0+0; set stereoflag $ssstereoflag ;recordMaybe "set stereoflag $stereoflag"}
.menubar.view add separator
#.menubar.view add command -label "2048x768" -command {wm geometry . 2048x1024+0+0; }
#.menubar.view add command -label Objects -command {recordAndExecMaybe {pack .objframe -side right -fill y -padx 1}}
.menubar.view add command -label Objects -command {recordAndExecMaybe {global pane; $pane.f1 configure -width 300}}
.menubar.view add command -label "Edit-Target Attributes" -command {attributePanel editPoint show}
.menubar.view add command -label Background -command {changebg $mainWindow}

#----options menu----
menu .menubar.options -tearoff false
.menubar add cascade -label Options -menu .menubar.options
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

#----tools menu----
menu .menubar.tools -tearoff false
.menubar add cascade -label Tools -menu .menubar.tools
proc addTool {{name {}} {hotKey {}}} {
	global mainWindow

	if {[string length "$name"] <= 0} {
		.menubar.tools add separator
	} else {
		.menubar.tools add command -label $name -accelerator $hotKey\
				-command "after 50 \"$mainWindow keyboard -1 -1 0 $hotKey 0 $hotKey\""
	}
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

}

if {$tcl_platform(platform) == "windows"} {
#	.menubar entryconfigure last -state disabled
}

#----help menu----
menu .menubar.help -tearoff false
.menubar add cascade -label Help -menu .menubar.help
.menubar.help add command -label About -command {wm deiconify .splash}

trace variable oneEyedFlag w oneEyedCursor
addOption "One-eyed Cursor" checkbutton {} oneEyedFlag
addOption "Right-Eye Dominant" checkbutton {} dominantEye
addOption {}
addOption "Static Subwindow Tethers" checkbutton {global subwindowList defaultTethersAlwaysOn
												  foreach sub $subwindowList {
													subwindowTethersAlwaysOn $sub $defaultTethersAlwaysOn}
												 } defaultTethersAlwaysOn

