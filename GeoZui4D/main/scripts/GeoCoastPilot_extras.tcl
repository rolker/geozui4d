#-----------------------------------------------------------------------------
# GUI setup
#-----------------------------------------------------------------------------

global objectFrame

#--------object frame--------
#we'll create an object frame, but not pack it
#frame .objframe

#set objframe [$pane add -weight 0]
global pane
set objnbframe [$pane add -weight 0]
NoteBook $objnbframe.objnb
#pack $objnbframe.objnb -fill both -expand true

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

#----Actual object list----
ScrolledWindow $objectFrame.listframe
Tree $objectFrame.listframe.objTree -deltax 6 -height 4
$objectFrame.listframe.objTree bindText <ButtonPress-3> "contextMenu"
set objectTree $objectFrame.listframe.objTree
$objectFrame.listframe setwidget $objectTree

pack $objectFrame.listframe -padx 2 -expand yes -fill both

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
