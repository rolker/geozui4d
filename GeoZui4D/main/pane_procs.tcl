#-----------------------------------------------------------------------------
# pane_procs.tcl -- Procedures for handling control-drag-and-dropping of
#                   windows.
#-----------------------------------------------------------------------------
global numPanes panedWindows panePath paneList
global dragWindowName sourcePane paneIsPopout
set numPanes 0
set panedWindows {}
set dragWindowName ""
set sourcePane ""
set paneList {}

#-----------------------------------------------------------------------------
# Start a window-drag operation, which allows a window to be moved from one
# sashed pane to another.
#-----------------------------------------------------------------------------
proc dragWindow {win x y} {
	global dragWindowName sourcePane

	set sourcePane [paneContaining $x $y]
	if {$sourcePane == ""} return; #don't know how this could happen...

	set dragWindowName $win
	$win configure -cursor icon
}

#-----------------------------------------------------------------------------
# Plop the currently dragged window into the pane under the x,y position.  If
# there is already something there, swap it out with the dragged window.
#-----------------------------------------------------------------------------
proc dropWindow {win x y} {
	global dragWindowName sourcePane

	$win configure -cursor arrow
	if {$dragWindowName != $win} return; #don't know how this could happen...
	set dragWindowName ""

	set dropTarget [paneContaining $x $y]
	if {$dropTarget == ""} return
	paneSwap $sourcePane $dropTarget
}

#-----------------------------------------------------------------------------
# Determine the pane that is under the passed x,y (mouse cursor)
#-----------------------------------------------------------------------------
proc paneContaining {x y} {
	global paneList panePath

	set candidatePane ""
	foreach pane $paneList {
		set path $panePath($pane)
		set minX [winfo rootx $path]	
		set minY [winfo rooty $path]
		set maxX [expr $minX + [winfo width $path]]
		set maxY [expr $minY + [winfo height $path]]
		if {$minX <= $x && $x <= $maxX && $minY <= $y && $y <= $maxY} {
			if {$candidatePane == "" || [string index $candidatePane 0] != "s"} {
				set candidatePane $pane
			} else {
				#the candidatePane is already a subwindow.  Don't change it!
				break
			}
		}
	}
	return $candidatePane
}

#-----------------------------------------------------------------------------
# Make sure the right bindings are on each pane, contents
#-----------------------------------------------------------------------------
proc recursiveBindTagEnsure {win tag} {

	set myTags [bindtags $win]
	if {[lsearch -exact $myTags $tag] < 0} {
		lappend myTags $tag
		bindtags $win $myTags
	}

	foreach child [winfo children $win] {
		recursiveBindTagEnsure $child $tag
	}
}

#-----------------------------------------------------------------------------
# Show the passed contents (win) into the passed pane
#-----------------------------------------------------------------------------
proc panePack {win pane} {
	global paneChildren paneIsPopout paneRaiseBoundary panePath raiseScript

	set path $panePath($pane)
	pack $win -in $path -expand true -fill both -side left
	set paneChildren($pane) $win
	set paneParent($pane) $pane

	if {$paneIsPopout($pane)} {
		raise $win
		set raiseScript(pane_$pane) "raise $win"
	} else {
		raise $win $path
	}

	recursiveBindTagEnsure $win $win
	bind $win <Control-ButtonPress-2> "dragWindow $win %X %Y; break"
	bind $win <Control-ButtonRelease-2> "dropWindow $win %X %Y; break"
	bind $win <Double-Button-2> "panePopout $win %X %Y; break"
}

#-----------------------------------------------------------------------------
# Hide the pane contents for the passed pane
#-----------------------------------------------------------------------------
proc paneForget {pane} {
	global paneChildren paneParent raiseScript paneIsPopout

	set win $paneChildren($pane)
	if {$win != ""} {
		pack forget $win
		set paneChildren($pane) ""
	}
	set paneParent($pane) ""
	if {$paneIsPopout($pane)} {
		set raiseScript(pane_$pane) ""
	}
}

#-----------------------------------------------------------------------------
# Swap the locations of the contents of pane1 and pane2
#-----------------------------------------------------------------------------
proc paneSwap {pane1 pane2} {
	global paneChildren

	set contents1 $paneChildren($pane1)
	set contents2 $paneChildren($pane2)

	if {$contents1 != ""} {
		paneForget $pane1
	}

	if {$contents2 != ""} {
		panePack $contents2 $pane1
	}

	if {$contents1 != ""} {
		panePack $contents1 $pane2
	}
}

#-----------------------------------------------------------------------------
# args is a list of what goes in each pane (a number for a plain pain, another
# argument list for a nested panedwindow).
# Ex: "paneLayout top h {left v 1 2} {right v 3 {mid h 4 6} 5}"
#-----------------------------------------------------------------------------
proc paneLayout {name direction args} {
	set path [eval "paneLayoutHelper {} $name $direction $args"]
	pack $path -side top -fill both -expand true
	return $path
}

#-----------------------------------------------------------------------------
proc paneLayoutHelper {parent name direction args} {
	global numPanes paneChildren paneParent panedWindows paneIsPopout
	global panePath paneList

	if {$parent == {}} {
		set toplevel 1
		set pwPath .$name
	} else {
		set toplevel 0
		set pwPath $parent.$name
	}
	eval "panedwindow $pwPath -orient $direction -borderwidth 0 -sashwidth 2 -sashrelief raised"
	set panePath($name) $pwPath
	bind $pwPath <Configure> "paneUsePolicyOnce $name"
	lappend panedWindows $name
	foreach pane $args {
		if {[regexp \[0-9\] [string range $pane 0 0]]} {
			set path [paneCreate $pwPath $name $pane]
			$pwPath add $path
			set paneChildren($pane) ""
			set paneParent($pane) $name
			set paneIsPopout($pane) 0
		} else {
			set childPane [eval "paneLayoutHelper $pwPath $pane"]
			$pwPath add $childPane
		}
	}

	set paneParent($name) $parent

	return $pwPath
}

#-----------------------------------------------------------------------------
# Create a generic sashed window pane.
#-----------------------------------------------------------------------------
proc paneCreate {parentPath parent pane {popout 0}} {
	set fullPath [paneRegister $parentPath $parent $pane pane_$pane $popout]
	return [frame $fullPath]
}

#-----------------------------------------------------------------------------
# Store info about the passed window for later reference
#-----------------------------------------------------------------------------
proc paneRegister {parentPath parent pane pane_name popout} {
	global paneChildren paneParent numPanes paneIsPopout paneList panePath

	if {$parentPath == {}} {
		set fullPath $pane_name
	} else {
		set fullPath $parentPath.$pane_name
	}
	set paneChildren($pane) ""
	set paneParent($pane) $parent
	set paneIsPopout($pane) $popout
	set panePath($pane) $fullPath
	lappend paneList $pane
	incr numPanes
	return $fullPath
}

#-----------------------------------------------------------------------------
# Allow user to create a resize policy to use during every resize.  See
# paneUsePolicyOnce for how the arguments work.
#-----------------------------------------------------------------------------
proc paneUsePolicy {name args} {
	global paneResizePolicy

	set paneResizePolicy($name) $args
	paneUsePolicyOnce $name
}

#-----------------------------------------------------------------------------
# Allow user to apply a resize policy once (now).  Arguments are always in
# pairs, "keyword operand", with keywords functioning as follows:
#	* percent op -- sash should be at op% of the way across the pane
#	                Ex: "paneUsePolicy toplevelSash percent 82"
#	* formula op -- sash should be at the location specified by the formula op,
#	                where $h or $w can appear in the formula as height or width
#	                Ex: "paneUsePolicy sideSash formula 130 formula {$h * 0.5 - 130}"
#-----------------------------------------------------------------------------
proc paneUsePolicyOnce {name args} {
	global paneResizePolicy panePath

	set win $panePath($name)

	if {$args == ""} {
		if {[array names paneResizePolicy $name] == ""} return
		set policy $paneResizePolicy($name)
	} else {
		set policy $args
	}

	if {$policy == ""} return

	set w [winfo width $win]
	set h [winfo height $win]
	set direction [$win cget -orient]

	set i 0
	foreach {type spec} $policy {
		if {$type == "percent"} {
			if {$direction == "vertical"} {
				set x 0
				set y [expr int($h * $spec * 0.01)]
			} else {
				set x [expr int($w * $spec * 0.01)]
				set y 0
			}
			$win sash place $i $x $y
			#puts "$win sash place $i $x $y"
		} elseif {$type == "formula"} {
			set val [expr int($spec)]
			$win sash place $i $val $val
			#puts "$win sash place $i $val $val"
		}
		incr i
	}
}

#-----------------------------------------------------------------------------
# A debug procedure that lists the width and height of each pane
#-----------------------------------------------------------------------------
proc paneEnsureSanity {} {
	global panedWindows

	foreach pw $panedWindows {
		puts "$pw: [winfo width $pw] x [winfo height $pw]"
	}
}

#-----------------------------------------------------------------------------
# Move the passed window contents into a new (popped out) subwindow
#-----------------------------------------------------------------------------
proc panePopout {panewin x y} {

	set pane [paneContaining $x $y]
	if {$pane == ""} return

	if {$x - 100 > 0} {
		set x [expr $x - 100]
	}
	if {$y - 100 > 0} {
		set y [expr $y - 100]
	}
	set newPane [paneSubwindow -x $x -y $y]
	if {$newPane == {}} {
		return {}
	}

	paneSwap $pane $newPane
	return $newPane
}

#-----------------------------------------------------------------------------
# Create a floating, empty subwindow that can house the same things other panes can
#-----------------------------------------------------------------------------
proc paneSubwindow {args} {
	global panedWindows numPanes toglPath subwindowCleanup

	incr numPanes
	set pane sub$numPanes
	set name pane_$pane
	subwindow .subwindowPane_$pane -name $name -parent top -width 285 -height 285 -titleheight 5 $args -contents empty
	paneRegister {} {} $pane $toglPath($name) 1
	set subwindowCleanup($name) "paneSubwindowCleanup $pane"
	raise .subwindowPane_$pane
	return $pane
}

#-----------------------------------------------------------------------------
# Remove the subwindow from the paneList
#-----------------------------------------------------------------------------
proc paneSubwindowCleanup {name} {
	global paneList

	set index [lsearch -exact $paneList $name]
	if {$index >= 0} {
		set paneList [lreplace $paneList $index $index]
	}
}

#-----------------------------------------------------------------------------
# Create a 3D window for use in these custom panes
#-----------------------------------------------------------------------------
proc paneToglWindow {name parentName type args} {
	global toglPath useStereo

	set procBody [info body subtogl]
	set procArgs [info args subtogl]

	#re-direct subtogl
	proc subtogl {windowPath args} "
		global absoluteWindowPath windowContents toglPath tetherEvent parentName

		set iname \[string range \$windowPath \[expr 1 + \[string last . \$windowPath\]\] end\]
		eval \"togl .$name -double true -depth true -stereo $useStereo -sharecontext main -ident \$iname -width 200 -height 200  -stencil true $args\"
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
		place .$name -x 50 -y 50
 	"

	set parent $toglPath($parentName)

	#create a window to stuff in a pane
	if {$type == "overview"} {
		$parent overview
	} elseif {$type == "profile" || $type == "cross-track"} {
		if {$type == "profile"} {
			set cutWidth 100
		} else {
			set cutWidth 400
		}
		#createCutaway $parentName $type -name $name -trackOffset 1000 -cutWidth $cutWidth -cutAngle 3
		createCutaway $parentName $type -name $name -trackOffset 100 -cutWidth $cutWidth -cutAngle 3
		set win $toglPath($name)
		#$win localHeightExaggeration 6.0
		$win perspective 0
		$win wireframe 1
	} else {
		$parent createSubwindow $name
	}

	#re-enable old subtogl
	proc subtogl $procArgs $procBody

	bind .$name <Unmap> ".$name priority 0.0"
	bind .$name <Map> ".$name priority restore"
	return .$name
}