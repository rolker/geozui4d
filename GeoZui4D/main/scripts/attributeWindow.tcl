#-----------------------------------------------------------------------------
# attributeWindow procedures
#
# Creates windows that allow an object's properties to be viewed and edited.
#
# Created by mdp 3/6/2003
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Procedure to create a new attribute subwindow
#-----------------------------------------------------------------------------
proc attributeWindow {path object args} {
	global toglPath subWindowCount
	global attributeWindowName attribUpdate
	global attribObject attribFullObject attribCount attribAllocated
	global attribPosableType attribPosableOffset subwindowCleanup
	global attribIsWrapper

	set wrapper 0
	set hideHandler {}
	set showHandler {}

	set parentPath $toglPath(main)
	set name [string range $object [expr 1 + [string last . $object]] end]

	#---Get the switches, if any
	foreach switch {wrapper hideHandler showHandler} {
		set remainder {}
		foreach {s v} $args {
			if {"$s" == "-$switch"} {
				set $switch $v
			} else {
				lappend remainder $s $v
			}
		}
		set args $remainder
	}

	set hideHandler "global attribUpdate;set attribUpdate($name) 0;$hideHandler"

	#---Get the subwindow name out of the path
	set windowRootPath [eval "subwindow $path -name \{$name\} -width 285 -height 285 $args -contents empty -hideHandler \{$hideHandler\} -showHandler \{$showHandler\}"]

	#---Get the subwindow names out of the paths
	set windowPath $toglPath($name)

	#---Attach the window to the passed object---
	set attributeWindowName($object) $name
	set attribObject($name) $object
	set attribFullObject($name) [gz tellObject $object getName]
	set attribCount($name) 0
	set attribAllocated($name) 0
	set attribPosableType($name) C
	set attribPosableOffset($name) 0
	set attribUpdate($name) 0
	set attribIsWrapper($name) $wrapper
	set subwindowCleanup($name) "attributeWindowCleanup $name; $subwindowCleanup($name)"

	#---Create common widgets---
	subwindowTitle $name "Attributes for $name ([gz tellObject $object objectType])"
	set pif [frame $windowPath.posableInfo -relief sunken -borderwidth 2]

	#---Full path info---
	set nameFrame [frame $pif.nameFrame]
	label $nameFrame.label -text "Address:" -justify right -anchor e
	label $nameFrame.target -textvariable attribFullObject($name) -justify left -anchor w  -relief sunken -borderwidth 2
	pack $nameFrame.label -side left
	pack $nameFrame.target -side left -fill x -expand yes
	pack $nameFrame -fill x -padx 2 -expand yes

	#--Posable Info--
	set posFrame [frame $pif.posFrame]
	set rotFrame [frame $pif.rotFrame]
	set etcFrame [frame $pif.etcFrame]
	label $posFrame.xLabel -text "x" -width 1
	label $posFrame.yLabel -text "y" -width 1
	label $posFrame.zLabel -text "z" -width 1
	checkbutton $posFrame.offset -text "Offset" -variable attribPosableOffset($name) -command "attributeUpdatePoseInfo $name"
	label $rotFrame.hLabel -text "h" -width 1
	label $rotFrame.pLabel -text "p" -width 1
	label $rotFrame.rLabel -text "r" -width 1
	#checkbutton $rotFrame.latlon -text "Lat/Lon"
	#pack $rotFrame.latlon -side left
	label $etcFrame.sLabel -text "s" -width 1
	foreach {frame specifics} "$posFrame {xEntry x yEntry y zEntry z} $rotFrame {hEntry h pEntry p rEntry r} $etcFrame {sEntry s}" {
		foreach {entry val} $specifics {
			entry $frame.$entry -width 9
			bind $frame.$entry <Return> "attributeSetPosable $name $frame $entry $val"
			bind $frame.$entry <FocusOut> "attributeSetPosable $name $frame $entry $val"
		}
	}
	pack $posFrame.xLabel $posFrame.xEntry $posFrame.yLabel $posFrame.yEntry $posFrame.zLabel $posFrame.zEntry -side left
	pack $posFrame.offset -side left
	pack $rotFrame.hLabel $rotFrame.hEntry $rotFrame.pLabel $rotFrame.pEntry $rotFrame.rLabel $rotFrame.rEntry -side left
	pack $etcFrame.sLabel $etcFrame.sEntry -side left
	radiobutton $etcFrame.current -text "Cur" -value C -variable attribPosableType($name) -width 2 -justify left -command "attributeUpdatePoseInfo $name"
	radiobutton $etcFrame.home -text "Hom" -value H -variable attribPosableType($name) -width 2 -justify left -command "attributeUpdatePoseInfo $name"
	radiobutton $etcFrame.min -text "min" -value m -variable attribPosableType($name) -width 2 -justify left -command "attributeUpdatePoseInfo $name"
	radiobutton $etcFrame.max -text "Max" -value M -variable attribPosableType($name) -width 2 -justify left -command "attributeUpdatePoseInfo $name"
	pack $etcFrame.current $etcFrame.home $etcFrame.min $etcFrame.max -side left -padx 2
	pack $posFrame -fill x -pady 2
	pack $rotFrame -fill x -pady 2
	pack $etcFrame -fill x -pady 2
	pack $pif -fill x

	#--Proper Attributes--
    set sw [ScrolledWindow $windowPath.sw -relief sunken -borderwidth 2]
    set sf [ScrollableFrame $sw.sf -height 10]
    $sw setwidget $sf
	
	pack $sw -fill both -expand yes

	#--Update paradigm--
	set uf [frame $windowPath.updateFrame]
	checkbutton $uf.continuous -text "Update Continuously" -variable attribUpdate($name) -command "attribUpdateContinuously $name"
	button $uf.updateButton -command "attributeUpdate $name" -text "Refresh"
	pack $uf.continuous -side left
	pack $uf.updateButton -fill x -side left
	pack $uf -fill x

	attributeUpdate $name 1
}

#-----------------------------------------------------------------------------
# Procedure to update a attribute subwindow
#-----------------------------------------------------------------------------
proc attribUpdateContinuously {name} {
	global attribUpdate attribFullObject attribIsWrapper

	if {$attribUpdate($name) == 0} {
		if {$attribIsWrapper($name) != 0} {
			gz event handler delete wposeOf$name
		}
		if {$attribIsWrapper($name) != 1} {
			gz event handler delete poseOf$name
		}
		gz event handler delete attribOf$name
	} else {
		#Delay evaluation of the $attribFullObject array variable
		if {$attribIsWrapper($name) != 0} {
			gz event handler create wposeOf$name for wrapperPoseChanged "global attribFullObject; if {\"%V\" == \"\$attribFullObject($name)\"} {attributeUpdatePoseInfo $name}"
			gz tellObject .EventMediator.user push wposeOf$name
		}
		if {$attribIsWrapper($name) != 1} {
			gz event handler create poseOf$name for objectPoseChanged "global attribFullObject; if {\"%V\" == \"\$attribFullObject($name)\"} {attributeUpdatePoseInfo $name}"
			gz tellObject .EventMediator.user push poseOf$name
		}
		gz event handler create attribOf$name for objectChanged "attributeUpdate $name 0"
		gz tellObject .EventMediator.internal push attribOf$name
	}
}

#-----------------------------------------------------------------------------
# Procedure to update a attribute subwindow
#-----------------------------------------------------------------------------
proc attributeUpdate {name {updatePoseToo 1} {updateLabels 0}} {
	global toglPath
	global attribCount attribAllocated attribFullObject attribIsWrapper

	set windowPath $toglPath($name)
    set subf [$windowPath.sw.sf getframe]
	set rowPath $subf.row
	set object $attribFullObject($name)
	set numAttributes [gz tellObject $object attribute count]
	set repack 0
	#---Create any extra rows needed---
	if {$numAttributes > $attribAllocated($name)} {
		for {set i $attribAllocated($name)} {$i < $numAttributes} {incr i} {
			frame $rowPath$i
			label $rowPath$i.label -text "[gz tellObject $object attribute name $i]:" -width 20 -anchor e -justify right
			entry $rowPath$i.entry -width 21
			pack $rowPath$i.label -side left
			pack $rowPath$i.entry -side left -fill x -expand yes
			pack $rowPath$i -side top -fill x -pady 2 -expand yes
			bind $rowPath$i.entry <Return> "gz tellObject \$attribFullObject($name) attribute value $i \[$rowPath$i.entry get\]"
			bind $rowPath$i.entry <FocusOut> "gz tellObject \$attribFullObject($name) attribute value $i \[$rowPath$i.entry get\]"
		}
		set attribAllocated($name) $numAttributes
	}
	if {$attribCount($name) != $numAttributes} {
		set repack 1
		set updateLabels 1
	}
	set focusWindow [focus -displayof $subf]
	for {set i 0} {$i < $numAttributes} {incr i} {
		if {$repack} {
			pack $rowPath$i -side top -fill x -pady 2 -expand yes
		}
		if {$updateLabels} {
			$rowPath$i.label configure -text "[gz tellObject $object attribute name $i]:"
		}
		if {"$focusWindow" != "$rowPath$i.entry"} {
			$rowPath$i.entry delete 0 end
			$rowPath$i.entry insert end "[gz tellObject $object attribute value $i]"
		}
	}
	if {$repack} {
		for {set i $numAttributes} {$i < $attribAllocated($name)} {incr i} {
			pack forget $rowPath$i
		}
	}
	set attribCount($name) $numAttributes
	if {$updatePoseToo || $attribIsWrapper($name)} {
		attributeUpdatePoseInfo $name
	}
}

#-----------------------------------------------------------------------------
# Procedure to set posable information from a attribute subwindow
#-----------------------------------------------------------------------------
proc attributeSetPosable {name path entry val} {
	global attribPosableOffset attribFullObject

	set value [$path.$entry get]
	
	gz posable set $attribFullObject($name) $val $value
}

#-----------------------------------------------------------------------------
# Procedure to update just the posable portion of a attribute subwindow
#-----------------------------------------------------------------------------
proc attributeUpdatePoseInfo {name} {
	global toglPath
	global attribFullObject attribCount attribAllocated
	global attribPosableType attribPosableOffset

	set windowPath $toglPath($name).posableInfo
	set posFrame $windowPath.posFrame
	set rotFrame $windowPath.rotFrame
	set etcFrame $windowPath.etcFrame
	foreach {x y z h p r s} [gz posable get $attribFullObject($name) $attribPosableType($name)xyzhprs] {}
	$posFrame.xEntry delete 0 end
	$posFrame.xEntry insert end $x
	$posFrame.yEntry delete 0 end
	$posFrame.yEntry insert end $y
	$posFrame.zEntry delete 0 end
	$posFrame.zEntry insert end $z
	$rotFrame.hEntry delete 0 end
	$rotFrame.hEntry insert end $h
	$rotFrame.pEntry delete 0 end
	$rotFrame.pEntry insert end $p
	$rotFrame.rEntry delete 0 end
	$rotFrame.rEntry insert end $r
	$etcFrame.sEntry delete 0 end
	$etcFrame.sEntry insert end $s
}

#-----------------------------------------------------------------------------
# Procedure to manage attribute subwindows
#-----------------------------------------------------------------------------
proc attributePanel {name command} {
	global attribObject

	#---find least-size address string, assuming .Posable.Object---
	set nameIndex [string last "Object." $name]
	if {$nameIndex >= 0} {
		set name [string range $name [expr 7 + $nameIndex] end]
	}
	#---Create an appropriate window name---
	regsub -all "\[.\]" $name "_" windowName

	if {$command == "show"} {
		if {[info exists attribObject($name)]} {
			attributeUpdate $name
			subwindowRestore $name
			subwindowRaise $name
			return $name
		} else {
			attributeWindow .aWin_$windowName $name
			return $name
		}
	} elseif {$command == "hide"} {
		if {[info exists attribObject($name)]} {
			subwindowMinimize $name
			return $name
		} else {
			error "$name is not a valid attribute panel name"
			return
		}
	} elseif {$command == "close"} {
		if {[info exists attribObject($name)]} {
			subwindowDestroy $name
		} else {
			error "$name is not a valid attribute panel name"
			return
		}
	} else {
		error "Invalid attribute panel command.  Must be show, hide, or close."
	}
}

#-----------------------------------------------------------------------------
# Procedure to clean up attribute subwindow variables
#-----------------------------------------------------------------------------
proc attributeWindowCleanup {name} {
	global attributeWindowName
	global attribObject attribFullObject attribCount attribAllocated
	global attribPosableType attribPosableOffset attribUpdate
	global attribIsWrapper

	unset attributeWindowName($name)
	unset attribObject($name)
	unset attribFullObject($name)
	unset attribCount($name)
	unset attribAllocated($name)
	unset attribPosableType($name)
	unset attribPosableOffset($name)
	unset attribIsWrapper($name)
	unset attribUpdate($name)
}
