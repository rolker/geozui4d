# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#-----------------------------------------------------------------------------
# Subwindow togl procedures
#
# The border implemented here uses the following values for xType and yType:
#		+------+------+------+
#		|-1, -1| 0, -1| 1, -1|
#		+------+------+------+
#		|-1, 0 | 0, 0 | 1, 0 |
#		+------+------+------+
#		|-1, 1 | 0, 1 | 1, 1 |
#		+------+------+------+
# A value of -2 indicates a non-resize operation.
#
# Created by mdp 7/13/2000
#-----------------------------------------------------------------------------

set subWindowCount 0
set xPlacement(main) 0
set yPlacement(main) 0
set xSize(main) 0
set ySize(main) 0
set xInnerSize(main) 0
set yInnerSize(main) 0
set xMouseStart 0
set yMouseStart 0
set xSizeStart 0
set ySizeStart 0
set resizeWidth(main) 0
set borderWidth(main) 0
set titleHeight(main) 0
set parentWindowPath(main) .
set windowConfiguration(main) ""
set windowStackLevel(main) 0
set windowStack(0) .
set subwindowList ""
set positionText(main) ""
set headingText(main) ""
set pitchText(main) ""
set windowContents(main) 3D
set tetherEvent(main) {}
set parentName(main) {}
set subwindowCleanup(main) {}
set hideScript(main) {}
set showScript(main) {}
set tethersAlwaysOn(main) 0
set windowContents(top) empty
set parentName(top) {}
set parentWindowPath(top) .
set toglPath(top) .
set windowStackLevel(top) 0
set raiseScript(main) {}
set raiseScript(top) {}
set isToplevel(main) 0

set defaultTethersAlwaysOn 1
set defaultSubwindowsToplevel 0

set sidelist {top_left_corner     top_side     top_right_corner
              left_side                        right_side
              bottom_left_corner  bottom_side  bottom_right_corner}

#set subLockButtons {set get lock_scale unlock_scale lock_position unlock_position
#					lock_rotate_y unlock_rotate_y}

if {$normal_mode != 0} {
#	set subLockButtons {lock_scale unlock_scale lock_position unlock_position
#						lock_rotate_y unlock_rotate_y lock_rotate_x unlock_rotate_x}
#	set subViewButtons {set_scale set_position set_rotate_y set_rotate_x separator
#						get_scale get_position get_rotate_y get_rotate_x}
	set subLockButtons {lock_position unlock_position lock_rotate_y unlock_rotate_y}
	set subViewButtons {set_position set_rotate_y separator
						get_position get_rotate_y}
} else {
	set subLockButtons {}
	set subViewButtons {}
}

foreach bitmap $subLockButtons {
	if {[string compare $bitmap "separator"] != 0} {
		image create bitmap bitmap_$bitmap -file [file join $appdir "bitmaps/$bitmap.bmp"]
	}
}

foreach bitmap $subViewButtons {
	if {[string compare $bitmap "separator"] != 0} {
		image create bitmap bitmap_$bitmap -file [file join $appdir "bitmaps/$bitmap.bmp"]
	}
}

image create bitmap bitmap_close -file [file join $appdir "bitmaps/close.bmp"]

#-----------------------------------------------------------------------------
proc initSubtogl {} {
	global absoluteWindowPath toglPath mainWindow

	set absoluteWindowPath(main) $mainWindow
	set toglPath(main) $mainWindow
}

#-----------------------------------------------------------------------------
#Procedure to create a new togl subwindow
#-----------------------------------------------------------------------------
proc subtogl {windowPath args} {
	return [eval "subwindow $windowPath $args -contents 3D"]
}

proc subwindow {windowPath args} {
    global subWindowCount resizeWidth borderWidth titleHeight
	global xPlacement yPlacement xSize ySize xInnerSize yInnerSize
	global mainWindow absoluteWindowPath windowConfiguration isToplevel
	global toglPath parentWindowPath windowStack windowStackLevel
	global LABEL_MINIMIZE LABEL_CLOSE LABEL_LOCK_LIGHTING LABEL_UNLOCK_LIGHTING
	global SUBWINDOW_HEIGHT SUBWINDOW_WIDTH LABEL_STATIC_TETHERS LABEL_DYNAMIC_TETHERS
	global sidelist subwindowList subLockButtons subViewButtons
	global view_set view_lock_scale view_lock_position view_lock_rotate
	global oneEyedFlag windowContents parentName tetherEvent
	global subwindowCleanup tethersAlwaysOn hideScript showScript
	global defaultTethersAlwaysOn raiseScript defaultSubwindowsToplevel
        global useStereo


	#---Setup default properties of the subtogl widget
	set overview 0
	set height $SUBWINDOW_HEIGHT
	set width $SUBWINDOW_WIDTH
	set showtitle 1
	set titleheight 19
	set borderwidth 3
	set resizewidth 10
	set setbuttons 1
	set lockbuttons 1
	set closebutton 0
	set x [expr ( $subWindowCount * 10 ) % 200]
	set y [expr ( $subWindowCount * 10 ) % 200]
	set parent main
	set contents empty
	set relief ridge
	set destroyable 1
	set tethers $defaultTethersAlwaysOn
	set hideHandler {}
	set showHandler {}
	set raiseHandler {}
	set emptyBackground {}
	set toplevel $defaultSubwindowsToplevel

	#---Get the subwindow name out of the path
	set name [string range $windowPath [expr 1 + [string last . $windowPath]] end]

	#---Get the width and height arguments, if any
	foreach switch {height width titleheight borderwidth resizewidth parent overview contents showtitle setbuttons lockbuttons closebutton x y relief name destroyable tethers hideHandler showHandler emptyBackground raiseHandler toplevel} {
		set remainder {}
		foreach {s v} $args {
			if {"$s" == "-$switch"} {
				set $switch $v
			} else {
				lappend remainder $s $v
			}
		}
		set args $remainder
#		set searchPattern "-$switch\[\t \]*(\[^\t \n\]*)"
#		regexp -- $searchPattern $args entireSwitch $switch
#		regsub -- $searchPattern $args "" args
	}
	set toglheight [expr $height - ( $borderwidth * 2 + $titleheight )]
	set toglwidth [expr $width - ( $borderwidth * 2 )]
	set middleheight [expr $height - ( $resizewidth * 2 )]

	#---Create the outer window and border
	if {$toplevel} {
		toplevel $windowPath -height $height -width $width
	} else {
		frame $windowPath -height $height -width $width -borderwidth 0
		frame $windowPath.top -height $resizewidth -borderwidth 0
		frame $windowPath.middle -height $middleheight -borderwidth 0
		frame $windowPath.bottom -height $resizewidth -borderwidth 0
		foreach sideName $sidelist {
			frame $windowPath.$sideName -height $resizewidth -width $resizewidth\
										-borderwidth $borderwidth\
										-relief $relief -cursor $sideName
		}
		pack $windowPath.top_left_corner -in $windowPath.top -side left
		pack $windowPath.top_side -expand yes -fill x -in $windowPath.top -side left
		pack $windowPath.top_right_corner -in $windowPath.top -side left
		pack $windowPath.left_side -expand yes -fill both -in $windowPath.middle -side left
		pack $windowPath.right_side -expand yes -fill both -in $windowPath.middle -side right
		pack $windowPath.bottom_left_corner -in $windowPath.bottom -side left
		pack $windowPath.bottom_side -expand yes -fill x -in $windowPath.bottom -side left
		pack $windowPath.bottom_right_corner -in $windowPath.bottom -side left
		pack $windowPath.top -fill x -side top
		pack $windowPath.middle -expand yes -fill both
		pack $windowPath.bottom -fill x -side bottom
	}

	if {$emptyBackground == {}} {
		frame $windowPath.innards -height [expr $titleheight + $toglheight] -width $toglwidth -borderwidth 0
	} else {
		frame $windowPath.innards -height [expr $titleheight + $toglheight] -width $toglwidth -borderwidth 0 -background $emptyBackground
	}
	frame $windowPath.innards.title -height $titleheight -width $toglwidth -relief raised -borderwidth 2 -cursor fleur
	if {$contents == "3D"} {
		eval "togl $windowPath.innards.togl $args -stereo $useStereo -sharecontext main -ident $name -height $toglheight -width $toglwidth -stencil true"
		$windowPath.innards.togl positionUpdated $x $y $width $height
		set toglPath($name) $windowPath.innards.togl
	} elseif {$contents == "colorkey"} {
		eval "togl $windowPath.innards.togl $args -ident $name -height $toglheight -width $toglwidth"
		set toglPath($name) $windowPath.innards.togl
		bindtags $windowPath.innards.togl "title_$name [bindtags $windowPath.innards.togl]"
	} else {
		set toglPath($name) $windowPath.innards
	}

	#---Set up the variables for window management
	set xPlacement($name) $x
	set yPlacement($name) $y
	set xSize($name) $width
	set ySize($name) $height
	set xInnerSize($name) $toglwidth
	set yInnerSize($name) [expr $titleheight + $toglheight]
	set resizeWidth($name) $resizewidth
	set borderWidth($name) $borderwidth
	set titleHeight($name) $titleheight
	set absoluteWindowPath($name) $windowPath
	set windowConfiguration($name) ""
	set parentWindowPath($name) $toglPath($parent)
	set parentName($name) $parent
	set windowStackLevel($name) [expr $windowStackLevel($parent) + 1]
	set windowContents($name) $contents
	set tetherEvent($name) {}
	set subwindowCleanup($name) "subwindowCleanup $name"
	set tethersAlwaysOn($name) $tethers
	set hideScript($name) $hideHandler
	set showScript($name) $showHandler
	set raiseScript($name) $raiseHandler
	set isToplevel($name) $toplevel

	#---Create the title panel
	if {$titleheight >= 0} {
		pack $windowPath.innards.title -fill x
	}
	if {$contents == "3D"} {
		if {$setbuttons} {
			#--Set/Get view buttons
			frame $windowPath.innards.viewButtons -background black
			set sepcount 0
			foreach {action} $subViewButtons {
				if {[string compare $action "separator"] == 0} {
					frame $windowPath.innards.viewButtons.separator$sepcount -height 2 -width 2 -background black
					pack $windowPath.innards.viewButtons.separator$sepcount
					incr sepcount
				} else {
					button $windowPath.innards.viewButtons.$action \
						-image bitmap_$action -height 8 \
						-command "recordAndExecMaybe \"$windowPath.innards.togl view_$action\""
					pack $windowPath.innards.viewButtons.$action -side top
				}
			}
			place $windowPath.innards.viewButtons -x 0 -y 0.0
		}

		#--Lock buttons
		if {$lockbuttons} {
			frame $windowPath.innards.title.lockButtons -height $titleheight -cursor "arrow"
			foreach {action default} $subLockButtons {
				checkbutton $windowPath.innards.title.lockButtons.$name$action \
					-image bitmap_$default -selectimage bitmap_$action \
					-indicatoron 0 -height 8 \
					-command "recordAndExecMaybe \"$windowPath.innards.togl view_$action \$$name$action\""
				pack $windowPath.innards.title.lockButtons.$name$action -side left
			}
			if {$overview} {
				set action lock_position
				$windowPath.innards.title.lockButtons.$name$action select
			}
		}
		#place $windowPath.innards.lockButtons -relx 1.0 -y 0 -anchor ne
		label $windowPath.innards.title.entry -textvariable positionText($name) -borderwidth 0 -width 35 -pady 0 -justify left -padx 0 -anchor nw
		if {$showtitle > 0} {
			frame $windowPath.innards.title.filler -width 16 -height 1
			label $windowPath.innards.title.label -text "+" -padx 0 -pady 0 -anchor ne -borderwidth 0
			label $windowPath.innards.title.headingLabel -text "H:" -padx 0 -pady 0 -anchor ne -borderwidth 0
			label $windowPath.innards.title.headingEntry -textvariable headingText($name) -borderwidth 0 -width 6 -pady 0 -justify left -padx 0 -anchor nw
			label $windowPath.innards.title.pitchLabel -text "P:" -padx 0 -pady 0 -anchor ne -borderwidth 0
			label $windowPath.innards.title.pitchEntry -textvariable pitchText($name) -borderwidth 0 -width 6 -pady 0 -justify left -padx 0 -anchor nw
			pack $windowPath.innards.title.lockButtons -side right
			pack $windowPath.innards.title.filler -side left
			pack $windowPath.innards.title.label -side left
			pack $windowPath.innards.title.entry -side left -expand yes -fill x
			pack $windowPath.innards.title.headingLabel -side left
			pack $windowPath.innards.title.headingEntry -side left -expand yes -fill x
			pack $windowPath.innards.title.pitchLabel -side left
			pack $windowPath.innards.title.pitchEntry -side left -expand yes -fill x
			bindtags $windowPath.innards.title.label "title_$name"
			bindtags $windowPath.innards.title.headingLabel "title_$name"
			bindtags $windowPath.innards.title.headingEntry "title_$name"
			bindtags $windowPath.innards.title.pitchLabel "title_$name"
			bindtags $windowPath.innards.title.pitchEntry "title_$name"
		} elseif {$showtitle != -1} {
			pack $windowPath.innards.title.entry -side left -expand yes -fill x
		}
	} else {
		label $windowPath.innards.title.entry -textvariable positionText($name) -borderwidth 0 -width 35 -pady 0 -justify left -padx 0 -anchor nw
		pack $windowPath.innards.title.entry -side left -expand yes -fill x
	}
	bindtags $windowPath.innards.title.entry "title_$name"
	bindtags $windowPath.innards.title "title_$name"
	#--Close button (added by mdp 5/28/2008)
	if {$closebutton} {
		if {$destroyable} {
			set closeCommand "subwindowDestroy"
		} else {
			set closeCommand "subwindowMinimize"
		}
		button $windowPath.innards.title.close \
				-image bitmap_close -height 8 -cursor arrow \
				-command "recordAndExecMaybe \"$closeCommand $name\""
		pack $windowPath.innards.title.close -side right
	}

	#---Create the actual windows
	if {$contents != "empty"} {
		pack $windowPath.innards.togl -expand yes -fill both
		if {$oneEyedFlag} {
#$windowPath.innards.togl configure -cursor "@[file join $appdir blank.cur]"
                    $windowPath.innards.togl configure -cursor "none"
		} else {
			$windowPath.innards.togl configure -cursor "arrow"
		}
	}
	if {$toplevel} {
		pack $windowPath.innards -expand yes -fill both
	} else {
		place $windowPath.innards -x $borderwidth -y $borderwidth -width $xInnerSize($name) -height $yInnerSize($name)
		subwindowPlace $name $x $y $width $height
		subwindowRaise $name
	}

	#---Create the option menu for the right-click
	set menuPath [createOptionMenu $windowPath $name]
	if {$contents == "3D"} {
		$menuPath add command -label $LABEL_MINIMIZE -command "$toglPath($name) minimize"
		$menuPath add separator
		$menuPath add command -label $LABEL_LOCK_LIGHTING -command "$toglPath($name) lighting lock"
		$menuPath add command -label $LABEL_UNLOCK_LIGHTING -command "$toglPath($name) lighting unlock"
		$menuPath add command -label $LABEL_STATIC_TETHERS -command "subwindowTethersAlwaysOn $name"
		$menuPath add command -label $LABEL_DYNAMIC_TETHERS -command "subwindowTethersAlwaysOn $name 0"
		$menuPath add command -label "Hide proxy" -command "$toglPath($name) inParent proxy show 0; $toglPath($name) inParent center show 0"
		$menuPath add command -label "Show proxy" -command "$toglPath($name) inParent proxy show 1; $toglPath($name) inParent center show 1"
		$menuPath add separator
	}
	if {$destroyable} {
		$menuPath add command -label $LABEL_CLOSE -command "subwindowDestroy $name"
	} else {
		$menuPath add command -label $LABEL_CLOSE -command "subwindowMinimize $name"
	}

	#---Define behavior of tethers
	if {$contents == "3D" && $tethers != 1} {
		$windowPath.innards.togl inParent tethers show -1
	}

	#---Create the bindings
	subwindowEnableBindings $name
	if {$contents == "3D"} {
		incr subWindowCount
	}
	lappend subwindowList $name
	
	return $windowPath
}

#-----------------------------------------------------------------------------
#verify the current list of all existing subwidows and return it when done.
proc subwindowVerifyList {} {
	global subwindowList absoluteWindowPath

	set trueList ""

	foreach name $subwindowList {
		if {[winfo exists $absoluteWindowPath($name)]} {
			lappend trueList $name
		}
	}

	set subwindowList $trueList
	return $trueList
}

#-----------------------------------------------------------------------------
#Enable user interaction with the passed subwindow
proc subwindowEnableBindings {name {bindBorder 1}} {
	global toglPath absoluteWindowPath recordingState isToplevel
	global RECORDER_RECORDING RECORDER_PLAYING
	global sidelist windowContents tethersAlwaysOn

	set windowPath $absoluteWindowPath($name)
	set contents $windowContents($name)

	if {$recordingState & $RECORDER_PLAYING} {
		return
	} elseif {$recordingState & $RECORDER_RECORDING} {
		set rec "recordAndExec \""
		set recEnd "\""
	} else {
		set rec {}
		set recEnd {}
	}

	if {$bindBorder && ($isToplevel($name) == 0)} {
		#---Title bindings---
		bind title_$name <ButtonPress-1> "$rec subwindowMoveMouseStart $name %X %Y$recEnd"
		bind title_$name <B1-Motion> "$rec subwindowMoveMouseMove $name %X %Y$recEnd"
		bind title_$name <ButtonRelease-1> "$rec subwindowMoveMouseStop $name %X %Y$recEnd"
		bind title_$name <ButtonPress-3> "$rec postSubToglMenu $name %x %y$recEnd"

		#---Border bindings---
		set xType -1
		set yType -1
		foreach side $sidelist {
			if {$xType == 0 && $yType == 0} {
				incr xType
			}

			bind $windowPath.$side <ButtonPress-1> "$rec subwindowResizeMouseStart $name %X %Y$recEnd"
			bind $windowPath.$side <B1-Motion> "$rec subwindowResizeMouseMove $xType $yType $name %X %Y$recEnd"
			bind $windowPath.$side <ButtonRelease-1> "$rec subwindowResizeMouseStop $xType $yType $name %X %Y$recEnd"

			incr xType
			if {$xType > 1} {
				set xType -1
				incr yType
			}
		}
	}

	#---Togl bindings---
	if {$contents != "empty"} {
		set localPath $toglPath($name)

		bind $localPath <KeyPress>      "$localPath keyboard %x %y %s %A 0 %N"
		bind $localPath <KeyRelease>    "$localPath keyboard %x %y %s %A 1 %N"
		bind $localPath <Motion>        "$localPath motion %x %y %s 0"
		if {$contents == "3D"} {
			bind $localPath <ButtonPress>   "focus $localPath; $localPath mouse %x %y %s %b 0"
		}
		bind $localPath <ButtonRelease> "focus $localPath; $localPath mouse %x %y %s %b 1"
		bind $localPath <MouseWheel>    "focus $localPath; if {%D > 0} {$localPath mouse %x %y %s 4 0} else {$localPath mouse %x %y %s 5 0}"
		if {!$bindBorder || $tethersAlwaysOn($name) == 1} {
			bind $localPath <Enter>         "$localPath motion %x %y %s 1"
			bind $localPath <Leave>         "$localPath motion %x %y %s -1"
		} else {
			bind $localPath <Enter>         "$localPath motion %x %y %s 1; subwindowEvent Enter $name"
			bind $localPath <Leave>         "$localPath motion %x %y %s -1; subwindowEvent Leave $name"
		}
	}
}

#-----------------------------------------------------------------------------
#Set whether or not a subwindow's tethers are shown constantly
proc subwindowTethersAlwaysOn {name {value 1}} {
	global toglPath windowContents tethersAlwaysOn parentName

	if {$windowContents($name) != "3D"} return

	set localPath $toglPath($name)
	set tethersAlwaysOn($name) $value
	if {$value == 1} {
		bind $localPath <Enter>         "$localPath motion %x %y %s 1"
		bind $localPath <Leave>         "$localPath motion %x %y %s -1"
		$localPath inParent tethers show 1
	} else {
		bind $localPath <Enter>         "$localPath motion %x %y %s 1; subwindowEvent Enter $name"
		bind $localPath <Leave>         "$localPath motion %x %y %s -1; subwindowEvent Leave $name"
		$localPath inParent tethers show -1
	}

	gz tellObject .EventMediator.Window.$parentName($name) trigger windowChanged
}

#-----------------------------------------------------------------------------
#Make the tethers appear and disappear
proc subwindowEvent {event name} {
	global toglPath tetherEvent parentName windowContents

	if {$name == "main" || $windowContents($name) != "3D"} {
		return
	}

	set path $toglPath($name)
	if {$event == "Enter"} {
		$path inParent tethers show 1
		subwindowEvent $event $parentName($name)
		set tetherEvent($name) [after 2000 "$path inParent tethers show -1"]
	} elseif {$event == "Leave"} {
		if {$tetherEvent($name) != {}} {
			after cancel $tetherEvent($name)
			set tetherEvent($name) {}
		}
		$path inParent tethers show -1
	}
}

#-----------------------------------------------------------------------------
#Disable user interaction with the passed subwindow
proc subwindowDisableBindings {name} {
	global toglPath absoluteWindowPath sidelist windowContents isToplevel

	set windowPath $absoluteWindowPath($name)
	set contents $windowContents($name)

	#---Title bindings---
	bind title_$name <ButtonPress-1> ""
	bind title_$name <B1-Motion> ""
	bind title_$name <ButtonRelease-1> ""
	bind title_$name <ButtonPress-3> ""

	#---Border bindings---
	if {$isToplevel($name) == 0} {
		set xType -1
		set yType -1
		foreach side $sidelist {
			if {$xType == 0 && $yType == 0} {
				incr xType
			}

			bind $windowPath.$side <ButtonPress-1> ""
			bind $windowPath.$side <B1-Motion> ""
			bind $windowPath.$side <ButtonRelease-1> ""

			incr xType
			if {$xType > 1} {
				set xType -1
				incr yType
			}
		}
	}

	#---Togl bindings---
	if {$contents != "empty"} {
		set localPath $toglPath($name)

		bind $localPath <KeyPress>      ""
		bind $localPath <KeyRelease>    ""
		bind $localPath <Motion>        ""
		bind $localPath <ButtonPress>   ""
		bind $localPath <ButtonRelease> ""
		bind $localPath <MouseWheel>    ""
		bind $localPath <Enter>         ""
		bind $localPath <Leave>         ""
	}
}

#-----------------------------------------------------------------------------
# This procedure places the given window using the given coordinates, relative
# to either the main window or the parent window, depending on the current
# setting.
proc subwindowPlace {name x y width height {update 0}} {
	global subwindowsBehavior SUBWINDOWS_SEPARATE
	global absoluteWindowPath
	global parentWindowPath
	global xPlacement yPlacement
	global xSize ySize xInnerSize yInnerSize borderWidth titleHeight

	set windowPath $absoluteWindowPath($name)

	if {[info exists subwindowBehavior] && $subwindowsBehavior == $SUBWINDOWS_SEPARATE} {
		place $windowPath -in . -x $x -y $y -width $width -height $height
	} else {
		place $windowPath -in $parentWindowPath($name) -x $x -y $y -width $width -height $height
	}

	if {$update} {
		set xPlacement($name) $x
		set yPlacement($name) $y
		set xSize($name) $width
		set ySize($name) $height
		set toglwidth [expr $width - ( $borderWidth($name) * 2 ) ]
		set toglheight [expr $height - ( $borderWidth($name) * 2 ) ]
		set xInnerSize($name) $toglwidth
		set yInnerSize($name) $toglheight
		place $absoluteWindowPath($name).innards -width $toglwidth -height $toglheight
	}
}

#-----------------------------------------------------------------------------
# This procedure raises the given window above those who are its siblings.
proc subwindowRaise {name} {
	global absoluteWindowPath
	global windowStackLevel windowStack raiseScript parentWindowPath

	set windowPath $absoluteWindowPath($name)

	if {$parentWindowPath($name) == "."} {
		catch {raise $windowPath}
	} else {
		catch {raise $windowPath $windowStack($windowStackLevel($name))}
	}
	set windowStack($windowStackLevel($name)) $windowPath
	if {$raiseScript($name) != ""} {
		eval "$raiseScript($name)"
	}
}

#-----------------------------------------------------------------------------
# Procedures that implement a subwindow move, upon grabbing the title-bar for
# instance
#-----------------------------------------------------------------------------
proc subwindowMoveMouseStart {name x y} {
	global xMouseStart yMouseStart tetherEvent toglPath windowContents
	global tethersAlwaysOn

	set xMouseStart $x
	set yMouseStart $y

	subwindowRaise $name

	#--Make tethers appear--
	if {$tetherEvent($name) != {}} {
		after cancel $tetherEvent($name)
		set tetherEvent($name) {}
	}
	if {$windowContents($name) == "3D" && $tethersAlwaysOn($name) != 1} {
 		$toglPath($name) inParent tethers show 1
	}
}

#-----------------------------------------------------------------------------
proc subwindowMoveMouseMove {name x y {update 0}} {
    global xMouseStart yMouseStart
	global xPlacement yPlacement
	global absoluteWindowPath
	global toglPath windowContents
	global recordingState RECORDER_PLAYING

	set deltaX [expr $x - $xMouseStart]
	set deltaY [expr $y - $yMouseStart]

	subwindowCheckCoords $name deltaX deltaY

	set newX [expr $xPlacement($name) + $deltaX]
	set newY [expr $yPlacement($name) + $deltaY]

	place $absoluteWindowPath($name) -x $newX -y $newY

	if {$update} {
		set xPlacement($name) $newX
		set yPlacement($name) $newY
	}

	if {($recordingState & $RECORDER_PLAYING) == 0} {
		if {$windowContents($name) == "3D"} {
			$toglPath($name) positionUpdated $newX $newY
		}
	}
}

#-----------------------------------------------------------------------------
proc subwindowMoveMouseStop {name x y} {
	global toglPath windowContents tethersAlwaysOn

    subwindowMoveMouseMove $name $x $y 1

	#--Make tethers vanish--
	if {$windowContents($name) == "3D" && $tethersAlwaysOn($name) != 1} {
		$toglPath($name) inParent tethers show -1
	}
}

#-----------------------------------------------------------------------------
# Procedures that implement a subwindow resize, upon grabbing an edge or
# corner, for instance
#-----------------------------------------------------------------------------
proc subwindowResizeMouseStart {name x y} {
	global xSizeStart ySizeStart
	global xSize ySize
	global xMouseStart yMouseStart

	set xSizeStart $xSize($name)
	set ySizeStart $ySize($name)
	set xMouseStart $x
	set yMouseStart $y

	subwindowRaise $name
}

#-----------------------------------------------------------------------------
proc subwindowResizeMouseMove {xType yType name x y {update 0}} {
	global xSizeStart ySizeStart
	global xSize ySize
    global xMouseStart yMouseStart
	global xPlacement yPlacement
	global xInnerSize yInnerSize
	global absoluteWindowPath
	global toglPath windowContents
	global recordingState RECORDER_PLAYING

	set path $absoluteWindowPath($name)
    set deltaX [expr $x - $xMouseStart]
	set deltaY [expr $y - $yMouseStart]

	subwindowCheckCoords $name deltaX deltaY $xType $yType

	set newX [expr $xSize($name) + $deltaX * $xType]
	set newY [expr $ySize($name) + $deltaY * $yType]
	set newXinner [expr $xInnerSize($name) + $deltaX * $xType]
	set newYinner [expr $yInnerSize($name) + $deltaY * $yType]
	set newXloc $xPlacement($name)
	set newYloc $yPlacement($name)

	if {$xType == -1} {
	    incr newXloc $deltaX
	}
	if {$yType == -1} {
		incr newYloc $deltaY
	}

	if {$update} {
	    set xSize($name) $newX
	    set ySize($name) $newY
	    set xInnerSize($name) $newXinner
	    set yInnerSize($name) $newYinner
		set xPlacement($name) $newXloc
		set yPlacement($name) $newYloc
	}

	place $path -x $newXloc -y $newYloc -width $newX -height $newY
	place $path.innards -width $newXinner -height $newYinner

	if {($recordingState & $RECORDER_PLAYING) == 0} {
		if {$windowContents($name) == "3D"} {
			$toglPath($name) positionUpdated $newXloc $newYloc $newX $newY
		}
	}
}

#-----------------------------------------------------------------------------
proc subwindowResizeAndMove {name width height x y} {
	global xSize ySize
	global xPlacement yPlacement
	global xInnerSize yInnerSize
	global absoluteWindowPath
	global toglPath windowContents

	set path $absoluteWindowPath($name)

	set newX $width
	set newY $height
	set newXinner [expr $xInnerSize($name) + $width - $xSize($name)]
	set newYinner [expr $yInnerSize($name) + $height - $ySize($name)]
	set newXloc $x
	set newYloc $y

    set xSize($name) $newX
    set ySize($name) $newY
    set xInnerSize($name) $newXinner
    set yInnerSize($name) $newYinner
	set xPlacement($name) $newXloc
	set yPlacement($name) $newYloc

	place $path -x $newXloc -y $newYloc -width $newX -height $newY
	place $path.innards -width $newXinner -height $newYinner

	if {$windowContents($name) == "3D"} {
		$toglPath($name) positionUpdated $newXloc $newYloc $newX $newY
	}
}

#-----------------------------------------------------------------------------
proc subwindowResizeMouseStop {xType yType name x y} {
	subwindowResizeMouseMove $xType $yType $name $x $y 1
}

#-----------------------------------------------------------------------------
# Procedure that ensures that the window never goes too far off the edge of
# the window.
#-----------------------------------------------------------------------------
proc subwindowCheckCoords {name deltax deltay {xType -2} {yType -2}} {
	global resizeWidth borderWidth xInnerSize yInnerSize
	global xPlacement xSize yPlacement ySize titleHeight

	upvar $deltax deltaX
	upvar $deltay deltaY

	set minInner [expr $resizeWidth($name) * 3 - $borderWidth($name) * 2]
	set minY [expr -1 * $titleHeight($name) + 1]
	set minX [expr $borderWidth($name) + 1]

	#---Don't let the window get too small---
	if {$xType > -2} {
		set diffX [expr $xInnerSize($name) + $deltaX * $xType]
		if {$diffX <= $minInner} {
			set incrDeltaX [expr ( -1 * $diffX + $minInner ) * $xType]
			incr deltaX $incrDeltaX
		}

		set diffY [expr $yInnerSize($name) + $deltaY * $yType]
		if {$diffY <= $minInner} {
			set incrDeltaY [expr ( -1 * $diffY + $minInner ) * $yType]
			incr deltaY $incrDeltaY
		}
	} else {
		set xType 1
		set yType -1
	}

	#---Keep part of the title bar on the screen---
	#---(At least for the top and left sides)---
	set diffX [expr $xPlacement($name) + $xSize($name) + $deltaX * $xType]
	if {$diffX <= $minX} {
		set deltaX [expr $minX - $xPlacement($name) - $xSize($name)]
	}
	if {$yType < 1} {
		set diffY [expr $yPlacement($name) + $deltaY]
		if {$diffY < $minY} {
			incr deltaY [expr -1 * $diffY + $minY]
		}
	}
}


#-----------------------------------------------------------------------------
#This function creates a new menu with the passed path, with commands applied
#to the passed window name.
#-----------------------------------------------------------------------------
proc createOptionMenu {windowPath name} {
	global LABEL_RESTORE_SUBS LABEL_OVERVIEW
	global toglPath windowContents

	set menuPath $windowPath.optionMenu
	menu $menuPath -tearoff 0
	if {$windowContents($name) == "3D"} {
		$menuPath add command -label $LABEL_OVERVIEW -command "$toglPath($name) overview"
		$menuPath add separator
		$menuPath add command -label $LABEL_RESTORE_SUBS -command "restoreSubwindows $name"
	}
	return $menuPath
}

#-----------------------------------------------------------------------------
#This function causes a ToglWindow to restore its subwindows.
#-----------------------------------------------------------------------------
proc restoreSubwindows {name} {
	global toglPath

	$toglPath($name) restoreSubwindows
}

#-----------------------------------------------------------------------------
proc subwindowRestore {name} {
	global absoluteWindowPath
	global windowConfiguration showScript

	if {$windowConfiguration($name) == ""} {
	    return
	}
	eval "place configure $absoluteWindowPath($name) $windowConfiguration($name)"
	set windowConfiguration($name) ""
	eval $showScript($name)
}

#-----------------------------------------------------------------------------
proc subwindowMinimize {name} {
	global absoluteWindowPath
	global windowConfiguration hideScript

	set windowConfiguration($name) [place info $absoluteWindowPath($name)]
	place forget $absoluteWindowPath($name)
	eval $hideScript($name)
}

#-----------------------------------------------------------------------------
proc subwindowDestroy {name} {
	global absoluteWindowPath subwindowCleanup
	global mainWindow
	global recordingState RECORDER_RECORDING

	focus $mainWindow
	destroy $absoluteWindowPath($name)
	eval $subwindowCleanup($name)

	if {$recordingState & $RECORDER_RECORDING} {
		$mainWindow recordCommand "subwindowDestroy $name"
	}
}

#-----------------------------------------------------------------------------
proc subwindowCleanup {name} {
	global xPlacement yPlacement xSize ySize xInnerSize yInnerSize resizeWidth
	global borderWidth titleHeight absoluteWindowPath toglPath parentWindowPath
	global windowConfiguration windowStackLevel positionText headingText pitchText
	global windowContents tetherEvent parentName subwindowCleanup tethersAlwaysOn
	global hideScript showScript

	unset yPlacement($name)
	unset xSize($name)
	unset ySize($name)
	unset xInnerSize($name)
	unset yInnerSize($name)
	unset resizeWidth($name)
	unset borderWidth($name)
	unset titleHeight($name)
	unset absoluteWindowPath($name)
	unset toglPath($name)
	unset parentWindowPath($name)
	unset windowConfiguration($name)
	unset windowStackLevel($name)
	unset positionText($name)
	if {$windowContents($name) == "3D"} {
		catch {unset headingText($name)}
		catch {unset pitchText($name)}
	}
	unset tetherEvent($name)
	unset windowContents($name)
	unset parentName($name)
	unset subwindowCleanup($name)
	unset tethersAlwaysOn($name)
	unset hideScript($name)
	unset showScript($name)
}

#-----------------------------------------------------------------------------
proc subwindowTitle {name string} {
	global positionText windowContents absoluteWindowPath

	set windowPath $absoluteWindowPath($name)
	set positionText($name) $string
	if {$string == ""} {
		pack forget $windowPath.innards.title
	} else {
		if {$windowContents($name) == "empty"} {
			$windowPath.innards.title.entry configure -justify center -anchor center
			pack $windowPath.innards.title -fill x
		} else {
			$windowPath.innards.title.entry configure -justify center -anchor center
			pack $windowPath.innards.title -before $windowPath.innards.togl -fill x
		}
	}
	return
}

#-----------------------------------------------------------------------------
proc postSubToglMenu {name localX localY args} {
	global absoluteWindowPath
	global toglPath
	global recordingState RECORDER_PLAYING

	set windowPath $absoluteWindowPath($name)
	set togl $toglPath($name)

	set globalX [expr $localX + [winfo rootx $togl]]
	set globalY [expr $localY + [winfo rooty $togl]]
	set menuPath $windowPath.optionMenu

	foreach arg $args {
		eval "$menuPath $arg"
	}
	if {($recordingState & $RECORDER_PLAYING) == 0} {
		tk_popup $menuPath $globalX $globalY
	}
}

#-----------------------------------------------------------------------------
proc bgerror {args} {
	global errorInfo

	puts "tcl-error" 1 "$errorInfo"
}
