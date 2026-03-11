# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#-----------------------------------------------------------------------------
# ClutEditor procedures
#
# Created by mdp 7/18/2000
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
#Procedure to create a new Clut Editor
#-----------------------------------------------------------------------------
proc cluteditor {windowPath args} {
	global clutEditor toglPath

	toplevel $windowPath
    wm protocol $windowPath WM_DELETE_WINDOW "recordAndExecMaybe \"wm withdraw $windowPath\""
    wm title $windowPath "Clut Editor"

	label $windowPath.label
	pack $windowPath.label -side top

#	togl $windowPath.gl -width 200 -height 500 -ident cluted -double true -stereo true
	togl $windowPath.gl -width 200 -height 500 -ident cluted -double true
	set clutEditor $windowPath.gl
	set toglPath(cluted) $clutEditor

	pack $clutEditor -side left

	label $windowPath.elementheight -text "Height:"
	pack $windowPath.elementheight

	frame $windowPath.newheight
	pack $windowPath.newheight 

	label $windowPath.newheight.label -text "New height:"
	pack $windowPath.newheight.label -side left

	entry $windowPath.newheight.text 
	pack $windowPath.newheight.text -side left

	frame $windowPath.buttons
	pack $windowPath.buttons

	button $windowPath.buttons.add -text "Add" -command "addelement $windowPath"
	pack $windowPath.buttons.add -side left

	button $windowPath.buttons.set -text "Set" -command "setelement $windowPath"
	pack $windowPath.buttons.set -side left

	button $windowPath.buttons.delete -text "Delete" -command "deleteelement $windowPath"
	pack $windowPath.buttons.delete -side left


# extents

	label $windowPath.clutmaxcurrent -text "Max:"
	pack $windowPath.clutmaxcurrent

	frame $windowPath.clutmax
	pack $windowPath.clutmax

	label $windowPath.clutmax.newlabel -text "New Max:"
	pack $windowPath.clutmax.newlabel -side left

	entry $windowPath.clutmax.entry 
	pack $windowPath.clutmax.entry -side left
    
	label $windowPath.clutmincurrent -text "Min:"
	pack $windowPath.clutmincurrent

	frame $windowPath.clutmin
	pack $windowPath.clutmin

	label $windowPath.clutmin.newlabel -text "New Min:"
	pack $windowPath.clutmin.newlabel -side left

	entry $windowPath.clutmin.entry 
	pack $windowPath.clutmin.entry -side left

	button $windowPath.modextents -text "Modify Extents" -command "modifyextents $windowPath"
	pack $windowPath.modextents
    
# generate range
	
	frame $windowPath.rangemax
	pack $windowPath.rangemax

	label $windowPath.rangemax.newlabel -text "Range Max:"
	pack $windowPath.rangemax.newlabel -side left

	entry $windowPath.rangemax.entry 
	pack $windowPath.rangemax.entry -side left
    
	frame $windowPath.rangemin
	pack $windowPath.rangemin

	label $windowPath.rangemin.newlabel -text "Range Min:"
	pack $windowPath.rangemin.newlabel -side left

	entry $windowPath.rangemin.entry 
	pack $windowPath.rangemin.entry -side left
    
   	frame $windowPath.stepsize
	pack $windowPath.stepsize

	label $windowPath.stepsize.label -text "Step Size:"
	pack $windowPath.stepsize.label -side left

	entry $windowPath.stepsize.entry 
	pack $windowPath.stepsize.entry -side left

	button $windowPath.createrange -text "Create Range" -command "createrange $windowPath"
	pack $windowPath.createrange
    
# clear range
	
	frame $windowPath.clearRangeMax
	pack $windowPath.clearRangeMax

	label $windowPath.clearRangeMax.newlabel -text "Clear Max:"
	pack $windowPath.clearRangeMax.newlabel -side left

	entry $windowPath.clearRangeMax.entry 
	pack $windowPath.clearRangeMax.entry -side left
    
	frame $windowPath.clearRangeMin
	pack $windowPath.clearRangeMin

	label $windowPath.clearRangeMin.newlabel -text "Clear Min:"
	pack $windowPath.clearRangeMin.newlabel -side left

	entry $windowPath.clearRangeMin.entry 
	pack $windowPath.clearRangeMin.entry -side left

	button $windowPath.clearRange -text "Clear Range" -command "clearrange $windowPath"
	pack $windowPath.clearRange

# clear clut

	frame $windowPath.clearclut
	pack $windowPath.clearclut
	
	label $windowPath.clearclut.label -text "Enable"
	pack $windowPath.clearclut.label -side left

    checkbutton $windowPath.clearclut.safety -command "if {\$safety == 1} then {$windowPath.clearclut.button configure -state active} else {$windowPath.clearclut.button configure -state disabled}"
	pack $windowPath.clearclut.safety -side left
    
	button $windowPath.clearclut.button -text "Clear Clut" -command "$windowPath.clearclut.safety invoke; clearclut $windowPath" -state disabled
	pack $windowPath.clearclut.button -side left

# close clut editor frame

	button $windowPath.exitbutton -text "Close Frame" -command "recordAndExecMaybe \"wm withdraw $windowPath\""
	pack $windowPath.exitbutton -side bottom -padx 10 -pady 5
    
    wm withdraw $windowPath
}

#-----------------------------------------------------------------------------
#Procedure to create a new Color Key
#-----------------------------------------------------------------------------
proc colorkey {windowPath args} {
	global colorKey toglPath

	subwindow .key -parent main -contents colorkey -relief flat -borderwidth 1 -titleheight -1 -width 100 -destroyable 0
	set colorKey $toglPath(key)
	subwindowMinimize key
}

#-----------------------------------------------------------------------------
#Procedure to add a range to a clut
#-----------------------------------------------------------------------------
proc addelement {clutEditor} {
	$clutEditor.gl addclutelement [$clutEditor.newheight.text get]
	$clutEditor.gl render
	redisplay
}

#-----------------------------------------------------------------------------
#Procedure to select a range to a clut
#-----------------------------------------------------------------------------
proc setelement {clutEditor} {
	$clutEditor.gl setclutelement [$clutEditor.newheight.text get]
	$clutEditor.gl render
	redisplay
}

#-----------------------------------------------------------------------------
#Procedure to remove a range from a clut
#-----------------------------------------------------------------------------
proc deleteelement {clutEditor} {
	$clutEditor.gl deleteclutelement
	$clutEditor.gl render
	redisplay
}

#-----------------------------------------------------------------------------
#Procedure to stretch and shift a clut by modifying extents (min and max)
#-----------------------------------------------------------------------------
proc modifyextents {clutEditor} {
	$clutEditor.gl modifyclutextents [$clutEditor.clutmax.entry get] [$clutEditor.clutmin.entry get]
	$clutEditor.gl render
	redisplay
}

#-----------------------------------------------------------------------------
#Procedure to create a range, intorpolating from exsisting colors
#-----------------------------------------------------------------------------
proc createrange {clutEditor} {
	$clutEditor.gl createclutrange [$clutEditor.rangemax.entry get] [$clutEditor.rangemin.entry get] [$clutEditor.stepsize.entry get]
	$clutEditor.gl render
	redisplay
}

#-----------------------------------------------------------------------------
#Procedure to clear a range
#-----------------------------------------------------------------------------
proc clearrange {clutEditor} {
	$clutEditor.gl clearclut [$clutEditor.clearRangeMax.entry get] [$clutEditor.clearRangeMin.entry get]
	$clutEditor.gl render
	redisplay
}


#-----------------------------------------------------------------------------
#Procedure to clear a clut
#-----------------------------------------------------------------------------
proc clearclut {clutEditor} {
	$clutEditor.gl clearclut
	$clutEditor.gl render
	redisplay
}

