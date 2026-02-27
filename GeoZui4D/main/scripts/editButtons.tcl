toplevel .editButtons
wm title .editButtons "GeoZui3D Editing Buttons"

#--------Frame with heading--------
frame .editButtons.editframe -relief groove -borderwidth 2
set editFrame .editButtons.editframe

label $editFrame.label -text "Editing Buttons" -relief sunken -background "#DDDDDD" -borderwidth 2
pack $editFrame.label -side top -fill x -padx 2 -pady 2

#----Buttons---
frame $editFrame.editButtons
set buttonFrame $editFrame.editButtons
button $buttonFrame.newImage -text "New Image" -command {createImage} -pady 0
button $buttonFrame.newBoundary -text "New Boundary" -command {createBoundary} -pady 0
button $buttonFrame.newGdl -text "New Data Lines" -command {createDataLines} -pady 0

pack $buttonFrame.newImage -side left -padx 2
pack $buttonFrame.newBoundary -side left -padx 2
pack $buttonFrame.newGdl -side left -padx 2
pack $buttonFrame -padx 2 -pady 2 -fill x

pack $editFrame -padx 2 -pady 2 -fill both

#----------------------Procedures----------------------
proc createImage {} {
	global texturetypes datadir

	#---Prompt for image (texture) file---
	set imageFile [tk_getOpenFile -initialdir $datadir -filetypes $texturetypes -title "Create New Image Using..."]
	if {$imageFile == {}} return

	#---Prompt for name of new (.screen) file---
	set guessedDir [file dirname $imageFile]
	set guessedFn "[file tail [file rootname $imageFile]].screen"
	set saveobjtype [list [list .screen .screen]]
	set fn [tk_getSaveFile -initialdir $guessedDir -initialfile $guessedFn -defaultextension .screen -filetypes $saveobjtype -title "Save New Image As..."]
	if {$fn == {}} return

	#---Fix up the imageFile name to be localized to the written file---
	set imageDirs [file split [file dirname $imageFile]]
	set fileDirs [file split [file dirname $fn]]
	set upPath ""
	set downPath ""
	set match 1
	foreach ipart $imageDirs fpart $fileDirs {
		if {$ipart != $fpart} {
			puts "Differs at $ipart vs. $fpart"
			set match 0
		}
		if {$match == 0} {
			if {$fpart != ""} {
				append upPath "../"
			}
			if {$ipart != ""} {
				set downPath [file join $downPath $ipart]
			}
		}
	}
	set imagePath [file join $upPath $downPath]
	set imageFile [file join $imagePath [file tail $imageFile]]

	#---Create the file---
	set outfile [open "$fn" "w"]
	foreach {x y z} [gz posable get Window.main xyz] {}
	puts $outfile "BEGIN SCREEN 1.0
SCREEN_WIDTH 100
%SCREEN_HEIGHT 100
%YAW 0
%PITCH 0
%ROLL 0
LOAD
LOCAL_ORIGIN $x $y $z
$imageFile
"
	close $outfile

	#---Open it---
	gz loadobject $fn
}

proc createBoundary {} {
	global datadir

	#---Prompt for name of new (.screen) file---
	set saveobjtype [list [list .boundary .boundary]]
	set fn [tk_getSaveFile -initialdir $datadir -defaultextension .boundary -filetypes $saveobjtype -title "Save New Boundary As..."]
	if {$fn == {}} return

	#---Create the file---
	set outfile [open "$fn" "w"]
	foreach {x y z} [gz posable get Window.main xyz] {}
	puts $outfile "BEGIN BOUNDARY 1.0
N_RECORDS 0
SCALE [expr 500 / [gz posable get Window.main s]]
HEIGHT_RATIO 1
YAW [gz posable get Window.main h]
PITCH [gz posable get Window.main p]
ROLL [gz posable get Window.main r]
LOCAL_ORIGIN $x $y $z
"
	close $outfile

	#---Open it---
	gz loadobject $fn
}

proc createDataLines {} {
	global datadir

	#---Prompt for name of new (.gdl) file---
	set saveobjtype [list [list .gdl .gdl]]
	set fn [tk_getSaveFile -initialdir $datadir -defaultextension .gdl -filetypes $saveobjtype -title "Save New Data Lines As..."]
	if {$fn == {}} return

	#---Create the file---
	set outfile [open "$fn" "w"]
	foreach {x y z s} [gz posable get Window.main xyzs] {}
	set offset [expr 0.02 / $s]
	puts $outfile "BEGIN DATA_LINES 1.0
N_RECORDS 1
N_COLUMNS 3
XYZ
COLOR ATTRIBUTE 3 AS_HEIGHT
LINE ON
TUBE ON
SIZE CONSTANT 2
TEXT NONE
DEGRADATION FACETS
LOCAL_ORIGIN $x $y $z
NEW 2
0.0 0.0 0.0
$offset $offset 0
"
	close $outfile

	#---Open it---
	gz loadobject $fn
}