# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

toplevel .textureButtons
wm title .textureButtons "GeoZui3D Texture Buttons"

#--------texture list--------
frame .textureButtons.textureframe -relief groove -borderwidth 2
set textureFrame .textureButtons.textureframe

label $textureFrame.label -text "Texture Buttons" -relief sunken -background "#DDDDDD" -borderwidth 2
pack $textureFrame.label -side top -fill x -padx 2 -pady 2

#frame $textureFrame.listframe
#set textureListFrame $textureFrame.listframe
#listbox $textureListFrame.texturelist -height 3 -selectmode browse -yscrollcommand "$textureListFrame.scroll set" -exportselection no
#scrollbar $textureListFrame.scroll -command "$textureListFrame.texturelist yview"
#pack $textureListFrame -padx 2 -fill both
#pack $textureListFrame.texturelist -side left -expand y -fill both
#pack $textureListFrame.scroll -side right -fill y
#bind $textureListFrame.texturelist <Double-Button-1> {setdefaulttexture $clutEditor}
#bind $textureListFrame.texturelist <Return> {setdefaulttexture $clutEditor}

#set textureList $textureListFrame.texturelist
#set defaultTextureLabel $textureFrame.defaulttexturelabel

#label $textureFrame.defaulttexturelabel -text "Default:" -anchor w -justify left
#pack $textureFrame.defaulttexturelabel -side top -expand yes -fill x

#----Texture buttons---
frame $textureFrame.textureButtons
button $textureFrame.textureButtons.editclut -text "Edit" -command {editclutbutton $clutEditor} -pady 0
button $textureFrame.textureButtons.savetexture -text "Save" -command {savetexturebutton $clutEditor} -pady 0
button $textureFrame.textureButtons.set -text "Assign" -command {settexture $clutEditor} -pady 0
button $textureFrame.textureButtons.deletetexture -image bitmap_delete -command {deletetexturebutton $clutEditor}

pack $textureFrame.textureButtons.editclut -side left -padx 2
pack $textureFrame.textureButtons.savetexture -side left -padx 2
pack $textureFrame.textureButtons.set -side left -padx 2
pack $textureFrame.textureButtons.deletetexture -side right -padx 2
pack $textureFrame.textureButtons -padx 2 -pady 2 -fill x

#----default texture button----
frame $textureFrame.custtexture
button $textureFrame.custtexture.default -text "Assign Default Texture to Object" -command {usedefaulttexture $clutEditor} -pady 0
pack $textureFrame.custtexture.default -side left -padx 2 -fill x -expand yes
pack $textureFrame.custtexture -padx 2 -pady 2 -fill x -side top

pack $textureFrame -padx 2 -pady 2 -fill both
