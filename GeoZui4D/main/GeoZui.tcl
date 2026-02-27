#-----------------------------------------------------------------------------
# GUI setup
#-----------------------------------------------------------------------------

global pane
global menuWin

source [file join $appdir "fullmenu.tcl"]

#--------object frame--------
#frame .objframe

#set objframe [$pane add -weight 0]
global pane
set objnbframe [$pane add -weight 0]
NoteBook $objnbframe.objnb
pack $objnbframe.objnb -fill both -expand true

set objframe [$objnbframe.objnb insert 0 0 -text "Objects"]
$objnbframe.objnb raise 0

frame $objframe.objects -relief groove -borderwidth 2
set objectFrame $objframe.objects

#----Close Frame Button (Done here to ensure that it always shows)----
button $objframe.exitbutton -text "Close Frame" -command {recordAndExecMaybe {global pane; $pane.f1 configure -width 0}} -pady 0
pack $objframe.exitbutton -side bottom -padx 4 -pady 4 -fill x

 
#--------object list--------
label $objectFrame.label -text "Objects" -relief sunken -background "#DDDDDD" -borderwidth 2
pack $objectFrame.label -side top -fill x -padx 2 -pady 2

#----Height Exaggeration frame (Done here to ensure that it always shows)----
frame $objframe.viewframe -relief groove -borderwidth 2
set viewFrame $objframe.viewframe
frame $viewFrame.spacer
pack $viewFrame.spacer -side top -pady 2
label $viewFrame.label -text "Height Exaggeration" -relief sunken -background "#DDDDDD" -borderwidth 2
pack $viewFrame.label -side top -fill x -pady 2 -padx 2

#----Height Exaggeration indicator(s)----
frame $viewFrame.heightExaggeration
set heightExaggeration 1.0
entry $viewFrame.heightExaggeration.entry -textvariable heightExaggeration -width 2
pack $viewFrame.heightExaggeration.entry -side left -expand yes -fill x -pady 2 -padx 2
frame $viewFrame.heightExaggeration.buttons
bind $viewFrame.heightExaggeration.entry <Return> "$viewFrame.heightExaggeration.buttons.use invoke"
#button $viewFrame.heightExaggeration.buttons.test -text "Test" -pady 0 -command {$mainWindow setHeightExaggeration $heightExaggeration}
button $viewFrame.heightExaggeration.buttons.use -text "Apply" -pady 0 -command {$mainWindow useHeightExaggeration $heightExaggeration}
button $viewFrame.heightExaggeration.buttons.reset -text "1.0" -pady 0 -command {$mainWindow useHeightExaggeration 1.0}
#pack $viewFrame.heightExaggeration.buttons.test -side left -padx 2
pack $viewFrame.heightExaggeration.buttons.use -side left -padx 2
pack $viewFrame.heightExaggeration.buttons.reset -side left -padx 2
pack $viewFrame.heightExaggeration.buttons -side left -pady 2
pack $viewFrame.heightExaggeration -fill x

pack $viewFrame -padx 1 -expand no -fill x -side bottom

#----info frame (Done here to ensure that it always shows)----
frame $objframe.infoframe -relief groove -borderwidth 2
set infoFrame $objframe.infoframe
frame $infoFrame.spacer
pack $infoFrame.spacer -side top -pady 2
label $infoFrame.label -text "Information" -relief sunken -background "#DDDDDD" -borderwidth 2
pack $infoFrame.label -side top -fill x -pady 2 -padx 2
set objectVFrame $infoFrame.vertical
set objectHFrame $infoFrame.horizontal
set objectInfo $objectVFrame.text
frame $objectVFrame
frame $objectHFrame
scrollbar $objectVFrame.yscroll -command "$objectInfo yview" -width 16
scrollbar $objectHFrame.xscroll -command "$objectInfo xview" -orient horizontal
text $objectVFrame.text -height 4 -width 10 -wrap none \
 	-yscrollcommand "$objectVFrame.yscroll set" \
	-xscrollcommand "$objectHFrame.xscroll set" \
	-font "courier 8"
pack $objectVFrame.text -side left -expand yes -fill both
pack $objectVFrame.yscroll -side top -expand yes -fill y
pack $objectVFrame -side top -expand yes -fill both -padx 3

pack $objectHFrame.xscroll -side left -expand yes -fill x
label $objectHFrame.filler -width 2
pack $objectHFrame.filler -side left
pack $objectHFrame -side top -fill x -padx 2

#----Projection indicator(s)----
frame $infoFrame.projection
label $infoFrame.projection.label -text "Projection:"
label $infoFrame.projection.name -textvariable currentProjection -justify left -anchor w
pack $infoFrame.projection.label -side left
pack $infoFrame.projection.name -side left -expand yes -fill x -pady 2 -padx 1
pack $infoFrame.projection -fill x

#----Position indicator(s)----
frame $infoFrame.position
label $infoFrame.position.label -text "+"
entry $infoFrame.position.entry -textvariable positionText(main)
pack $infoFrame.position.label -side left
pack $infoFrame.position.entry -side left -expand yes -fill x -pady 2 -padx 1
pack $infoFrame.position -fill x

#----Rotation indicator(s)----
frame $infoFrame.rotation
label $infoFrame.rotation.headingLabel -text "Heading"
entry $infoFrame.rotation.headingEntry -textvariable headingText(main) -width 6
label $infoFrame.rotation.pitchLabel -text "Pitch"
entry $infoFrame.rotation.pitchEntry -textvariable pitchText(main) -width 6
pack $infoFrame.rotation.headingLabel -side left
pack $infoFrame.rotation.headingEntry -side left -expand yes -fill x -pady 2 -padx 1
pack $infoFrame.rotation.pitchLabel -side left
pack $infoFrame.rotation.pitchEntry -side left -expand yes -fill x -pady 2 -padx 1
pack $infoFrame.rotation -fill x

#----Time display----
frame $infoFrame.gztime
label $infoFrame.gztime.timeDisplay -textvariable GZtimestr
pack $infoFrame.gztime.timeDisplay -side left -expand yes -fill x -pady 2 -padx 1
pack $infoFrame.gztime -fill x
ComboBox $infoFrame.gztime.timezone -editable false -values [lsort [array names timezones]] -textvariable timezone -modifycmd {gztime timezone $timezones($timezone)}

pack $infoFrame.gztime.timezone -side right

pack $infoFrame -padx 1 -expand yes -fill both -side bottom

#----Actual object list----
ScrolledWindow $objectFrame.listframe
Tree $objectFrame.listframe.objTree -deltax 6 -height 4
$objectFrame.listframe.objTree bindText <ButtonPress-3> "contextMenu"
set objectTree $objectFrame.listframe.objTree
$objectFrame.listframe setwidget $objectTree

pack $objectFrame.listframe -padx 2 -expand yes -fill both



#----How to do it as a tabbed notebook----
#NoteBook $objectFrame.notebook -height 1 -side bottom -internalborderwidth 0 -tabbevelsize 4
#pack $objectFrame.notebook -padx 2 -expand yes -fill both
#set mainPage [$objectFrame.notebook insert 0 main -text main]
#
#ScrolledWindow $mainPage.listframe
#
#set objectTree [Tree $mainPage.listframe.objTree -deltax 6 -height 4]
#$objectTree bindText <ButtonPress-3> "contextMenu"
#$mainPage.listframe setwidget $objectTree
#pack $objectTree -side left -expand yes -fill both
#
#pack $mainPage.listframe -expand yes -fill both
#$objectFrame.notebook raise main

#----small delete and reload buttons----
frame $objectFrame.delframe
image create bitmap bitmap_delete -file [file join $appdir "bitmaps/delete.bmp"]
button $objectFrame.delframe.delete -image bitmap_delete -command {deleteobj}
image create bitmap bitmap_reload -file [file join $appdir "bitmaps/reload.bmp"]
button $objectFrame.delframe.reload -image bitmap_reload -command {reloadobj}
pack $objectFrame.delframe.delete -side top -padx 2 -pady 2
pack $objectFrame.delframe.reload -side top -padx 2 -pady 2
pack $objectFrame.delframe -side right -padx 2

#----hide and show buttons----
frame $objectFrame.visframe
button $objectFrame.visframe.save -text Save -command {saveobj} -pady 0
button $objectFrame.visframe.hide -text Hide -command {hideobj} -pady 0
button $objectFrame.visframe.show -text Show -command {showobj} -pady 0
button $objectFrame.visframe.info -text Info -command {infoobj} -pady 0
pack $objectFrame.visframe.save -side left -padx 2
pack $objectFrame.visframe.hide -side left -padx 2
pack $objectFrame.visframe.show -side left -padx 2
pack $objectFrame.visframe.info -side left -padx 2
pack $objectFrame.visframe -padx 2 -pady 2 -fill x
pack $objectFrame -expand yes -fill both -padx 1

#----save scene button----
frame $objectFrame.savescene
button $objectFrame.savescene.btn -text "Save scene" -command {savescene} -pady 0
pack $objectFrame.savescene.btn -side left -padx 2 -fill x -expand yes
pack $objectFrame.savescene -padx 2 -pady 2 -fill x

frame $objframe.spacer
pack $objframe.spacer -side top -pady 2

#--------texture list--------
frame $objframe.textureframe -relief groove -borderwidth 2
set textureFrame $objframe.textureframe

label $textureFrame.label -text "Textures" -relief sunken -background "#DDDDDD" -borderwidth 2
pack $textureFrame.label -side top -fill x -padx 2 -pady 2

frame $textureFrame.listframe
set textureListFrame $textureFrame.listframe
listbox $textureListFrame.texturelist -height 3 -selectmode browse -yscrollcommand "$textureListFrame.scroll set" -exportselection no
scrollbar $textureListFrame.scroll -command "$textureListFrame.texturelist yview"
pack $textureListFrame -padx 2 -fill both
pack $textureListFrame.texturelist -side left -expand y -fill both
pack $textureListFrame.scroll -side right -fill y
bind $textureListFrame.texturelist <Double-Button-1> {setdefaulttexture $clutEditor}
bind $textureListFrame.texturelist <Return> {setdefaulttexture $clutEditor}

set textureList $textureListFrame.texturelist
set defaultTextureLabel $textureFrame.defaulttexturelabel

label $textureFrame.defaulttexturelabel -text "Default:" -anchor w -justify left
pack $textureFrame.defaulttexturelabel -side top -expand yes -fill x

#----Texture buttons---
frame $textureFrame.textureButtons
button $textureFrame.textureButtons.editclut -text "Edit" -command {editclutbutton $clutEditor} -pady 0
button $textureFrame.textureButtons.savetexture -text "Save" -command {savetexturebutton $clutEditor} -pady 0
button $textureFrame.textureButtons.set -text "Assign" -command {settexture $clutEditor} -pady 0
button $textureFrame.textureButtons.key -text "Key" -command {keyclutbutton $colorKey} -pady 0
button $textureFrame.textureButtons.deletetexture -image bitmap_delete -command {deletetexturebutton $clutEditor}

pack $textureFrame.textureButtons.editclut -side left -padx 2
pack $textureFrame.textureButtons.savetexture -side left -padx 2
pack $textureFrame.textureButtons.set -side left -padx 2
pack $textureFrame.textureButtons.key -side left -padx 2
pack $textureFrame.textureButtons.deletetexture -side right -padx 2
pack $textureFrame.textureButtons -padx 2 -pady 2 -fill x

#----default texture button----
frame $textureFrame.custtexture
button $textureFrame.custtexture.default -text "Assign Default Texture to Object" -command {usedefaulttexture $clutEditor} -pady 0
pack $textureFrame.custtexture.default -side left -padx 2 -fill x -expand yes
pack $textureFrame.custtexture -padx 2 -pady 2 -fill x -side top

pack $textureFrame -padx 2 -pady 2 -fill both


#--------Toolbar--------

#ButtonBox .buttonbox
#
#.buttonbox add -image [Bitmap::get open] \
#		 -command {loadobj [tk_getOpenFile -initialdir $datadir -filetypes [getObjectFilterList]]}\
#		 -helptext "Open an object"
#
#.buttonbox add -text "W" \
#		 -command "$mainWindow keyboard -1 -1 0 w 0 w"\
#		 -helptext "Hide or Show widgets"
#
#.buttonbox add -text "Nav" \
#		-command {gz newobject NavigationPath d [gz getOrigin X] [gz getOrigin Y] 0}\
#		-helptext "New Navigation Path"
#
#pack .buttonbox -side top -anchor w


#--------togl widget for main display--------
#togl $mainWindow -stereo true -depth true -double true -width 100 -height 100 -ident main
#set useStereo false
if {[catch "togl $mainWindow -stereo $useStereo -depth true -double true -width 100 -height 100 -ident main -stencil true"]} {
	puts "Disabling stereo"
	set useStereo false
	if {[catch "togl $mainWindow -depth true -double true -width 100 -height 100 -ident main -stencil true"]} {
		puts "Problem initializing Togl widget"
		set savedInfo $errorInfo
		error "Problem initializing Togl widget" $savedInfo
	}
}


createOptionMenu $mainWindow main

pack $mainWindow -expand true -fill both -side left

pack $pane -expand true -fill both -side top
bind $pane <Double-1> {recordAndExecMaybe {global pane; $pane.f1 configure -width 300}}

#---Attribute editor---
attributeWindow .editPoint .Interactor.pointHandle.editPoint -destroyable 0 -wrapper 2 -showHandler {gz tellObject .EventMediator.user push editPointSelection} -hideHandler {gz tellObject .EventMediator.user remove editPointSelection}
gz event handler create editPointSelection for objectSelected {
	global attribFullObject
	if {%iM < 0} {
		set attribFullObject(editPoint) %V
		set localstr ""
	} else {
		set attribFullObject(editPoint) .Interactor.pointHandle.editPoint
		set localstr "wrapper around "
	}
	subwindowTitle editPoint "Attributes for Edit-Target ($localstr[gz tellObject %V objectType])"
	attributeUpdate editPoint 1 1
}
gz tellObject .EventMediator.user push editPointSelection
subwindowMinimize editPoint



#---Main window setup---
#---this can't be put in visualizer.tcl without getting the mainWindow in front of the splashscreen
wm geometry . 640x480
wm protocol . WM_DELETE_WINDOW {gz_exit wm}
focus $mainWindow
update
$pane.f1 configure -width 0


#---Splash Screen---
toplevel .splash
wm title .splash "GeoZui4D"
wm protocol .splash WM_DELETE_WINDOW {wm withdraw .splash}
canvas .splash.canvas -height 300 -width 400
image create photo splashPhoto -file [file join $appdir "GeoZui_splash.gif"] -height 300 -width 400
.splash.canvas create image 200 150 -image splashPhoto
#label .splash.versionLabel -text "GeoZui4D Version $GeoZuiVersion $GeoZuiDescriptor"
label .splash.versionLabel -text "GeoZui4D V$GeoZuiVersion"
label .splash.copyright -text "Copyright (c) 2000-2010.  All rights reserved.
 Not to be copied or distributed without written agreement.
 Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu"
button .splash.okbutton -text "OK" -command "wm withdraw .splash" -takefocus 1
pack .splash.canvas -expand yes -fill both
pack .splash.versionLabel
pack .splash.copyright
pack .splash.okbutton -pady 10
bind .splash <KeyRelease-Return> ".splash.okbutton invoke"
wm deiconify .splash
after 8000 {wm withdraw .splash}
#update

#---Allow user interaction---
bind . <KeyPress-Q> {gz_exit Q}

#---Ground the common functions from other interfaces---
proc gnAttachToVessel {args} {}
