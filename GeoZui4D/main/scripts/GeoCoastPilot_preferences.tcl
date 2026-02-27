#-----------------------------------------------------------------------------
# GeoCoastPilot's "Edit GeoCoastPilot Preferences" Dialog
#-----------------------------------------------------------------------------

global coastPilot
global speedFactor standardEmphasisFactor
global configVariables
global keelDepth cautionDepth
global refreshCFRs
global varList

set varList {}

lappend configVariables keelDepth cautionDepth tideDataURL refreshCFRs
lappend configVariables speedFactor standardEmphasisFactor

toplevel .preferences
wm title .preferences "Edit GeoCoastPilot Preferences"
wm protocol .preferences WM_DELETE_WINDOW {wm withdraw .preferences}

set fields {"Startup Options"								section	{
				"Automatically download latest CFRs on startup"	bool	refreshCFRs
			}
			"Adjustable Contour Levels"						section {
				"Keel contour depth (at MLLW)"					number	keelDepth
				"Caution contour depth (at MLLW)"				number	cautionDepth
			}
			"3D User Interface Options"						section {
				"Animation speed (default 100%)"			percent		speedFactor
				"Feature exaggeration (default 0.55)"		number		standardEmphasisFactor
			}
}

#--------------------------------------------------------------
proc newFields {fields {prepend {}}} {
	global varList defaultFont
	
	set sectionNum 0
	set fieldFrame $prepend.fields
	frame $fieldFrame

	frame $fieldFrame.labels
	frame $fieldFrame.entries
	foreach {label type trueVar} $fields {
		if {$type == "section"} {
			set name $fieldFrame.entries.section$sectionNum
			frame $name -borderwidth 2 -relief ridge
			set sectionFrame [newFields $trueVar $name]
			label $name.sectionLabel -text $label -anchor w -justify left -font "$defaultFont bold"
			pack $name.sectionLabel -side top -expand no -fill x
			pack $sectionFrame -side top -fill both -expand yes
			pack $name -side top -fill both -expand yes
			incr sectionNum
			continue
		}
		global $trueVar
		if {$type == "percent"} {
			set baseVar $trueVar
			set trueVar percent$baseVar
			global $trueVar
			eval "set $trueVar [expr \$$baseVar * 100]"
			trace variable $trueVar w "updatePercentVar $baseVar"
		}
		lappend varList $trueVar
		set var "pref$trueVar"
		global $var
		eval "set $var \$$trueVar"
		label $fieldFrame.labels.label_$var -text $label -anchor e -justify right
		pack $fieldFrame.labels.label_$var -fill x -expand yes -pady 4
		if {$type == "file"} {
			frame $fieldFrame.entries.file_$var -height 1
			entry $fieldFrame.entries.file_$var.entry -textvariable $var -width 50
			button $fieldFrame.entries.file_$var.browse -text "Browse..." -command "browseCommand $var"
			pack $fieldFrame.entries.file_$var.entry -side left -fill x -expand yes
			pack $fieldFrame.entries.file_$var.browse -side left -padx 4
			pack $fieldFrame.entries.file_$var -fill x -expand yes -padx 2
		} elseif {$type == "bool"} {
			checkbutton $fieldFrame.entries.b_$var -variable $var -text "" -anchor w -height 1
			pack $fieldFrame.entries.b_$var -fill x -expand yes -pady 4
		} elseif {$type == "radio"} {
			frame $fieldFrame.entries.radio_$var -height 1
			foreach option $value {
				radiobutton $fieldFrame.entries.radio_$var.op_$option -variable $var -value $option -text $option
				pack $fieldFrame.entries.radio_$var.op_$option -fill x -expand yes -padx 2 -side left
			}
			pack $fieldFrame.entries.radio_$var -fill x -expand yes -padx 2 -pady 4
		} else {
			entry $fieldFrame.entries.e_$var -textvariable $var -width 8
			pack $fieldFrame.entries.e_$var -fill x -expand yes -padx 2 -pady 4
		}
	}
	pack $fieldFrame.labels -side left
	pack $fieldFrame.entries -side left -fill x -expand yes

	return $fieldFrame
}

#--------------------------------------------------------------
proc browseCommand {var} {
	global $var
	global datadir

	set retval [tk_getOpenFile -initialdir $datadir]
	if {$retval != ""} {set $var $retval}
}

#--------------------------------------------------------------
proc updatePercentVar {base {name1 {}} {name2 {}} {op w}} {
	global $base $name1
	
	set $base [eval "expr \$$name1 / 100"]
}


#--------------------------------------------------------------
proc applyValues {} {
	global varList
	
	foreach var $varList {
		set prefVarName pref$var
		global $var $prefVarName
		eval "set $var \$$prefVarName"
	}
}

#--------------------------------------------------------------
proc restoreValues {} {
	global varList
	
	foreach var $varList {
		set prefVarName pref$var
		global $var $prefVarName
		eval "set $prefVarName \$$var"
	}
}

set fieldFrame [newFields $fields .preferences]
pack $fieldFrame -fill both -expand yes

button .preferences.cancel -text "Cancel" -command "restoreValues; wm withdraw .preferences"
pack .preferences.cancel -side left -fill x -expand yes -pady 2 -padx 2
button .preferences.apply -text "Apply" -command "applyValues"
pack .preferences.apply -side left -fill x -expand yes -pady 2 -padx 2
button .preferences.ok -text "OK" -command "applyValues; wm withdraw .preferences"
pack .preferences.ok -side left -fill x -expand yes -pady 2 -padx 2

wm withdraw .preferences