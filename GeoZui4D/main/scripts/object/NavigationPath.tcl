# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

source [file join $appdir "scripts/object/NavigationPath/NavigationPathUI.tcl"]

#toplevel .navPath
#label .navPath.startLabel -text "Start time:"
#pack .navPath.startLabel -side left
#label .navPath.startTime -textvariable navStartTimeStr
#pack .navPath.startTime  -side left
#frame .navPath.buttons
#pack .navPath.buttons -side bottom
#button .navPath.buttons.back -text "back" -command {set navStartTime [expr $navStartTime - 600]}
#pack .navPath.buttons.back -side left
#button .navPath.buttons.fwd -text "fwd" -command {set navStartTime [expr $navStartTime + 600]}
#pack .navPath.buttons.fwd -side left
