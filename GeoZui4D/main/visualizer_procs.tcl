#============================================================================#
############################ General procedures ##############################
#============================================================================#

global loadingGZ
set loadingGZ 0

#-----------------------------------------------------------------------------
#GL redisplay procedure
#-----------------------------------------------------------------------------
proc redisplay {} {
	global mainWindow

    $mainWindow render
}

#-----------------------------------------------------------------------------
#background color change procedure
#-----------------------------------------------------------------------------
proc changebg {win} {
	set color [tk_chooseColor -title "Background color"]
	if {[string length $color] > 0} {
		$win bgcolor $color
		recordAndExecMaybe redisplay
	}
}

#-----------------------------------------------------------------------------
#Enable user interaction with the main window
#-----------------------------------------------------------------------------
proc enableBindings {} {
	global mainWindow clutEditor

	bind $mainWindow <KeyPress>      "$mainWindow keyboard %x %y %s %A 0 %N"
	bind $mainWindow <KeyRelease>    "$mainWindow keyboard %x %y %s %A 1 %N"
	bind $mainWindow <Motion>        "$mainWindow motion %x %y %s 0"
	bind $mainWindow <ButtonPress>   "focus $mainWindow; $mainWindow mouse %x %y %s %b 0"
	bind $mainWindow <ButtonRelease> "focus $mainWindow; $mainWindow mouse %x %y %s %b 1"
	bind $mainWindow <MouseWheel>    "focus $mainWindow; if {%D > 0} {$mainWindow mouse %x %y %s 4 0} else {$mainWindow mouse %x %y %s 5 0}"
	bind $mainWindow <Enter>         "$mainWindow motion %x %y %s 1"
	bind $mainWindow <Leave>         "$mainWindow motion %x %y %s -1"
	bind $clutEditor <Motion>        "$clutEditor motion %x %y %s 0"
	bind $clutEditor <ButtonPress>   "$clutEditor mouse %x %y %s %b 0"
	bind $clutEditor <ButtonRelease> "$clutEditor mouse %x %y %s %b 1"
	bind $clutEditor <Enter>         "$clutEditor motion %x %y %s 1"
	bind $clutEditor <Leave>         "$clutEditor motion %x %y %s -1"

	foreach name [subwindowVerifyList] {
		subwindowEnableBindings $name
	}
}

#-----------------------------------------------------------------------------
#Disable user interaction with the main window
#-----------------------------------------------------------------------------
proc disableBindings {} {
	global mainWindow clutEditor

	bind $mainWindow <KeyPress>      ""
	bind $mainWindow <KeyRelease>    ""
	bind $mainWindow <Motion>        ""
	bind $mainWindow <ButtonPress>   ""
	bind $mainWindow <ButtonRelease> ""
	bind $mainWindow <MouseWheel>    ""
	bind $mainWindow <Enter>         ""
	bind $mainWindow <Leave>         ""
	bind $clutEditor <Motion>        ""
	bind $clutEditor <ButtonPress>   ""
	bind $clutEditor <ButtonRelease> ""
	bind $clutEditor <Enter>         ""
	bind $clutEditor <Leave>         ""

	foreach name [subwindowVerifyList] {
		subwindowDisableBindings $name
	}
}

#============================================================================#
###################### Recording & Playback Procedures #######################
#============================================================================#
#-----------------------------------------------------------------------------
#This procedure is used during playback, to make the calling script smaller 
#-----------------------------------------------------------------------------
proc idle {{count 1}} {
	global mainWindow

	for {set i 0} {$i < $count} {incr i} {
		$mainWindow idleCallBack
		update
	}
}

#-----------------------------------------------------------------------------
#This procedure is used at the beginning of recording to record important
#state information.
#-----------------------------------------------------------------------------
set globalVariables {stereoflag toglPath dominantEye oneEyedFlag}
proc beginRecord {} {
	global mainWindow subWindowCount globalVariables

	$mainWindow recordCommand "set subWindowCount $subWindowCount"
	foreach variable $globalVariables {
		global $variable
		$mainWindow recordCommand "global $variable"
		if {[info exists $variable] && ( [array exists $variable] == 0 ) } {
			set val {}
			set val [eval set $variable]
			if {$variable == "recordingState" || $variable == "highResMutexHolder" || \
				$variable == "GeoZuiVersion" || $variable == "GeoZuiDescriptor" || \
				$variable == "currentProjection"} {
				$mainWindow recordTimestamp "set $variable \{$val\}"
			} else {
				$mainWindow recordCommand "set $variable \{$val\}"
			}
		}
	}
	$mainWindow recordCommand "wm geometry . [winfo geometry .]"
	bind . <Configure> {if {"%W" == "."} {recordMaybe "wm geometry . [winfo geometry .]"}}
	timestamp
}

proc timestamp {} {
	global recorderResolution
	global recordingState RECORDER_RECORDING

	if {$recordingState == $RECORDER_RECORDING} {
		.glwindow recordTimestamp Sync
	}
	after $recorderResolution timestamp
}

#-----------------------------------------------------------------------------
#This procedure is used at the end of recording to restore bindings to
#their proper states.
#-----------------------------------------------------------------------------
proc endRecord {} {
	after cancel timestamp
	bind . <Configure> ""
	enableBindings
}

#-----------------------------------------------------------------------------
#This procedure is used to record a command, conditionally
#-----------------------------------------------------------------------------
proc recordMaybe {command} {
	global mainWindow
	global recordingState RECORDER_RECORDING

	if {$recordingState == $RECORDER_RECORDING} {
		$mainWindow recordCommand $command
	}
}

#-----------------------------------------------------------------------------
#This procedure is used to record a comment, conditionally
#-----------------------------------------------------------------------------
proc recordTimestampMaybe {{comment {}}} {
	global mainWindow
	global recordingState RECORDER_RECORDING

	if {$recordingState == $RECORDER_RECORDING} {
		$mainWindow recordTimestamp $comment
	}
}

#-----------------------------------------------------------------------------
#This procedure is used to both record and execute a command, conditionally
#-----------------------------------------------------------------------------
proc recordAndExecMaybe {command} {
	recordMaybe $command
	eval $command
}

#-----------------------------------------------------------------------------
#This procedure is used to both record and execute a command
#-----------------------------------------------------------------------------
proc recordAndExec {command} {
	global mainWindow

	$mainWindow recordCommand $command
	eval $command
}

#-----------------------------------------------------------------------------
#This procedure is used to pause during recording and playback
#-----------------------------------------------------------------------------
proc pauseRecording {} {
	global recordingState RECORDER_PAUSED

	set recordingState [expr $recordingState ^ $RECORDER_PAUSED]
}

#-----------------------------------------------------------------------------
#This procedure is used to stop during recording and playback
#-----------------------------------------------------------------------------
proc stopRecording {} {
	global recordingState RECORDER_STOPPED

	set recordingState $RECORDER_STOPPED
}

#-----------------------------------------------------------------------------
#This procedure is used at the beginning of playback to perform proper setup
#-----------------------------------------------------------------------------
proc beginPlayback {} {
	global mainWindow

	after cancel "idleProc" ; update;# first kill the idle events
	disableBindings
	bind $mainWindow <KeyPress> pauseRecording
	bind $mainWindow <KeyPress-Escape> stopRecording

	.menubar add command -label "0.000" -command {}
}

#-----------------------------------------------------------------------------
#This procedure is used during playback to provide an indicator of the current
#playback time.
#-----------------------------------------------------------------------------
proc updatePlaybackTime {time} {
	.menubar entryconfigure end -label "$time"
}

#-----------------------------------------------------------------------------
#This procedure is used at the end of playback to restore bindings to
#their proper states.
#-----------------------------------------------------------------------------
proc endPlayback {} {
	global mainWindow

	bind $mainWindow <KeyPress-Escape> {}
	enableBindings
	after idle idleProc

	.menubar delete end
}

#-----------------------------------------------------------------------------
#This procedure is used during normal operation.  It reschedules itself
#automatically each time it is run.
#-----------------------------------------------------------------------------
proc idleProc {} {
	global mainWindow loadingGZ
	global highResMutexHolder drewLowRes

    $mainWindow idleCallBack
	update
	if {($highResMutexHolder == 0 && $drewLowRes == 0) || $loadingGZ} {
		after 30 idleProc
	} else {
		after idle idleProc
	}
}

#============================================================================#
############################ Object procedures ###############################
#============================================================================#

#-----------------------------------------------------------------------------
# Procedure for regenerating the file filter list from currently loaded types.
#-----------------------------------------------------------------------------
proc getObjectFilterList {} {
	global mainWindow objectTypeAlias

	set extensions [gz getObjectExtensions]
	set dotExtensions {.sdv .gzx .gzn .gznotes}
	foreach extension $extensions {lappend dotExtensions .$extension}

	set supportedTypes [list "Supported file types" $dotExtensions]
	set objTypes [list $supportedTypes [list $objectTypeAlias(sdv) .sdv]]

	foreach extension $extensions type [gz getObjectTypes] {
		if {[string length $extension] == 0} continue
		if {[info exists objectTypeAlias($extension)]} {
			lappend objTypes [list "$objectTypeAlias($extension) Files" .$extension]
		} else {
			lappend objTypes [list "$type Files" .$extension]
		}
	}
	lappend objTypes {{All} *}
	return $objTypes
}

#-----------------------------------------------------------------------------
#load object procedure
#-----------------------------------------------------------------------------
proc loadobj {filenames} {
	global datadir loadingGZ

	if {[string length $filenames] < 1} return

	set loadingGZ 1
	foreach fn $filenames {
#		gz loadobject [string trimright $fn [file tail $fn]] [file tail $fn]
		gz loadobject [string trimright $fn]
	}

	set loadingGZ 0
	redisplay
	set datadir [regsub -all {\\} [file dirname $fn] "/"]
}

#-----------------------------------------------------------------------------
#procedure for converting a base grid (Caris HIPS) to gutm and loading the gutm
#-----------------------------------------------------------------------------
proc loadBaseGrid {filenames} {
	global appdir

	set f [open [file join $appdir base2gutm secondary_choices.ini] r]
	set fileFields {}
	set i 0
	while {[eof $f] == 0} {
		gets $f val($i)
		if {$val($i) == ""} continue
		append fileFields " {$val($i)}"
		incr i
	}

	foreach filename $filenames {
		set outfilename [file rootname $filename].gutm

		set wd [pwd]
		cd [file join $appdir base2gutm]
		if {[catch "exec base2gutm -l $filename $outfilename" list]} {
			tk_messageBox -message "$list\n\nLoad aborted." -parent . -type ok -title "Error in base2gutm process"
			cd $wd
			return
		}
		set choice [eval "tk_dialog .loadBaseGridDlg {Specify base2gutm parameters...} {The following is information about $filename.\n-----\n$list\n-----\nThe primary field will be depth.\nWhat do you want the secondary field to be?\n} {} 0 $fileFields"]

		if {$choice < 0} return

		set secondary $val($i)
		if {$secondary == "<NONE>"} {
			set secondaryString ""
		} else {
			set secondaryString "-s $secondary"
		}
		if {[catch "exec base2gutm -p Depth $secondaryString $filename $outfilename" list]} {
			tk_messageBox -message "$list\n\nLoad aborted." -parent . -type ok -title "Error in base2gutm process"
			cd $wd
			return
		}
		cd $wd
		gz loadobject $outfilename
	}
}

#-----------------------------------------------------------------------------
#new object procedure
#-----------------------------------------------------------------------------
proc newobj {otype olabel args} {
	eval "gz newobject $otype $olabel $args"
	redisplay
}

#-----------------------------------------------------------------------------
#Procedure for adding an object to the object listbox
#-----------------------------------------------------------------------------
proc addobj {index parent obj label {bitmap {}}} {
#	global objectList
	global objectTree
	global appdir
	
#	$objectList insert end $label
#$objectTree subwidget hlist add $path -text $label
	if { [$objectTree exists $parent] == 0} {
		$objectTree insert end root $parent -text "tmp"
		puts "$parent exists"
	}
	if { [$objectTree exists $obj] } {
		$objectTree move $parent $obj $index
  	if {$bitmap == {}} {
    	$objectTree itemconfigure $obj -text $label
  	} else {
  		if {[catch {set bitmapName [Bitmap::get [file join $appdir "bitmaps/$bitmap.gif"]]}]} {
  			$objectTree itemconfigure $obj -text $label -image [Bitmap::get [file join $appdir "bitmaps/Object.gif"]]
  		} else {
  			$objectTree itemconfigure $obj -text $label -image $bitmapName
  		}
  	}
		
	}
	if {$bitmap == {}} {
		$objectTree insert $index $parent $obj -text $label
	} else {
		if {[catch {set bitmapName [Bitmap::get [file join $appdir "bitmaps/$bitmap.gif"]]}]} {
			$objectTree insert $index $parent $obj -text $label -image [Bitmap::get [file join $appdir "bitmaps/Object.gif"]]
		} else {
			$objectTree insert $index $parent $obj -text $label -image $bitmapName
		}
	}
}

#-----------------------------------------------------------------------------
#Procedure for removing an object from the object listbox
#-----------------------------------------------------------------------------
proc removeobj {id} {
	global objectTree
	
#	$objectTree subwidget hlist delete entry $id
	$objectTree delete $id
}

#-----------------------------------------------------------------------------
#Procedure for deleting an object from the scene
#-----------------------------------------------------------------------------
proc deleteobj {} {
	global objectTree
	set numDeleted 0


#	set objselection [$objectTree subwidget hlist info selection]
	set objselection	[$objectTree selection get]
	foreach {objid} $objselection {
		puts $objid
		gz deleteobject $objid
		incr numDeleted
	}

	redisplay
}

#-----------------------------------------------------------------------------
#Procedure for reloading an object in the scene
#-----------------------------------------------------------------------------
proc reloadobj {} {
	global objectTree

#	set objselection [$objectTree subwidget hlist info selection]
	set objselection	[$objectTree selection get]
	foreach {objid} $objselection {
		gz reloadobject $objid
	}
	redisplay
}

#-----------------------------------------------------------------------------
#Object Info procedure
#-----------------------------------------------------------------------------
proc infoobj {} {
	global objectTree

	clearObjectInfo
#	set objselection [$objectTree subwidget hlist info selection]
	set objselection	[$objectTree selection get]
	foreach {objid} $objselection {
		gz explainobject $objid
	}
}

#-----------------------------------------------------------------------------
#Object Info clearing procedure
#-----------------------------------------------------------------------------
proc clearObjectInfo {} {
	global objectInfo

	$objectInfo configure -state normal
	$objectInfo delete 1.0 end
	$objectInfo configure -state disabled
}

#-----------------------------------------------------------------------------
#Object Info adding procedure
#-----------------------------------------------------------------------------
proc appendObjectInfo {string} {
	global objectInfo

	$objectInfo configure -state normal
	$objectInfo insert end $string
	$objectInfo configure -state disabled
}

#-----------------------------------------------------------------------------
#load texture procedure
#-----------------------------------------------------------------------------
proc loadtexture {filenames} {
	global datadir clutEditor

	if {[string length $filenames] < 1} return

	foreach fn $filenames {
		$clutEditor loadtexture [string trimright $fn]
	}
	set datadir [regsub -all {\\} [file dirname $fn] "/"]
}

#-----------------------------------------------------------------------------
#Window location update procedure
#-----------------------------------------------------------------------------
proc setWorkspaceLocation {win string} {
	global positionText

	set positionText($win) $string
	return
}

#-----------------------------------------------------------------------------
#Window heading update procedure
#-----------------------------------------------------------------------------
proc setWorkspaceHeading {win string} {
	global headingText

	set headingText($win) $string
	return
}

#-----------------------------------------------------------------------------
#Window pitch update procedure
#-----------------------------------------------------------------------------
proc setWorkspacePitch {win string} {
	global pitchText

	set pitchText($win) $string
	return
}

#-----------------------------------------------------------------------------
#Time update procedure
#-----------------------------------------------------------------------------
#proc setTime {win string} {
#	global timeText

#	set timeText($win) $string
#	return
#}


#-----------------------------------------------------------------------------
#Object Hiding procedure
#-----------------------------------------------------------------------------
proc hideobj {} {
	global objectTree

#	set objselection [$objectTree subwidget hlist info selection]
	set objselection	[$objectTree selection get]
	foreach {objid} $objselection {
		gz hideobject $objid
		redisplay
	}
}


######### Widget procs replaced by "gz widgets on/off" ##########

#-----------------------------------------------------------------------------
#Widget Hiding and Showing procedures
#-----------------------------------------------------------------------------
#proc hideWidget {} {
#	global widgetVisible
#	set widgetVisible -1
#	redisplay
#}

#proc showWidget {} {
#	global widgetVisible
#	set widgetVisible 1
#	redisplay
#}

#-----------------------------------------------------------------------------
#Procedure to save an object to a file
#-----------------------------------------------------------------------------
proc saveobj {} {
	global objectTree
	global datadir

#	set objselection [$objectTree subwidget hlist info selection]
	set objselection	[$objectTree selection get]
	foreach {objid} $objselection {
		set dext "."
		append dext [gz getobjext $objid]
		set saveobjtype [list [list $dext $dext]]
		set fn [tk_getSaveFile -initialdir $datadir -defaultextension $dext -filetypes $saveobjtype -title "Save Object"]
		if {$fn != ""} {
			gz saveobject $objid $fn
		}
	}
}

#-----------------------------------------------------------------------------
#Scene-clearing procedure
#-----------------------------------------------------------------------------
proc clearScene {} {
	global numEditedObjects

	set result 1
	if {$numEditedObjects > 0} {
		if {$numEditedObjects == 1} {
			set areObjectsEditedText "is 1 object that has"
			set saveText "save it"
		} else {
			set areObjectsEditedText "are $numEditedObjects objects that have"
			set saveText "save them"
		}
		set result [tk_dialog .clearSceneDialog "Clear Scene"\
		"There $areObjectsEditedText\nbeen edited but not saved.\n\
		 Would you like to $saveText?" {} 0 "Yes, $saveText" "No, clear scene"\
		"Save entire scene" "Cancel"]
	}
	if {$result == 0} {
		gz saveEdited
	}
	if {$result == 2} {
		savescene
	}
	if {$result < 3} {
		gz clearScene
	}
}

#-----------------------------------------------------------------------------
#Object Redisplay procedure
#-----------------------------------------------------------------------------
proc showobj {} {
	global objectTree

#	set objselection [$objectTree subwidget hlist info selection]
	set objselection	[$objectTree selection get]
	foreach {objid} $objselection {
		gz showobject $objid
		redisplay
	}
}

#-----------------------------------------------------------------------------
#One-eyed cursor setting
#-----------------------------------------------------------------------------
proc oneEyedCursor {args} {
    global oneEyedFlag toglPath appdir

    if {$oneEyedFlag == 1} {
#set cursor "@[file join $appdir blank.cur]"
		set cursor "none"
    } else {
		set cursor "arrow"
    }

    $toglPath(main) configure -cursor $cursor
	foreach name [subwindowVerifyList] {
		$toglPath($name) configure -cursor $cursor
	}
	return
}

#-----------------------------------------------------------------------------
#Procedure to create context-sensitive menus per object
#-----------------------------------------------------------------------------
proc createObjectMenu {classname entries} {
	global ObjectTypeAlias mainWindow

	set menuPath .objectMenu_$classname
	menu $menuPath -tearoff 0
	$menuPath add command -label "Object"
	$menuPath add separator
	foreach {label command} $entries {
		if {[string length $label] < 1} {
			$menuPath add separator
		} else {
			eval "set label \"$label\""
			$menuPath add command -label $label -command "gz tellSelectedObject $command"
		}
	}
	return $menuPath
}

#-----------------------------------------------------------------------------
#Procedure to post context-sensitive menu for an object
#-----------------------------------------------------------------------------
proc postObjectMenu {name classname objectname localX localY args} {
	global absoluteWindowPath
	global toglPath
	global recordingState RECORDER_PLAYING
	global objframe

#	set windowPath $absoluteWindowPath($name)
	if {[string index $name 0] == "."} {
		set togl $name
	} else {
		set togl $toglPath($name)
	}

	set globalX [expr $localX + [winfo rootx $togl]]
	set globalY [expr $localY + [winfo rooty $togl]]
	set menuPath .objectMenu_$classname

	foreach arg $args {
		if {[string length $arg] > 0} {
			eval "$menuPath $arg"
		}
	}
	$menuPath entryconfigure 0 -label $objectname
	if {($recordingState & $RECORDER_PLAYING) == 0} {
		tk_popup $menuPath $globalX $globalY
	}
}

proc contextMenu {id} {
	gz postobjectmenu $id
}

#============================================================================#
############################# Texture procedures ################################
#============================================================================#

#-----------------------------------------------------------------------------
#Color lookup table selection procedure
#-----------------------------------------------------------------------------
proc settexture {win} {
	global objectTree textureList

#	set objselection [$objectTree subwidget hlist info selection]
	set objselection	[$objectTree selection get]
	foreach {objid} $objselection {
		scan [$textureList get [$textureList curselection]] {%[0-9]} textureid
		$win usecustomtexture $objid $textureid
		redisplay
	}
}

#-----------------------------------------------------------------------------
#Default texture selection procedure
#-----------------------------------------------------------------------------
proc usedefaulttexture {win} {
	global objectTree

	set objselection	[$objectTree selection get]
	foreach {objid} $objselection {
		$win usedeftexture $objid
		redisplay
	}
}

#-----------------------------------------------------------------------------
#Procedure to save the objects and textures into a scenefile
#-----------------------------------------------------------------------------
proc savescene {} {
	set scenetypes {
		{{XML Scene} { .gzx }}
	}
	gz saveEdited
	set fn [tk_getSaveFile -defaultextension .gzx -filetypes $scenetypes -title "Save scene"]
	if {$fn != ""} {
#		set outfile [open $fn w]
#        gz buildscene "[file dirname $fn]/"
		gz savescene $fn "[file dirname $fn]/"
	}
#	puts $outfile end
#	close $outfile
}

#-----------------------------------------------------------------------------
#Procedure to add a texture to the list
#-----------------------------------------------------------------------------
proc addtexture {label} {
	global textureList

	$textureList insert end $label
	return
}

#-----------------------------------------------------------------------------
#Procedure to delete a texture from the list
#-----------------------------------------------------------------------------
proc deletetexturebutton {win} {
    global textureList

    scan [$textureList get [$textureList curselection]] {%[0-9]} textureid
    $textureList delete [$textureList curselection]

    $win deletetexture $textureid
	return
}

#-----------------------------------------------------------------------------
#Procedure to change which texture is the default
#-----------------------------------------------------------------------------
proc setdefaulttexture {win} {
	global textureList defaultTextureLabel

	scan [$textureList get [$textureList curselection]] {%[0-9]} textureid
	$win defaulttexture $textureid
	$defaultTextureLabel configure -text "Default: [$textureList get [$textureList curselection]]"
#	puts "yup: [$textureList get [$textureList curselection]]"
	redisplay
}

#-----------------------------------------------------------------------------
#Procedure to save a texture to a texture file
#-----------------------------------------------------------------------------
proc savetexturebutton {win} {
    global textureList

    scan [$textureList get [$textureList curselection]] {%[0-9]} textureid

    set texturetypes {
	{{Color Table} {.clut}}
	{{Grid} {.grid}}
	{{Image} {.jpg .ppm}}
    }

    set fn [tk_getSaveFile -defaultextension .clut -filetypes $texturetypes -title "Save Texture"]

    if {$fn != ""} {
	$win savetexture $fn $textureid
    }
}

#-----------------------------------------------------------------------------
#Procedure to edit a CLUT from the GUI
#-----------------------------------------------------------------------------
proc editclutbutton {win} {
    global textureList
	
    if {[$textureList curselection] != ""} {
	scan [$textureList get [$textureList curselection]] {%[0-9]} textureid
	recordAndExecMaybe ".cluted.label configure -text \"[$textureList get [$textureList curselection]]\""
	if {[catch "$win editclut $textureid"]} {
	    return
	}
#	recordAndExecMaybe "pack .cluted -side left"
	recordAndExecMaybe "wm deiconify .cluted"
	$win render
    }
}

#-----------------------------------------------------------------------------
#Procedure to show the clutEditor in color-key mode
#-----------------------------------------------------------------------------
proc keyclutbutton {win} {
    global textureList
	
    if {[$textureList curselection] != ""} {
	scan [$textureList get [$textureList curselection]] {%[0-9]} textureid
	recordAndExecMaybe ".cluted.label configure -text \"[$textureList get [$textureList curselection]]\""
	if {[catch "$win editclut $textureid"]} {
	    return
	}
	recordAndExecMaybe "subwindowRestore key"
	$win render
    }
}

#-----------------------------------------------------------------------------
#Procedure to do last-minute cleanup like writing the .conf file.
#-----------------------------------------------------------------------------
proc gz_exit {{fromWhence user}} {
	global configfilename configVariables configReadOnlyVariables
	global numEditedObjects

	set result 1
	if {$numEditedObjects > 0} {
		if {$numEditedObjects == 1} {
			set areObjectsEditedText "is 1 object that has"
			set saveText "save it"
		} else {
			set areObjectsEditedText "are $numEditedObjects objects that have"
			set saveText "save them"
		}
		set result [tk_dialog .exitDialog "Exiting..."\
		"There $areObjectsEditedText\nbeen edited but not saved.\n\
		 Would you like to $saveText?" {} 1 "Yes, $saveText" "No, just exit."\
		"Save entire scene" "Cancel"]
	}
	if {$result == 0} {
		gz saveEdited
	}
	if {$result == 2} {
		savescene
	}
	if {$result == 3} {
		return 0
	}

	foreach var $configVariables {
		set seenVar($var) 0
		global $var
	}

# The following dumps GeoZui4D.conf files all over my file system! In some cases, it 
# causes problems with values read from ~/.GeoZui4D/GeoZui4D.conf. This should be reworked 
# to support multiple conf files and keeping in mind GZ can be launched from various
# working directories. rja 2009-04-01

#	if {[catch {file rename -force "$configfilename" "$configfilename.bak"}]} "gz exit"
#	if {[catch {set previousFile [open "$configfilename.bak" r]}]} "gz exit"
#	if {[catch {set newFile [open "$configfilename" w]}]} "gz exit"

#	while {![eof $previousFile]} {
#		gets $previousFile line
#		if {[regexp {^set[ \t\n]*([^ \t\n]*)[ \t\n]*(.*)} $line all var val]} {
#			global $var
#			set seenVar($var) 1
#			if {[lsearch $configReadOnlyVariables $var] >= 0} {
#				#it's one of those variables we don't want to change.
#				puts $newFile "$line"
#			} else {
#				eval "puts \$newFile \"set $var \\\"\$$var\\\"\""
#			}
#		} else {
#			if {[string length $line] > 0 || ![eof $previousFile]} {
#				puts $newFile "$line"
#			}
#		}
#	}
#	close $previousFile

#	foreach var $configVariables {
#		if {!$seenVar($var)} {
#			eval "puts \$newFile \"set $var \\\"\$$var\\\"\""
#		}
#	}
#	close $newFile

	puts "exiting..."
	gz exit
}

proc delayed_exit {} {
	after 5 "gz_exit delayed_exit"
}

#-----------------------------------------------------------------------------
#Procedure to ask a question in a box
#-----------------------------------------------------------------------------
proc ask {question} {
	global answer
	
	toplevel .popup 
	
	set geo [wm geometry .]
	regexp {([0-9]+).([0-9]+).([0-9]+).([0-9]+)} $geo matchVar width height x y
	wm geometry .popup "+[expr $x+$width/2]+[expr $y+$height/2]"
	
	grab .popup
	wm title .popup ""
	label .popup.question -text $question
	pack .popup.question
	entry .popup.answer -bg white 
	pack .popup.answer
	focus .popup.answer
	bind .popup <Return> {set answer [.popup.answer get];  destroy .popup}
	button .popup.ok -text OK -command {set answer [.popup.answer get];  destroy .popup}
	pack .popup.ok
	

	tkwait window .popup
	return $answer
}
