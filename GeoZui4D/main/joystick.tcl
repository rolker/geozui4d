#-----------------------------------------------------------------------------
# joystick.tcl -- Support procedures for controlling GeoZui3D's main 3D window
#                 with a joystick.
#-----------------------------------------------------------------------------

namespace eval joystick {

	#joystick variables
	variable joystickSpeed 0
	variable joystickHeadingInertia 0
	variable lastJoystickSpeed 0
	variable buttonScripts
	variable lastButtonPressed
	variable buttonState
	variable pitchFilter
	variable pitchFilterSize 5
	variable pitchFilterIndex 0

	#exported procedures
	namespace export init listStops

	#--------------------------------------------
	# Initializes resources
	#--------------------------------------------
	proc init {} {
		variable buttonScripts
		variable buttonState
		global mainWindow toglPath

		gz address create toplevel Joystick
		gz address create .Joystick Animators
		gz address create .Joystick Internal

		#---joystick handler---
		gz posable create .Joystick.Internal joystickState object
		gz event handler create joystickTracker "joystick::trackJoystick %JA1 %JA2 %JA3 %JB1 %JB2 %JB3 %JB4"
		gz tellObject .EventMediator.joystick push joystickTracker
		for {set i 1} {$i <= 4} {incr i} {
			set buttonScripts($i) {}
			set buttonState($i) 0
		}
		set buttonScripts(any) {}

		#---Vehicle & animators---
		gz posable create .Joystick.Internal proxy object
		gz posable set .Joystick.Internal.proxy zps 60 17 0.001
		#previous 3 lines superseded by next line
		gz animator create .Joystick.Animators moveit VectorAnimator Window.main
		gz tellObject .Joystick.Animators.moveit direction .Joystick.Internal.joystickState
		gz tellObject .Joystick.Animators.moveit speed .Joystick.Internal.joystickState
		gz tellObject .Joystick.Animators.moveit effect hxyz
		gz tellObject .Joystick.Animators.moveit activate 1
	}

	#--------------------------------------------
	# Initializes joystick input
	#--------------------------------------------
	proc initJoystick {} {
			variable pitchFilter
			variable pitchFilterSize
			variable pitchFilterIndex
			for {set i 0} {$i < $pitchFilterSize} {incr i} {
				set pitchFilter($i) 0
			}
			set pitchFilterIndex 0
			gz posable set .Joystick.Internal.joystickState s 0
			activateJoystick
	}

	#--------------------------------------------
	# Handles joystick input
	#--------------------------------------------
	proc trackJoystick {heading speed pitch button1 button2 button3 button4} {
		variable pitchFilter
		variable pitchFilterSize
		variable pitchFilterIndex
		variable buttonScripts
		variable lastButtonPressed
		variable buttonState

		set buttonPressed 0
		for {set i 1} {$i <= 4} {incr i} {
			if "\$button$i == 1 && $buttonState($i) != 1" {
				set buttonState($i) 1
				set buttonPressed $i
				if {$buttonScripts($i) != {}} {
					uplevel #0 $buttonScripts($i)
				}
				set lastButtonPressed $i
				if {$buttonScripts(any) != {}} {
					uplevel #0 $buttonScripts(any)
				}
			}
			if "\$button$i != $buttonState($i)" {
				set buttonState($i) 0; #to get here, button# must be 0.
			}
		}

		variable joystickSpeed
		variable joystickHeadingInertia
		variable bouyancyLevel
		#use 1% of input, average with 98% of intertia (=> 1% dampening)
		if {$button1 == 1 && $button2 == 1 && $button3 == 1 && $button4 == 1} {
			set joystickSpeed [expr 0.01 * ((2000 - $speed) / 500.0) + 0.98 * $joystickSpeed]
		} else {
			set joystickSpeed [expr 0.01 * ((2000 - $speed) / 1500.0) + 0.98 * $joystickSpeed]
		}
		#if {$joystickSpeed < 0} {
		#	set speed [expr -$joystickSpeed]
		#} else {
			set speed $joystickSpeed
		#}
		#use 2% of input (dampened by current speed), average with 97% of intertia (rest is dampening)
		#set joystickHeadingInertia [expr 0.02 * ($heading / 32000.0) / (1 + 2 * sqrt($speed)) + 0.97 * $joystickHeadingInertia]
		#Try not dampening heading by speed...
		#set joystickHeadingInertia [expr 0.02 * ($heading / 32000.0) + 0.97 * $joystickHeadingInertia]
		set joystickHeadingInertia [expr 0.02 * ($heading / 32000.0) + 0.95 * $joystickHeadingInertia]
		gz posable offset .Joystick.Internal.joystickState h $joystickHeadingInertia
		gz posable set .Joystick.Internal.joystickState s $joystickSpeed
		set pitchFilter($pitchFilterIndex) $pitch
		if {[incr pitchFilterIndex] >= $pitchFilterSize} {
			set pitchFilterIndex 0
		}
		set accum 0
		for {set i 0} {$i < $pitchFilterSize} {incr i} {
			set accum [expr $accum + $pitchFilter($i)]
		}
		set pitch [expr $accum / $pitchFilterSize]
		if {$pitch < -5000} {
			set pitch [expr (-5000 - $pitch) / 1000.0]
			gz posable set .Joystick.Internal.joystickState p $pitch
			gz posable set Window.main p [expr 15 + $pitch]
		} elseif {$pitch > 5000} {
			set pitch [expr (5000 - $pitch) / 3000.0]
			gz posable set .Joystick.Internal.joystickState p $pitch
			gz posable set Window.main p [expr 15 + $pitch]
		} else {
			gz posable set .Joystick.Internal.joystickState p 0
			gz posable set Window.main p 15
		}
		# Bouyancy (Rise slower than fall for our purposes):
		#set zval [gz posable get Window.main z]
		#if {$zval < $bouyancyLevel} {
		#	set diff [expr ($bouyancyLevel - $zval) * 0.005]
			#clamp upward bouyancy
		#	if {$diff > 0.1 && $diff < 0.5} {
		#		set diff 0.1
		#	} else {
		#		set diff [expr $diff - 0.4]
		#	}
		#	gz posable offset Window.main z $diff
		#} else {
		#	gz posable offset Window.main z [expr ($bouyancyLevel - $zval) * 0.01]
		#}
	}

	#--------------------------------------------
	# Assigns joystick button behaviors (scripts)
	#--------------------------------------------
	proc assignJoystickButton {button {script {}}} {
		variable buttonScripts

		set buttonScripts($button) "$script" 
	}

	#--------------------------------------------
	# Activates/Deactivates joystick
	#--------------------------------------------
	proc activateJoystick {{value 1}} {
		if {$value == 1} {
			gz posable set .Joystick.Internal.joystickState h [gz posable get Window.main h]
			gz tellObject .EventMediator.joystick continuouslyGenerate axis 1
		} else {
			gz tellObject .EventMediator.joystick continuouslyGenerate axis 0
		}
	}
}

#joystick::init
#joystick::initJoystick