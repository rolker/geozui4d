# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#init mission plan gui

namespace eval missionPlanner {

global mpid

#set savedWidth [lindex [.pane.f1 configure -width] end]
#set missionPlanFrame($mpid) [.pane add -weight 0]
set missionPlanFrame($mpid) [$objnbframe.objnb insert end $mpoid -text "$mpid"]

#.pane.f1 configure -width $savedWidth
#.pane.f2 configure -width 200


set nb($mpid) [NoteBook $missionPlanFrame($mpid).notebook]

pack $nb($mpid) -fill both -expand true

set plannerTab($mpid) [$nb($mpid) insert end 0 -text "Mission Planner"]
set toplevelTab($mpid) [$nb($mpid) insert end 1 -text "Mission"]
set commandsTab($mpid) [$nb($mpid) insert end 2 -text "Commands"]

#  Mission planner configuration

checkbutton $plannerTab($mpid).snapAngle -text "Snap to Angle" -variable angleSnap($mpid) -command "gz tellObject $mpid angleSnap \$angleSnap($mpid)"
pack $plannerTab($mpid).snapAngle -fill x

LabelFrame $plannerTab($mpid).angleSpacingFrame -text "Angle Spacing"
ComboBox $plannerTab($mpid).angleSpacingFrame.f.box -values [list 1 5 10 15 25 45 90] -editable true -textvariable angleSpacing($mpid) -command "gz tellObject $mpid angleSpacing \$angleSpacing($mpid)" -modifycmd "gz tellObject $mpid angleSpacing \$angleSpacing($mpid)"
$plannerTab($mpid).angleSpacingFrame.f.box setvalue @1
pack $plannerTab($mpid).angleSpacingFrame -fill x
pack $plannerTab($mpid).angleSpacingFrame.f.box -fill x


checkbutton $plannerTab($mpid).enableGrid -text "Show Grid" -variable gridOn($mpid) -command "gz tellObject $mpid gridEnabled \$gridOn($mpid)"
$plannerTab($mpid).enableGrid select

pack $plannerTab($mpid).enableGrid -fill x

checkbutton $plannerTab($mpid).snapGrid -text "Snap to Grid" -variable gridSnap($mpid) -command "gz tellObject $mpid gridSnap \$gridSnap($mpid)"

pack $plannerTab($mpid).snapGrid -fill x



LabelFrame $plannerTab($mpid).gridSpacingFrame -text "Grid Spacing"
ComboBox $plannerTab($mpid).gridSpacingFrame.f.box -values [list .5 1 2 5 10 20 50 100 500 1000] -editable true -textvariable gridSpacing($mpid) -command "gz tellObject $mpid gridSpacing \$gridSpacing($mpid)" -modifycmd "gz tellObject $mpid gridSpacing \$gridSpacing($mpid)"
$plannerTab($mpid).gridSpacingFrame.f.box setvalue @7
pack $plannerTab($mpid).gridSpacingFrame -fill x
pack $plannerTab($mpid).gridSpacingFrame.f.box -fill x

LabelFrame $plannerTab($mpid).gridSizeFrame -text "Grid Size"
ComboBox $plannerTab($mpid).gridSizeFrame.f.box -values [list 5 10 20 50 100 1000] -editable true -textvariable gridSize($mpid) -command "gz tellObject $mpid gridSize \$gridSize($mpid)" -modifycmd "gz tellObject $mpid gridSize \$gridSize($mpid)"
$plannerTab($mpid).gridSizeFrame.f.box setvalue @4
pack $plannerTab($mpid).gridSizeFrame -fill x
pack $plannerTab($mpid).gridSizeFrame.f.box -fill x

LabelFrame $plannerTab($mpid).gridHeightFrame -text "Grid Height"
ComboBox $plannerTab($mpid).gridHeightFrame.f.box -values [list 0 -1 -2 -5 -10 -20 -50 -100 -200 -500] -editable true -textvariable gridHeight($mpid) -command "gz tellObject $mpid gridHeight \$gridHeight($mpid)" -modifycmd "gz tellObject $mpid gridHeight \$gridHeight($mpid)"
$plannerTab($mpid).gridHeightFrame.f.box setvalue @4
pack $plannerTab($mpid).gridHeightFrame -fill x
pack $plannerTab($mpid).gridHeightFrame.f.box -fill x

LabelFrame $plannerTab($mpid).defaultDepthFrame -text "Default Depth"
ComboBox $plannerTab($mpid).defaultDepthFrame.f.box -values [list 1 -2 -5 -10 -20 -50 -100] -editable true -textvariable defaultDepth($mpid) -command "gz tellObject $mpid defaultDepth \$defaultDepth($mpid)" -modifycmd "gz tellObject $mpid defaultDepth \$defaultDepth($mpid)"
$plannerTab($mpid).defaultDepthFrame.f.box setvalue @3
pack $plannerTab($mpid).defaultDepthFrame -fill x
pack $plannerTab($mpid).defaultDepthFrame.f.box -fill x


# georef and globals section of mission

set toplevelScroll($mpid) [ScrolledWindow $toplevelTab($mpid).toplevelScroll]
pack $toplevelScroll($mpid) -expand true -fill both 
ScrollableFrame $toplevelScroll($mpid).toplevelFrame
set toplevelFrame($mpid) [$toplevelScroll($mpid).toplevelFrame getframe]
$toplevelScroll($mpid) setwidget $toplevelScroll($mpid).toplevelFrame

# command section

set commandPanes($mpid) [PanedWindow $commandsTab($mpid).commandPanes -side left]
pack $commandPanes($mpid) -expand true -fill both

set commandAddPane($mpid) [$commandPanes($mpid) add]
set commandAddScroll($mpid) [ScrolledWindow $commandAddPane($mpid).commandAddScroll]
pack $commandAddScroll($mpid) -expand true -fill both 
ScrollableFrame $commandAddScroll($mpid).commandAddFrame
set commandAddFrame($mpid) [$commandAddScroll($mpid).commandAddFrame getframe]
$commandAddScroll($mpid) setwidget $commandAddScroll($mpid).commandAddFrame

set commandListPane($mpid) [$commandPanes($mpid) add]
set commandListScroll($mpid) [ScrolledWindow $commandListPane($mpid).commandListScroll]
pack $commandListScroll($mpid) -expand true -fill both 
set commandList($mpid) [ListBox $commandListScroll($mpid).commandList -selectfill true -selectmode single -dragenabled true -dropenabled true -dropovermode p -dropcmd "gz tellObject $mpid cmddrag"]
$commandListScroll($mpid) setwidget $commandList($mpid)

set commandDetailPane($mpid) [$commandPanes($mpid) add]
set commandDetails($mpid) [PagesManager $commandDetailPane($mpid).commandDetails]
pack $commandDetails($mpid) -expand true -fill both

bind $commandList($mpid) <<ListboxSelect>> {gz tellObject $mpid cmdSelected [$missionPlanner::commandList($mpid) curselection]}

$nb($mpid) raise 1

}