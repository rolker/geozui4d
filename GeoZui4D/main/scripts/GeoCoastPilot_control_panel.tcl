#-----------------------------------------------------------------------------
# GeoCoastPilot Control Panel GUI setup
#-----------------------------------------------------------------------------

global minLat maxLat minLon maxLon
global coastPilot
global maxScale minScale
global interactivityBase interactivityMotion
global interactivityMotionRequests
global speedFactor standardEmphasisFactor
global northUpMode
global configVariables
global keelDepth cautionDepth

#----------------------------------------------------------------------
#---GeoCoastPilot Control Panel---
set rightSidePanel [frame .pane.f1.frame.rightSide -width 250]
set gcpControlPanel [frame $rightSidePanel.gcpControlPanel -width 250]
pack $gcpControlPanel -side top -fill both -expand true
pack $rightSidePanel -side right -fill both -expand false

#We're hijacking the infoFrame variable here...
set infoFrame [frame $gcpControlPanel.viewParameters -borderwidth 2 -relief sunken]

#								 depth " Depth" " 0000.0m"
#								 distance " Distance" " 000.0nm"
#								 bearing " Bearing" " 000\xb0"
#								 rBearing " Rel. Bearing" " 000\xb0"
#								 etr " ETR" " 0:00:00"

#----Position indicator(s)----
label $infoFrame.latLon -justify left -anchor w -font "$defaultFont bold"
pack $infoFrame.latLon -side top -fill x -pady 1

set latLonFrame [frame $infoFrame.latLonFrame]
set latLonEntry [frame $latLonFrame.entries]

foreach {name varname labelval width default} {latitude latText "Lat" 10 " 00\xb0\x30\x30'00.00\"N"
											   longitude lonText "Lon " 11 " 00\xb0\x30\x30'00.00\"E"
											  } {
	global $varname
	frame $latLonEntry.$name
	label $latLonEntry.$name.label -anchor e -justify left -text $labelval
	entry $latLonEntry.$name.value -justify left -width $width -textvariable $varname
	pack $latLonEntry.$name.label -side left -expand false
	pack $latLonEntry.$name.value -side left -expand true
	pack $latLonEntry.$name -side left -expand true -fill x -padx 2 -pady 1
	bind $latLonEntry.$name.value <Return> setPosLatLon
}
pack $latLonEntry -side left -fill y -expand true

pack $latLonFrame -side top -fill x -expand true
#pack $latLonFrame -side left -fill x

proc updateLatLon {latD latStr lonD lonStr lat lon} {
	global infoFrame
	global latText lonText

	#set newLat [format "%02d%02d'%05.2f\"%c" $latD $latM $latS $latDir]
	#set newLon [format "%03d%02d'%05.2f\"%c" $lonD $lonM $lonS $lonDir]
	set latD [expr abs($latD)]
	set lonD [expr abs($lonD)]
	set newLat [format "%02d\xb0%s" $latD $latStr]
	set newLon [format "%03d\xb0%s" $lonD $lonStr]
	$infoFrame.latLon configure -text "$newLat   $newLon"
	set latText $lat
	set lonText $lon

#	$infoFrame.latitude.value configure -text $newLat
#	$infoFrame.longitude.value configure -text $newLon
}

proc setPosLatLon {} {
	global latText lonText headingText pitchText currentProjection
	global minLat maxLat minLon maxLon
	set x ""
	set y ""

	if {$latText < $minLat} {
		set latText $minLat
	} elseif {$latText > $maxLat} {
		set latText $maxLat
	}
	if {$lonText < $minLon} {
		set lonText $minLon
	} elseif {$lonText > $maxLon} {
		set lonText $maxLon
	}
	foreach {x y} [gz projection apply $currentProjection $lonText $latText] {}
	if {$pitchText(main) < 0 || $pitchText(main) > 90} {
		set pitch [gz posable get Window.main p]
	} else {
		set pitch $pitchText(main)
	}
	set x [expr $x + 0.0000001]; #There's some problem with the display algorithm, so we'll move a little to compensate
	set y [expr $y - 0.0000001]; #There's some problem with the display algorithm, so we'll move a little to compensate
	gz posable set Window.main xyhp $x $y $headingText(main) $pitch
}


#----Rotation indicator(s)----
frame $infoFrame.rotation
label $infoFrame.rotation.headingLabel -text "Heading"
entry $infoFrame.rotation.headingEntry -textvariable headingText(main) -width 6
label $infoFrame.rotation.pitchLabel -text "Pitch"
entry $infoFrame.rotation.pitchEntry -textvariable pitchText(main) -width 5
pack $infoFrame.rotation.headingLabel -side left
pack $infoFrame.rotation.headingEntry -side left -expand yes -pady 2 -padx 1
pack $infoFrame.rotation.pitchLabel -side left
pack $infoFrame.rotation.pitchEntry -side left -expand yes -pady 2 -padx 1
pack $infoFrame.rotation -pady 2

button $infoFrame.rotation.goto -text "Goto" -command setPosLatLon
pack $infoFrame.rotation.goto -side left -expand true -fill x -padx 2

#pack $infoFrame -fill x -pady 2 -padx 1
pack $infoFrame -side right -fill x -pady 2 -padx 1
bind $infoFrame.rotation.pitchEntry <Return> setPosLatLon
bind $infoFrame.rotation.headingEntry <Return> setPosLatLon

#---Navigation buttons---
set navs [frame $infoFrame.navButtons]
button $navs.home -text "Home" -command goHome
button $navs.aerial -text "Aerial" -command goAerial
checkbutton $navs.northUp -indicatoron false -text "NorthUp" -variable northUpMode -command {goNorthUp} -selectcolor "#CCAAAA"
button $navs.overview -text "Overview" -command goOverview
pack $navs.home -side left -padx 2 -expand yes -fill x
pack $navs.aerial -side left -padx 2 -expand yes -fill x
pack $navs.overview -side left -padx 2 -expand yes -fill x
pack $navs.northUp -side left -padx 2 -expand yes -fill x
pack $navs -side top -pady 1 -expand yes -fill x

proc goHome {} {
	global mainWindow
	
	$mainWindow keyboard -1 -1 0 h 0 h
	updateScale
}

proc goAerial {} {
	set oldPitch [gz posable get Window.main p]
	gz posable set Window.main p 28
	gz posable offset Window.main s [expr $oldPitch/28]
}

proc goNorthUp {} {
	global northUpMode
	if {$northUpMode == 1} {
		rangeReached
		gz posable set Window.main h 0
		gz posable set Window.main mh 0
		gz posable set Window.main Mh 0
	} else {
		gz posable unset Window.main Mh
		gz posable unset Window.main mh
	}
}

proc goOverview {} {
	set oldPitch [gz posable get Window.main p]
	gz posable set Window.main p 90
	gz posable offset Window.main s [expr $oldPitch/90]
}

global widgetsOn
set widgetsOn 1
checkbutton $infoFrame.displayWidgets -indicatoron false -text "Show Rotation & Measurement Tools" -variable widgetsOn -command {gz widgets $widgetsOn} -selectcolor "#CCAAAA"
pack $infoFrame.displayWidgets -fill x -pady 2 -side top

#---Textual Search panel---
global searchText previousSearchText searchIndex priorEndLoc
set previousSearchText ""
set priorEndLoc ""
set searchIndex 1.0
set searchFrame [frame $gcpControlPanel.search -borderwidth 2 -relief sunken]
entry $searchFrame.entry -textvariable searchText
button $searchFrame.findback -text "<" -command "searchForText 0"
button $searchFrame.findforward -text "Find  >" -command "searchForText 1"
pack $searchFrame.entry -side left -expand yes -fill x -pady 2 -padx 1
pack $searchFrame.findback -side left -pady 2 -padx 1
pack $searchFrame.findforward -side left -pady 2 -padx 1
#pack $searchFrame -side bottom -fill x -pady 2 -padx 1
pack $searchFrame -side top -fill x -pady 2 -padx 1
bind $searchFrame.entry <Return> {searchForText 1}

proc searchForText {searchingForward} {
	global coastPilot
	global searchText previousSearchText searchIndex priorEndLoc
	
	if {$searchText == ""} {
		set searchText $previousSearchText
		if {$searchText == ""} return;
	}
	
	set direction "-backwards"
	if {$searchingForward} {
		set direction "-forwards"
	}

	if {$priorEndLoc != ""} {
		$coastPilot tag remove searchResult $searchIndex $priorEndLoc
		set priorEndLoc ""
	}	
	if {$searchText == $previousSearchText} {
		if {$searchingForward} {
			set searchIndex "$searchIndex + 1 chars"
		} else {
			set searchIndex "$searchIndex - 1 chars"
		}
	} else {
		set searchIndex [$coastPilot index "@0,0"]
		set previousSearchText $searchText
	}
	set newLoc [$coastPilot search -nocase $direction -- $searchText $searchIndex]
	if {$newLoc == ""} {
		puts "\07"
		return
	}
	$coastPilot see $newLoc
	set priorEndLoc "$newLoc + [string length $searchText] chars"
	$coastPilot tag add searchResult $newLoc $priorEndLoc
	$coastPilot tag raise searchResult
	set searchIndex $newLoc
}

proc tagText {searchText tag} {
	global coastPilot
	
	set searchIndex [$coastPilot index "@0,0"]
	set newLoc [$coastPilot search -nocase -- $searchText $searchIndex end]
	while {$newLoc != ""} {
		$coastPilot tag add $tag $newLoc "$newLoc + [string length $searchText] chars"
		set searchIndex "$newLoc + 1 chars"
		set newLoc [$coastPilot search -nocase -- $searchText $searchIndex end]
	}
}

#---Display Buttons---
set displayFrame [frame $gcpControlPanel.display -borderwidth 2 -relief sunken -height 0]

global showChart
set showChart 0
set chartFrame [frame $displayFrame.chart]
label $chartFrame.label -text "Base Display:" -anchor w -justify left
radiobutton $chartFrame.chart -text "Nautical Chart" -variable showChart -value 1 -command chartOrBathy
radiobutton $chartFrame.bathy -text "Bathymetry" -variable showChart -value 0 -command chartOrBathy
proc chartOrBathy {} {
	global showChart
	
	if {$showChart} {
		. configure -cursor wait
		update
		gz tellObject bathy hide
		gz tellObject land hide
		gz tellObject chart show
		. configure -cursor ""
	} else {
		gz tellObject chart hide
		gz tellObject bathy show
		gz tellObject land show
	}
}
pack $chartFrame.label -side top -fill x -padx 1
pack $chartFrame.chart -side left -fill x -pady 2 -padx 1
pack $chartFrame.bathy -side left -fill x -pady 2 -padx 1
pack $chartFrame -side top -fill x

global emphasisFactor
set emphasisFactor $standardEmphasisFactor
set featureFrame [frame $displayFrame.feature]
trace variable standardEmphasisFactor w "$featureFrame.exaggerated configure -value \$standardEmphasisFactor; $featureFrame.exaggerated invoke ; #"
label $featureFrame.label -text "Display key features:" -anchor w -justify left
radiobutton $featureFrame.actual -text "Actual size" -variable emphasisFactor -value 0.0 -command featureEmphasis
radiobutton $featureFrame.exaggerated -text "Exaggerated" -variable emphasisFactor -value $standardEmphasisFactor -command featureEmphasis
proc featureEmphasis {} {
	global emphasisObject emphasisFactor
	gz tellObject $emphasisObject emphasisFactor $emphasisFactor
}
pack $featureFrame.label -side top -fill x -padx 1
pack $featureFrame.actual -side left -fill x -pady 2 -padx 1
pack $featureFrame.exaggerated -side left -fill x -pady 2 -padx 1
pack $featureFrame -side top -fill x

pack $displayFrame -side top -pady 2 -fill x -padx 1

#---Feature Info Textual Panel---
global objectInfo
set featureInfoFrame [frame $rightSidePanel.featureInfoFrame -borderwidth 2 -relief sunken]
set featureInfo [text $featureInfoFrame.featureInfo -wrap word -cursor arrow -width 20]
set fiScroll [scrollbar $featureInfoFrame.scroll -command "$featureInfo yview"]
$featureInfo configure  -yscrollcommand "$fiScroll set"
set objectInfo $featureInfo

global infoFeature
set infoFeature ""
set featureLabelFrame [frame $featureInfoFrame.featureLabelFrame]
label $featureLabelFrame.label -text "Feature: "
pack $featureLabelFrame.label -side left -expand no
label $featureLabelFrame.name -textvariable infoFeature -width 34 -font "$defaultFont bold" -justify left -anchor w
pack $featureLabelFrame.name -side left -expand yes -fill x -padx 2
pack $featureLabelFrame -side top -expand yes -fill x -pady 2 -padx 1

pack $fiScroll -fill y -expand no -side right
pack $featureInfo -fill both -expand yes -side right
pack $featureInfoFrame -side top -expand yes -fill both -padx 1

$featureInfo configure -state disabled
