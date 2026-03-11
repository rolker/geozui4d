# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#-------------------------------------
#
# Interface for predictor testing
#
#-------------------------------------
source [file join $appdir "pane_procs.tcl"]
source [file join $appdir "joystickPredictor.tcl"]

#-----variables--------
global gnVessel top count 
global viewOption visiblePredictor shading visibleVessel
global currentStrength currentDirection predictorTime
global currentGZtime
global file

#-----inititialize variables--------
set viewOption "headsup"
set rbutton "headsup"
set visiblePredictor 1
set visibleVessel 1
set shading 1
set currentStrength 0
set currentDirection 0
set predictorTime 10
set file ""

set gnVessel {}
set count 0
set data_dir "C:/GeoZui3D_data/GeoNav3D/predictor_test/data/"
image create photo bitmap_file -file [file join $appdir "bitmaps/file.gif"]

#---------------------------------------------------------------------------
proc PopulateList {dir} {
	set origDir [pwd]

	if [catch {cd $dir} err] {
		puts stderr $err
		return 0
	}
	
	set fileList [glob *.txt]	
	set allFiles [lsort $fileList]
	cd $origDir 
	return $allFiles
}

#---------------------------------------------------------------------------
proc InitListBox {dir listbox} {
	global count 
	set initialList [PopulateList $dir]
	if {$initialList != 0} {
		foreach item $initialList {
			set count [incr count]
			#set name [string trimright $item .txt]
			set name [file rootname $item]
			$listbox insert $count $count -text $name -image bitmap_file
		}
	} else {
		puts "sorry you have a listbox problem"
	}
	return
}

#-------
# GUI
#-------

#from GeoZui.tcl: => set objnbframe [.pane add -weight 0]
#                 => NoteBook $objnbframe.objnb

set predictor [$objnbframe.objnb insert 2 3 -text "Predictor"]


#--------first create the panes-----------
PanedWindow $predictor.panes -side left
pack $predictor.panes -side top -expand yes -fill both -pady 2 -padx 2m 


#----------------------------------------------------------------------------------
#--- top frame, details of predictor, such as time, position, etc.
#----------------------------------------------------------------------------------
#---Save path in variable for ease of use----
set top [$predictor.panes add]

#------------create scrolling user list to pack first in top frame-------------
#---this must be last or the scroll won't appear-------
#-----------user frame-----------
set userFrame $top.userframe
frame $userFrame -relief groove -borderwidth 2

#-----------user label-----------
label $userFrame.label -text "Users" -relief sunken -background "#DDDDDD" -borderwidth 2


#-----------entry field box and label--------
LabelEntry $top.name -label "User Name:" -helptext "You may change the name of the selected user"
bind $top.name.e <Return> {addFile [$top.name.e get]}

#-----------buttons for saving user files--------
set fileButtons [ButtonBox $userFrame.buttons -orient horizontal -spacing 2]
$fileButtons add -text "Start" -command "UserButtons start" -helptext "Starts collecting user input in file specified by user name"
$fileButtons add -text "Stop" -command "UserButtons stop" -helptext "Stops collecting user input in file specified by user name"

#-----------user scrolling list-----------
ScrolledWindow $userFrame.listframe
ListBox $userFrame.listframe.list -selectmode single
DynamicHelp::add $userFrame.listframe.list -text "Click to add to user data or double-click to load a users data"
$userFrame.listframe setwidget $userFrame.listframe.list
bind $userFrame.listframe.list <<ListboxSelect>> {gznotes selectChanged [$Notes::userFrame.listframe.list curselection]}
$userFrame.listframe.list bindText <Double-1> {gznotes selectActivated [$Predictor::userFrame.listframe.list curselection]}
InitListBox $data_dir $userFrame.listframe.list

#-----------pack everything in the top frame--------------------
pack $top.name -side bottom
pack $fileButtons -side bottom
pack $userFrame.label -side top -fill x -padx 2 -pady 2
#pack $userFrame.listframe.list -expand yes -side bottom -fill both
pack $userFrame.listframe -expand yes -fill both -padx 2
pack $userFrame -padx 1 -expand yes -fill both -side bottom


#----------------------------------------------------------------------------------
#--- middle frame, user list view radio buttons and save user button
#----------------------------------------------------------------------------------

#---Save path in variable for ease of use----
set middle [$predictor.panes add]

#--------radio button frame & label------
labelframe $middle.rbuttonframe -text "View Option"
set radiobuttonFrame $middle.rbuttonframe

#--------radio buttons in left top frame------
foreach {radiob name val} {
	headsup "Heads Up" headsup
	northup "North Up" northup
	} {
	 
	radiobutton $radiobuttonFrame.$radiob \
		-command radioButtonBehavior \
		-text $name \
		-value  $val\
		-variable rbutton \
		-relief flat\
		-anchor w \
		-width 12
	pack $radiobuttonFrame.$radiob -side top  
}

#------------create buttons to pack at right of middle frame-------------
#--------button frame & label----------
labelframe $middle.buttonframe -text "Toggle Variables"
set buttonFrame $middle.buttonframe

#--------buttons in middle frame-----------
foreach {button name state} {predictor "Visible Predictor" 0
							 
			} {
			set displayMain_$button $state
			checkbutton $buttonFrame.$button -text $name \
											 -indicatoron false \
											 -variable displayMain_$button \
											 -anchor w \
											 -command "gnButton $button" \
											 -width 15 
			pack $buttonFrame.$button -side top -pady 2 
}

#--------current frame and label--------
labelframe $middle.currentframe -text "Current Info"
set currentFrame $middle.currentframe

#--------entry boxes for current values--------
set strengthFrame [frame $currentFrame.strengthFrame]
label $strengthFrame.label -text "Strength in knots" 
entry $strengthFrame.entry -textvariable currentStrength -width 9
bind $strengthFrame.entry <Return> {updateStrength $currentStrength}

set directionFrame [frame $currentFrame.directionFrame]
label $directionFrame.label -text "Direction         " 
entry $directionFrame.entry -textvariable currentDirection -width 9
bind $directionFrame.entry <Return> {updateDirection $currentDirection}

#--------predictor frame and label--------
labelframe $middle.predictorframe -text "Predictor Info"
set predictorFrame $middle.predictorframe

#--------entry boxes for predictor values--------
set timeFrame [frame $predictorFrame.timeFrame]
label $timeFrame.label -text "Time in sec"
entry $timeFrame.entry -textvariable predictorTime -width 9
bind $timeFrame.entry <Return> {updatePredictorTime $predictorTime}

#--------scale window for main display--------
global mainScaleLog mainScale 
set mainScale 5000
set mainScaleLog [expr log10($mainScale)]

set scaleWindow [frame $middle.scaleWindow -relief raised -borderwidth 2]
set scaleFrame [frame $scaleWindow.scaleFrame]
label $scaleFrame.label -text "Scale 1:"
entry $scaleFrame.entry -textvariable mainScale -width 9
bind $scaleFrame.entry <Return> {mainScaleSelect $mainScale}

scale $scaleWindow.scale -orient vertical \
						 -length 100 -from 7 -to 2 \
						 -resolution 0.01 \
						 -command "mainScaleMoved" \
						 -tickinterval 0 \
						 -variable mainScaleLog \
						 -showvalue false \
						 -width 10

gz posable create .Posable.Window.main target object
gz animator create .Posable.Window.main targetAnimator ImitatingAnimator .Posable.Window.main .Posable.Window.main.target
gz tellObject .Posable.Window.main.targetAnimator : effect s & speed 5

set scalebuttonFrame [frame $scaleWindow.buttonFrame]
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
	button $scalebuttonFrame.v$value -text "1:$text" \
							    -command "mainScaleSelect $value" \
								-width 11 \
								-justify right \
								-anchor e
	place $scalebuttonFrame.v$value -x 1 -rely [expr (7.0 - log10($value)) / 5.3]
}


#-----------pack everything in the top middle frame--------------------
pack $radiobuttonFrame  -fill both -side top 
pack $buttonFrame       -fill both -side top

pack $strengthFrame.label
pack $strengthFrame.entry 
pack $strengthFrame -side top  
pack $directionFrame.label  
pack $directionFrame.entry  
pack $directionFrame -side top
pack $currentFrame -side top -padx 2 -fill x

pack $timeFrame.label -side top
pack $timeFrame.entry -side top
pack $timeFrame -side top
pack $predictorFrame -side top -fill x

pack $scaleFrame.label -side left
pack $scaleFrame.entry -side left -fill x
pack $scaleFrame -side top -fill x -pady 2
pack $scaleWindow.scale -side left -fill both
pack $scalebuttonFrame -side right -expand true -fill both
pack $scaleWindow -side top -expand true -fill y





#--------------#
#--- procs ----#
#--------------#

#---------------------------------------------------------------------------
proc gnAttachToVessel {vessel} {
	global mainWindow gnVessel

	set bridge $gnVessel.Bridge
	set gnVessel [gz tellObject $vessel getName]
	set bridge $vessel.Bridge
	set gps $vessel.GPS

	catch "gz posable decouple $mainWindow and $bridge"
	
	foreach var {h p r x y z} val [gz posable get $bridge hprxyz] {
		set $var $val
	}

	# north up forced because this is how boat loads (default)
	# put window where boat is
	gz posable set Window.main hprxyz $h $p $r $x $y $z


	# forces heads up - locks (d) interp values as double, (l) localized-relative lock (apply scale, orient, trans),
	# (t) trans, (h) heading 
	$mainWindow lockTo $bridge dlt
	gz posable couple Window.main and Object.vessel.Bridge H

	gz posable set Window.main ps 35 0.013
}

#---------------------Sets the users view according to the radio button chosen---------------------
proc radioButtonBehavior {} {
	global rbutton viewOption
	global mainWindow
	global mainScale
	global file
	
	if {$rbutton == "bridge"} {
		set viewOption "bridge"
		gz posable couple Window.main and Object.vessel.Bridge H
		gz posable set Window.main ps 4.3 0.07
		$mainWindow perspective 1
	}

	if {$rbutton == "northup"} {
		set viewOption "northup"
		gz posable decouple Window.main and Object.vessel.Bridge H
		$mainWindow lockTo Object.vessel.Bridge dlt
		#plan view
		gz posable set Window.main hps 0 90 .0015
		$mainWindow perspective 0
		mainScaleSelect 600
	}
	
	if {$rbutton == "headsup"} {
		set viewOption "headsup"
		gz posable couple Window.main and Object.vessel.Bridge H

		#plan view 
		gz posable set Window.main ps 90 .0015
		$mainWindow perspective 0
		mainScaleSelect 600
	}

	if {$rbutton == "seagull"} {
		set viewOption "seagull"
		#headsup view
		gz posable couple Window.main to Object.vessel.Bridge H
		
		#seagull/oblique
		gz posable set Window.main ps 25 .005
		$mainWindow perspective 1
		mainScaleSelect 400
	}
	UpdateFile $file
	return
}

#---------------------------------------------------------------------------
proc gnButton {button} {
	global displayMain_$button
	global buttonFrame 
	global file visiblePredictor visibleVessel shading
	global predictorTime

	upvar #0 displayMain_$button state

	if {$button == "predictor"} {
		if {$state == 0} {
			gz tellObject vessel lookahead $predictorTime
			set visiblePredictor 1
			$buttonFrame.$button configure -text "Visible Predictor"
			gz tellObject vessel predictor 1
			gz tellObject vessel toggle 1
		} else {
			set visiblePredictor 0
			$buttonFrame.$button configure -text "Hidden Predictor" 
			#gz tellObject vessel predictor 0
			gz tellObject vessel lookahead 0
			#gz tellObject vessel toggle 0
			#set state 0
		}
	}
	if {$button == "shading"} {
		foreach object "[gz address list .Posable.Object.bathymetry_objects] [gz address list .Posable.Object.land_objects]" {
			catch "gz tellObject $object shadeExaggeration [expr $state * 5 + 0.001]"
		}
		if {$state == 0} {
			set shading 0
			$buttonFrame.$button configure -text "Off - Shading"
		} else {
			set shading 1
			$buttonFrame.$button configure -text "On - Shading" 
		}
	}
	if {$button == "toggle"} {
		if {$state == 0} {
			set visibleVessel 1
			$buttonFrame.$button configure -text "Visible Vessel"
			gz tellObject vessel toggle 0
		} else {
			set visibleVessel 0
			$buttonFrame.$button configure -text "Hidden Vessel"
			gz tellObject vessel toggle 1
		}
	}
	UpdateFile $file
	return
}

#---------------------------------------------------------------------------
proc mainScaleSelect {newVal} {
	global mainScaleLog mainScale 

	regsub -all {[^0-9\.]} $newVal "" newVal
	set mainScale $newVal
	set mainScaleLog [expr log10($newVal)]
	gz posable set .Posable.Window.main.target s [expr 1.0 / $mainScale]
}

#---------------------------------------------------------------------------
proc mainScaleMoved {newVal} {
	global mainScale

	set mainScale [expr int(pow(10, $newVal))]
	gz posable set .Posable.Window.main.target s [expr 1.0 / $mainScale]
}

#---------------------------------------------------------------------------
proc updateStrength {newVal} {
	global currentStrength 
	global file

	regsub -all {[^0-9\.]} $newVal "" newVal
	set currentStrength $newVal
	gz tellObject vessel current strength $currentStrength
	UpdateFile $file
}

#---------------------------------------------------------------------------
proc updateDirection {newVal} {
	global currentDirection 
	global file

	regsub -all {[^0-9\.]} $newVal "" newVal
	set currentDirection $newVal
	gz tellObject vessel current direction $currentDirection
	UpdateFile $file
}

#---------------------------------------------------------------------------
proc updatePredictorTime {newVal} {
	global predictorTime
	global file
	
	regsub -all {[^0-9\.]} $newVal "" newVal
	set predictorTime $newVal
	gz tellObject vessel lookahead $predictorTime
	UpdateFile $file
}

#---------------------------------------------------------------------------
proc addFile {name} {
	global data_dir top count 
	global viewOption visiblePredictor shading visibleVessel
	global currentStrength currentDirection predictorTime
	global GZtime GZtimestr
	global file
	set file $name

	set origDir [pwd]
	cd $data_dir

	#--check if name is already in the predictor_test/data/ directory
	if {[file exists $name.txt]} {	
		set choice [tk_messageBox -type yesno \
								  -default no \
								  -message "That name is already being used...would you like to append to this file?"\
								  -icon question]
		if {$choice} {
			#--ok to append to this file
			gz tellObject vessel logfile $name.txt
			gz tellObject vessel dumplog $name.log
			gz tellObject vessel crosstrack path test_path_great_bay_gdl ;#reset it each time
		} else {
			#--otherwise clear entry field
			$top.name.e delete 0 end
		}
				
	} else {
		set count [incr count]
		gz tellObject vessel logfile $name.txt
		gz tellObject vessel dumplog $name.log
		gz tellObject vessel crosstrack path test_path_great_bay_gdl ;# reset it each time
		$top.userframe.listframe.list insert $count $count -text $name -image bitmap_file
		#--clear the entry field
		$top.name.e delete 0 end
		#-- create a companion file to store all variables
		UpdateFile $name
		

		#left.listScroll.list selection set %i
		#details.name.e insert 0 \"%s\
	}
	
	cd $origDir
	return
}

#---------------------------------------------------------------------------
# use when a user variable has changed - in conjunction with trace command
proc UpdateFile {file} {
	global data_dir
	global viewOption visiblePredictor shading visibleVessel
	global currentStrength currentDirection predictorTime
	global currentGZtime

	set origDir [pwd]
	cd $data_dir

	if [catch {open $file.vars "a"} fileId] {
			puts stderr "Cannot open $file.vars: $fileId in UpdateFile"
		} else {
			#companion file name will be --> $file.vars
			#add the values of all variables here.
			puts -nonewline $fileId "$currentGZtime "
			puts -nonewline $fileId "$viewOption "
			puts -nonewline $fileId "$visiblePredictor " 
			puts -nonewline $fileId "$visibleVessel "  
			puts -nonewline $fileId "$currentStrength "
			puts -nonewline $fileId "$currentDirection "
			puts $fileId "$predictorTime "
			close $fileId
		}

	cd $origDir
	return
}

#---------------------------------------------------------------------------
proc UserButtons {button} {
	global top count data_dir
	if {$button == "start"} {
		#start collecting data using Matt's command
		set name [$top.name.e get]
		
		return
	}

	if {$button == "stop"} {
		#stop collecting data using Matt's command		
		return
	}

	if {$button == "rename"} {
		#rename file using Matt's command
		return
	}
}


