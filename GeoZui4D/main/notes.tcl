#/* Copyright (c) 2004 Data Visualization Research Lab,
#//                    Center for Coastal and Ocean Mapping,
#//                    University of New Hampshire.  All rights reserved.
#//                    http://www.ccom.unh.edu/vislab
#// Not to be copied or distributed without written agreement.
#// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
#*/

#-------------------------------------
#
# Interface to tempo-spatial notes
#
#-------------------------------------

namespace eval Notes {

image create photo bitmap_notes_red_ball -file [file join $appdir "bitmaps/notes_red_ball.gif"]

#-------
# GUI
#-------

set notes [$objnbframe.objnb insert 1 1 -text "Notes"]


PanedWindow $notes.panes -side left
pack $notes.panes -expand true -fill both -side top

#--- left frame, list of notes and add button

set left [$notes.panes add]

frame $left.buttons

frame $left.buttons.leftSpacer
pack $left.buttons.leftSpacer -fill x -expand true -side left

frame $left.buttons.rightSpacer
pack $left.buttons.rightSpacer -fill x -expand true -side right


set noteButtons [ButtonBox $left.buttons.note -orient vertical -spacing 2]
pack $noteButtons -fill x -side left;

$noteButtons add -text "Add" -command "gznotes add" -helptext "Add a note at the current time, space and viewpoint"
$noteButtons add -text "Delete" -command "gznotes delete" -helptext "Delete currently selected note"
$noteButtons add -text "Adjust" -command "gznotes adjust" -helptext "Sets currently selected note to current time, space and viewpoint"

set fileButtons [ButtonBox $left.buttons.file -orient vertical -spacing 2]
pack $fileButtons -fill x -side left;

$fileButtons add -text "Open" -command "Notes::opennotes" -helptext "Open notes from a .gznotes file"
$fileButtons add -text "Save" -command "gznotes save" -helptext "Save notes in .gznotes format"
$fileButtons add -text "Save as" -command "Notes::savenotesas" -helptext "Save notes in a different .gznotes file, or export as .txt or .cvs ascii flat files"


pack $left.buttons -side bottom -fill x

ScrolledWindow $left.listScroll
ListBox $left.listScroll.list -selectmode single
DynamicHelp::add $left.listScroll.list -text "Click to select a note for editing, or double-click to view a note"
$left.listScroll setwidget $left.listScroll.list
bind $left.listScroll.list <<ListboxSelect>> {gznotes selectChanged [$Notes::left.listScroll.list curselection]}
$left.listScroll.list bindText <Double-1> {gznotes selectActivated [$Notes::left.listScroll.list curselection]}

pack $left.listScroll -expand true -fill both -side top

#--- right frame, details of notes, such as time, position, etc.

set details [$notes.panes add]

LabelEntry $details.name -label "Name:" -helptext "You may change the label of the selected note"
bind $details.name.e <KeyRelease>  {gznotes changeLabel [$Notes::details.name.e get]}
pack $details.name -fill x

LabelEntry $details.time -label "Time:" -editable False -helptext "Time, in seconds since Jan 1st, 1970 00:00 GMT (Unix standard)"
pack $details.time -fill x

LabelEntry $details.position -label "Position:" -editable False -helptext "Position of the center of workspace, in meters, in the current UTM zone"
pack $details.position -fill x

TitleFrame $details.codes -text "Codes:"
pack $details.codes -fill x

set codes [$details.codes getframe]

Entry $codes.one -helptext "User specific code #1, may be used to classify notes as it will show up in its own field in the output file"
pack $codes.one -fill x -expand true
bind $codes.one <KeyRelease> {gznotes changeCode 1 [$Notes::codes.one get]}

Entry $codes.two -helptext "User specific code #2, may be used to classify notes as it will show up in its own field in the output file"
pack $codes.two -fill x  -expand true
bind $codes.two <KeyRelease> {gznotes changeCode 2 [$Notes::codes.two get]}

Entry $codes.three -helptext "User specific code #3, may be used to classify notes as it will show up in its own field in the output file"
pack $codes.three -fill x  -expand true
bind $codes.three <KeyRelease> {gznotes changeCode 3 [$Notes::codes.three get]}

ScrolledWindow $details.textScroll
text $details.textScroll.text -height 10;
DynamicHelp::add  $details.textScroll.text -text "General description, observations, etc. about current note"
bind $details.textScroll.text <KeyRelease>  {gznotes changeDescription [$Notes::details.textScroll.text get 1.0 end]}
$details.textScroll setwidget $details.textScroll.text
pack $details.textScroll -expand true -fill both


#--------------#
#--- procs ----#
#--------------#

#--- togle notes - shows and hides the notes window

namespace export togleNotes;

proc togleNotes {} {
	if { [wm state $notes] == "withdrawn"} {
		wm state $notes normal
		raise $notes
	} else {
		wm withdraw $notes
	}
}

namespace export savenotesas

proc savenotesas {} {
	global datadir

	set fn [tk_getSaveFile -initialdir $datadir -defaultextension .gznotes -filetypes { {{GeoZui4D Notes} {.gznotes} } {{Text export} {.txt .csv} }} -title "Save Notes"]
	if {$fn != ""} {
		gznotes save $fn
	}
}

namespace export opennotes

proc opennotes {} {
	global datadir

	set fn [tk_getOpenFile -initialdir $datadir -defaultextension .gznotes -filetypes { {{GeoZui4D Notes} {.gznotes} } } -title "Open Notes"]
	if {$fn != ""} {
		gznotes open $fn
	}

}

} 