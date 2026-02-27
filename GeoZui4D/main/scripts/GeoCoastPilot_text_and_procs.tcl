#-----------------------------------------------------------------------------
# GeoCoastPilot Coast Pilot text window and procedures that link all the parts
# together (3D GUI, text window, info frame in the control panel.
#-----------------------------------------------------------------------------

global minLat maxLat minLon maxLon
global coastPilot
global maxScale minScale
global interactivityBase interactivityMotion
global interactivityMotionRequests
global speedFactor standardEmphasisFactor
global northUpMode

#----------------------------------------------------------------------
#---GeoCoastPilot Textual area Panel---
set coastPilot [text .pane.f1.frame.coastPilot -wrap word -cursor arrow -width 80]
set cpScroll [scrollbar .pane.f1.frame.scroll -command "$coastPilot yview"]
$coastPilot configure  -yscrollcommand "$cpScroll set"
set borderColorNormal "0.9 0.7 0.7"
set borderColorHighlighted "1.0 0.0 0.0"

pack $cpScroll -fill y -expand no -side right
pack $coastPilot -fill both -expand yes -side right

global datadir
#set file [open [file join $datadir "textPanel.tcl" ] "r"]
#$coastPilot insert end [read $file]
#close $file
$coastPilot tag configure searchResult -background "#AACCEE" -borderwidth "2"
$coastPilot configure -state disabled

#----------------------------------
proc featureClicked {name} {
	global featureObject featureCoords mostRecentFeature infoFeature
	
	set infoFeature ""
	set ob $featureObject($name)
	#puts "Zero"
	gz tellObject land select 0; #deselect any selected objects
	
	if {$ob == {}} {
		#displayInfoFor "$name"
		set coords $featureCoords($name)
		set obName ""
		if {$coords != ""} {
			moveTowardCoords $coords
			catch "set obName \[gz tellObject #...$name getName\]"
			if {$obName != ""} {
				gz tellObject $obName select 1
			}
		}
		objectClicked "#$name" 0
		if {$coords == ""} {
			catch "set obName \[gz tellObject #...$name getName\]"
			if {$obName != ""} {
				gz tellObject $obName select 1
				moveToward $obName
			}
		}
		return
	}
	
	if {$mostRecentFeature != ""} {
		rangeReached
	}

	gz tellObject $ob select 1
	objectClicked .Posable.Object.$ob 0
	moveToward .Posable.Object.$ob
}

#----------------------------------
proc moveTowardCoords {coordSpec} {
	global currentProjection
	
	set firstCoord ""
	set secondCoord ""
	regexp {[^0-9]*([^NEWS]*)([NEWS])[^,]*,[^0-9]*([^NEWS]*)([NEWS])} $coordSpec all firstCoord firstDir secondCoord secondDir
	
	if {$firstCoord == "" || $secondCoord == ""} return;

	if {$firstDir == "S" || $firstDir == "N"} {
		set lat [parseDegrees $firstCoord $firstDir]
		set lon [parseDegrees $secondCoord $secondDir]
	} else {
		set lat [parseDegrees $secondCoord $secondDir]
		set lon [parseDegrees $firstCoord $firstDir]
	}

	foreach {wx wy} [gz projection apply $currentProjection $lon $lat] {}
	gz posable set .Animator.direction xy $wx $wy
	moveToward .Animator.direction	
}

#----------------------------------
proc parseDegrees {coord dir} {
	set degrees ""
	set minutes ""
	set seconds ""
	regexp {([0-9.]*)([^0-9.]*([0-9.]*)([^0-9.]*([0-9.]*))?)?} $coord all degrees sepminsec minutes sepsec seconds
	
	set degrees [string trimleft $degrees 0]
	set minutes [string trimleft $minutes 0]
	set seconds [string trimleft $seconds 0]
	if {$degrees == ""} {
		set degrees 0
	}
	if {$minutes == ""} {
		set minutes 0
	}
	if {$seconds == ""} {
		set seconds 0
	}
	set coordVal [expr $degrees + ($minutes / 60.0) + ($seconds / 3600.0)]
	if {$dir == "N" || $dir == "E"} {
		return $coordVal
	} else {
		return -$coordVal
	}
}

#----------------------------------
proc moveToward {target} {
	global featureObject speedFactor toglPath northUpMode
	global mostRecentFeature borderColorNormal borderColorHighlighted

	setInteractivity motion
	
	#Set up the attractors
	gz tellObject .Animator.mover base $target
	gz tellObject .Animator.mover speed [expr $speedFactor * 0.5 / ([gz posable get Window.main s] * ([gz posable get Window.main p] + 4))]
	gz tellObject .Animator.mover effect xy
	gz tellObject .Animator.mover activate
	
	if {$target != ".Animator.direction"} {
		set dx [expr [gz posable get $target x] - [gz posable get Window.main x]]
		set dy [expr [gz posable get $target y] - [gz posable get Window.main y]]
		eval "gz posable set .Animator.direction xyz [gz posable get $target xy] [gz posable get Window.main z]"
	} else {
		#---Orient from Center-of-screen to selected point---
		#foreach {a b c d width} [$toglPath(main) configure -width] {}
		#foreach {a b c d height} [$toglPath(main) configure -height] {}
		#foreach {x y z} [$toglPath(main) plotPoint screen [expr $width / 2.0] [expr $height / 2.0] y 0] {}
		#---Orient from eyepoint to selected point---
		foreach {x y z} [$toglPath(main) getEyepoint] {}
		set dx [expr [gz posable get $target x] - $x]
		set dy [expr [gz posable get $target y] - $y]
		#---Orient from center of workspace to selected point---
		#set dx [expr [gz posable get $target x] - [gz posable get Window.main x]]
		#set dy [expr [gz posable get $target y] - [gz posable get Window.main y]]
	}
	set newHeading [expr atan2($dx, $dy) * 180 / 3.1415927]
	set oldHeading [gz posable get Window.main h]
	set speed [expr abs($oldHeading - $newHeading)]
	while {$speed > 180} {
		set speed [expr $speed - 360]
	}
	set speed [expr $speedFactor * abs($speed)/3]
	if {$northUpMode == 0} {
		gz posable set .Animator.direction h $newHeading
		gz tellObject .Animator.turner speed $speed
		gz tellObject .Animator.turner activate
	}
	
	#Set up a proximity alert to shut off animator when close enough
	if {$mostRecentFeature != ""} {
		alterScreenGroup $mostRecentFeature 0
	}
	alterScreenGroup $target 1
	set mostRecentFeature $target
	gz tellObject rangeFinder manage $target
	if {[gz tellObject $target objectType] != "Feature" || [gz tellObject $target currentChild objectType] != "TextureScreen"} {
		set distance [expr 1.0 + $speed]
	} else {
		set distance [expr 1.5 * [gz tellObject $target currentChild screenWidth]]
		set temp [expr 3 * [gz tellObject $target currentChild screenHeight]]
		if {$distance < $temp} {
			set distance $temp
		}
	}
	gz tellObject rangeFinder threshold $distance
}

global mostRecentFeature
set mostRecentFeature ""

#----------------------------------
proc rangeReached {} {
	global mostRecentFeature

	#stop moving!
	if {$mostRecentFeature != "" && [gz tellObject rangeFinder numManaging] > 2} {
		gz tellObject rangeFinder relinquish $mostRecentFeature
	}
	gz tellObject .Animator.mover activate 0
	gz tellObject .Animator.turner activate 0

	setInteractivity base
}

#----------------------------------
proc alterScreenGroup {target highlight} {
	global borderColorHighlighted borderColorNormal
	set groupParent ""
	if {$target == ".Animator.direction"} return
	if {$target != ""} {
		set type [gz tellObject $target objectType]
		if {[string range $type 0 2] == "S57"} return
	}
	if {$target != "" && [gz tellObject $target objectType] != "Feature"} {
		regexp {^(.*)\.[^\.]*} [gz tellObject $target getName] all groupParent
	} else {
		set groupParent $target
	}
	if {$groupParent != ""} {
		foreach child [gz address list $groupParent] {
			if {$highlight} {
				eval "gz tellObject $child borderColor $borderColorHighlighted"
				gz tellObject $child borderWidth 4
			} else {
				eval "gz tellObject $child borderColor $borderColorNormal"
				gz tellObject $child borderWidth 2
			}
		}
	}
	
}

#----------------------------------
proc setSelectedFeature {object} {
	global infoFeature

	set objectType [gz tellObject $object objectType]
	if {$objectType == "Gutm"} {
		set object ""
	}

	if {$objectType == "S57_supplemental"} {
		set object [gz tellObject $object getParentS57]
	}

	if {$object == ""} {
		gz tellObject land select 0
		puts "Zero"
	} else {
		gz tellObject $object select 1
		after 40 "gz tellObject $object select 1"; #Something resets it... so we need to be diligent!
	}

	set objectEssence ""
	regexp {\.([^\.]*$)} $object all objectEssence
	if {$objectEssence == ""} {
		set infoFeature ""
		clearObjectInfo
	} else {
		set infoFeature [regsub -all "_" $objectEssence " "]
		if {[string range $objectType 0 2] == "S57"} {
			clearObjectInfo
			gz tellObject $object info
		}
	}
}

#----------------------------------
proc doubleClicked {x y} {
	global toglPath mostRecentFeature

	if {$mostRecentFeature != ""} {
		set result ""
		catch "set result \[gz tellObject $mostRecentFeature objectType\]"
		if {[regexp ".*_area" $result] == 0} {
			moveToward $mostRecentFeature
		}
	} else {
		foreach {wx wy wz} [$toglPath(main) plotPoint screen $x $y -1] {
			gz posable set .Animator.direction xyz $wx $wy $wz
			moveToward .Animator.direction	
		}
	}
}

#----------------------------------
proc displayInfoFor {features {fullyQualified 0}} {
	global infoFeature
	clearObjectInfo
	if {$fullyQualified} {
		if {![catch "gz tellObject $features info"]} {
			set name [string range $features [expr [string last "." $features] + 1] end]
			set infoFeature [regsub -all "_" $name " "]
			return; #return on first one that gets it working!
		}
	}
	foreach feature $features {
		#If "Lighted_Range" appears, then the actual name in S57 probably doesn't have "Lighted" there
		set feature [regsub -nocase "lighted_range" $feature "Range"]
		#If dots appear, then the actual name in S57 will have underscores "U.S." --> "U_S_", etc...
		set feature [regsub -all "\[.\]" $feature "_"]
		#If "Bridge" appears, it may have other things between...
		set feature [regsub -nocase "_bridge" $feature "*Bridge"]
		#If "highway" appears, it may not be in the name...
		set feature [regsub -nocase "_highway" $feature "*"]
		if {![catch "gz tellObject #{S57}...*$feature* info"]} {
			set fullname [gz tellObject #{S57}...*$feature* getName]
			set name [string range $fullname [expr [string last "." $fullname] + 1] end]
			set infoFeature [regsub -all "_" $name " "]
			return; #return on first one that gets it working!
		}
	}
	puts "Unable to locate information for $feature"
}