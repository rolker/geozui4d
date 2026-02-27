#------------------------------------------------------------------------------
# scriptSupport.tcl -- supporting procedures for GeoZui3D script windows
#
# This should be included by all GeoZui3D scripts.  It will only define
# the necessary procedures once, for all.
#
# Created by mdp 11/29/2001
#------------------------------------------------------------------------------

if {[info exists scriptInstance] == 0} {

	global scriptInstance scriptSupportVersion
	set scriptInstance 0
	set scriptSupportVersion 1.0

	#--------------------------------------------------------------
	proc script {name code} {
		global scriptInstance scriptSupportVersion

		set internalCode "variable scriptName $name$scriptInstance; incr ::scriptInstance; $code"
		namespace eval $name$scriptInstance $internalCode
	}

	#--------------------------------------------------------------
	proc browseCommand {var} {
		variable $var
		global datadir

		set retval [tk_getOpenFile -initialdir $datadir]
		if {$retval != ""} {set $var $retval}
	}

	#--------------------------------------------------------------
	proc conversionToplevel {fields namespace title} {

		toplevel .$namespace
		wm title .$namespace $title

		set fieldFrame [fieldFrame $fields $namespace]
		pack $fieldFrame -fill both -expand yes
		bind $fieldFrame <Destroy> "namespace delete $namespace"

		button .$namespace.convert -text "Convert" -command ".$namespace.convert configure -state disabled; $namespace\:\:convert;	.$namespace.convert configure -state normal"
		pack .$namespace.convert -fill x -expand yes -pady 2
		label .$namespace.status -text "Ready" -borderwidth 2 -relief sunken -justify left -anchor w
		pack .$namespace.status -fill x -expand yes -pady 2
	}

	#--------------------------------------------------------------
	proc parameterToplevel {fields namespace title {message {}}} {
		variable paramToplevelContinue
		toplevel .param$namespace
		wm title .param$namespace $title

		if {$message != {}} {
			label .param$namespace.message -text $message -wraplength 400
			pack .param$namespace.message
		}
		set fieldFrame [fieldFrame $fields $namespace param]
		pack $fieldFrame -fill both -expand yes

		set paramToplevelContinue 0
		button .param$namespace.convert -text "Done" -command "set paramToplevelContinue 1; destroy .param$namespace"
		pack .param$namespace.convert -fill x -expand yes -pady 2

		vwait paramToplevelContinue
	}

	#--------------------------------------------------------------
	proc fieldFrame {fields namespace {prepend {}}} {
		set fieldFrame .$prepend$namespace.fields
		frame $fieldFrame

		frame $fieldFrame.labels
		frame $fieldFrame.entries
		foreach {var value label type} $fields {
			variable $namespace\:\:$var $value
			label $fieldFrame.labels.label_$var -text $label -anchor e -justify right
			pack $fieldFrame.labels.label_$var -fill x -expand yes -pady 4
			if {$type == "file"} {
				frame $fieldFrame.entries.file_$var -height 1
				entry $fieldFrame.entries.file_$var.entry -textvariable $namespace\:\:$var -width 50
				button $fieldFrame.entries.file_$var.browse -text "Browse..." -command "browseCommand $namespace\:\:$var"
				pack $fieldFrame.entries.file_$var.entry -side left -fill x -expand yes
				pack $fieldFrame.entries.file_$var.browse -side left -padx 4
				pack $fieldFrame.entries.file_$var -fill x -expand yes -padx 2
			} elseif {$type == "bool"} {
				checkbutton $fieldFrame.entries.b_$var -variable $namespace\:\:$var -text "" -anchor w -height 1
				pack $fieldFrame.entries.b_$var -fill x -expand yes -pady 4
			} elseif {$type == "radio"} {
				frame $fieldFrame.entries.radio_$var -height 1
				foreach option $value {
					radiobutton $fieldFrame.entries.radio_$var.op_$option -variable $namespace\:\:$var -value $option -text $option
					pack $fieldFrame.entries.radio_$var.op_$option -fill x -expand yes -padx 2 -side left
				}
				pack $fieldFrame.entries.radio_$var -fill x -expand yes -padx 2 -pady 4
			} else {
				entry $fieldFrame.entries.e_$var -textvariable $namespace\:\:$var
				pack $fieldFrame.entries.e_$var -fill x -expand yes -padx 2 -pady 4
			}
		}
		pack $fieldFrame.labels -side left
		pack $fieldFrame.entries -side left -fill x -expand yes

		return $fieldFrame
	}
}
