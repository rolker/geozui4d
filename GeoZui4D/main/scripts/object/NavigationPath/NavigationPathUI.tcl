lappend auto_path ../../../BWidget-1.4.1
package require BWidget

toplevel .navPath
frame .navPath.defaultsFrame
pack .navPath.defaultsFrame -expand false -fill x
frame .navPath.defaultsFrame.corridorFrame
pack .navPath.defaultsFrame.corridorFrame -expand true -fill x
label .navPath.defaultsFrame.corridorFrame.label -text "Corridor width:"
pack .navPath.defaultsFrame.corridorFrame.label -side left
label .navPath.defaultsFrame.corridorFrame.value -textvariable corridorWidth
pack .navPath.defaultsFrame.corridorFrame.value -side left
scale .navPath.defaultsFrame.corridorFrame.scale -from 1 -orient horizontal -showvalue 0 -to 250 -variable corridorWidth
pack .navPath.defaultsFrame.corridorFrame.scale -expand true -fill x -side left
frame .navPath.defaultsFrame.speedFrame
pack .navPath.defaultsFrame.speedFrame -fill x
label .navPath.defaultsFrame.speedFrame.label -text "Default speed:"
pack .navPath.defaultsFrame.speedFrame.label -side left
label .navPath.defaultsFrame.speedFrame.value -textvariable defaultSpeed
pack .navPath.defaultsFrame.speedFrame.value -side left
scale .navPath.defaultsFrame.speedFrame.scale -from 0.1 -orient horizontal -resolution 0.1 -showvalue 0 -to 75 -variable defaultSpeed
pack .navPath.defaultsFrame.speedFrame.scale -expand true -fill x -side left
frame .navPath.startTimeFrame
pack .navPath.startTimeFrame -fill x -side top
label .navPath.startTimeFrame.label -text "Start time:"
pack .navPath.startTimeFrame.label -side left
label .navPath.startTimeFrame.value -textvariable navStartTimeStr
pack .navPath.startTimeFrame.value -side left
SpinBox .navPath.startTimeFrame.hour -width 2
pack .navPath.startTimeFrame.hour -side left
SpinBox .navPath.startTimeFrame.minute -width 2
pack .navPath.startTimeFrame.minute -side left
scale .navPath.startTimeFrame.scale -from -720 -orient horizontal -showvalue 0 -to 720 -variable navStartTimeOffset
pack .navPath.startTimeFrame.scale -expand true -fill x -side left
checkbutton .navPath.startTimeFrame.lockBox -text Lock -variable startTimeLock
pack .navPath.startTimeFrame.lockBox -side left
frame .navPath.endTimeFrame
pack .navPath.endTimeFrame -fill x -side top
label .navPath.endTimeFrame.label -text "Arrival time:"
pack .navPath.endTimeFrame.label -side left
label .navPath.endTimeFrame.value -textvariable navEndTimeStr
pack .navPath.endTimeFrame.value -side left
SpinBox .navPath.endTimeFrame.hour -width 2
pack .navPath.endTimeFrame.hour -side left
SpinBox .navPath.endTimeFrame.minute -width 2
pack .navPath.endTimeFrame.minute -side left
scale .navPath.endTimeFrame.scale -from -720 -orient horizontal -showvalue 0 -to 720 -variable navEndTimeOffset
pack .navPath.endTimeFrame.scale -expand true -fill x -side left
checkbutton .navPath.endTimeFrame.lockBox -text Lock -variable endTimeLock
pack .navPath.endTimeFrame.lockBox -side left
frame .navPath.distanceFrame
pack .navPath.distanceFrame -fill x
label .navPath.distanceFrame.label -text "Distance:"
pack .navPath.distanceFrame.label -side left
label .navPath.distanceFrame.value -textvariable totalDistance
pack .navPath.distanceFrame.value -side left
frame .navPath.legs -borderwidth 1 -relief groove
pack .navPath.legs -expand true -fill both
label .navPath.legs.label -text Legs
pack .navPath.legs.label -anchor nw
listbox .navPath.legs.legsList -height 3
pack .navPath.legs.legsList -expand true -fill both
frame .navPath.legs.startTimeFrame
pack .navPath.legs.startTimeFrame -expand false -fill x -side top
label .navPath.legs.startTimeFrame.label -text "Start time:"
pack .navPath.legs.startTimeFrame.label -side left
SpinBox .navPath.legs.startTimeFrame.hour -width 2
pack .navPath.legs.startTimeFrame.hour -side left
SpinBox .navPath.legs.startTimeFrame.minute -width 2
pack .navPath.legs.startTimeFrame.minute -side left
scale .navPath.legs.startTimeFrame.scale -orient horizontal -showvalue 0
pack .navPath.legs.startTimeFrame.scale -expand true -fill x -side left
checkbutton .navPath.legs.startTimeFrame.lockBox -text Lock
pack .navPath.legs.startTimeFrame.lockBox -side left
frame .navPath.legs.endTimeFrame
pack .navPath.legs.endTimeFrame -expand false -fill x -side top
label .navPath.legs.endTimeFrame.label -text "End time:"
pack .navPath.legs.endTimeFrame.label -side left
SpinBox .navPath.legs.endTimeFrame.hour -width 2
pack .navPath.legs.endTimeFrame.hour -side left
SpinBox .navPath.legs.endTimeFrame.minute -width 2
pack .navPath.legs.endTimeFrame.minute -side left
scale .navPath.legs.endTimeFrame.scale -orient horizontal -showvalue 0
pack .navPath.legs.endTimeFrame.scale -expand true -fill x -side left
checkbutton .navPath.legs.endTimeFrame.lockBox -text Lock
pack .navPath.legs.endTimeFrame.lockBox -side left
frame .navPath.legs.speedFrame
pack .navPath.legs.speedFrame -expand false -fill x -side top
label .navPath.legs.speedFrame.label -text "Speed:"
pack .navPath.legs.speedFrame.label -side left
label .navPath.legs.speedFrame.value -text 6.8
pack .navPath.legs.speedFrame.value -side left
scale .navPath.legs.speedFrame.scale -orient horizontal -showvalue 0
pack .navPath.legs.speedFrame.scale -expand true -fill x -side left
checkbutton .navPath.legs.speedFrame.lockBox -text Lock
pack .navPath.legs.speedFrame.lockBox -side left
frame .navPath.legs.distanceFrame
pack .navPath.legs.distanceFrame -fill x
label .navPath.legs.distanceFrame.label -text "Distance:"
pack .navPath.legs.distanceFrame.label -side left
label .navPath.legs.distanceFrame.value -textvariable legDistance
pack .navPath.legs.distanceFrame.value -side left

