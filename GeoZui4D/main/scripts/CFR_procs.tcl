# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#-----------------------------------------------------------------------------
# Setup and procedures for dealing with CFR's in GeoCoastPilot
#-----------------------------------------------------------------------------

global coastPilot
global httpIsSetup
global CFRList
global configVariables

lappend configVariables refreshCFRs

#Global variables
set httpIsSetup 0
set CFRList {}

#--------------Set up the CFR display window------------------
subwindow .cfr -parent main -width 400 -destroyable 0 -closebutton 1
set CFRFrame .cfr.innards
set CFRText [text $CFRFrame.text -wrap none -cursor arrow -height 1]
set CFRScrollY [scrollbar $CFRFrame.scrollY -command "$CFRText yview"]
frame $CFRFrame.scroll
set CFRScrollX [scrollbar $CFRFrame.scroll.scrollX -command "$CFRText xview" -orient horizontal]
set CFRSpacer [label $CFRFrame.scroll.spacer -width 2 -height 1]
$CFRText configure -xscrollcommand "$CFRScrollX set" -yscrollcommand "$CFRScrollY set"
pack $CFRScrollX -fill x -expand yes -side left
pack $CFRSpacer -side left
pack $CFRFrame.scroll -side bottom -fill x -expand no
pack $CFRScrollY -fill y -expand no -side right
pack $CFRText -fill both -fill both -expand yes -side top

set CFRBoldColor "#3344BB"
$CFRText tag configure CFRTitle -foreground $CFRBoldColor -font "{MS Sans Serif} 12 bold"
$CFRText tag configure CFRChapter -foreground $CFRBoldColor -font "{MS Sans Serif} 10 bold"
$CFRText tag configure CFRPart -foreground $CFRBoldColor -font "{MS Sans Serif} 10"
$CFRText tag configure CFRSubpart -foreground $CFRBoldColor -font "{MS Sans Serif} 8 bold"
$CFRText tag configure CFRSection -font "{MS Sans Serif} 8 bold"
$CFRText tag configure CFRtoc -foreground "#338822" -font "{MS Sans Serif} 8"
$CFRText tag configure CFRAnnotation -foreground "#777777" -font "{Times New Roman} 8 italic"
$CFRText configure -state disabled
subwindowResizeAndMove cfr 450 400 570 50
subwindowMinimize cfr

#-----------------------
proc CFRSetTitle {title} {
	subwindowTitle cfr $title
	CFRshow
}

#-----------------------
proc CFRClear {} {
	global CFRText
	
	$CFRText configure -state normal
	$CFRText delete 1.0 end
	$CFRText configure -state disabled
	
	foreach variable {currentCFRTitle currentCFRChapter currentCFRPart currentCFRSubpart} {
		global $variable
		set $variable ""
	}
}

#-----------------------
proc CFRshow {} {
	subwindowRestore cfr
}

#-----------------------
proc CFRAddText {text} {
	global CFRText
	global currentCFRTitle currentCFRChapter
	global currentCFRPart currentCFRSubpart

	if {$text == "" || $text == "\n"} return
	
	set text [regsub -all {\[deg\]} $text "\xb0"]
	set text [regsub -all {\[min\]} $text "'"]
	set text [regsub -all {\[sec\]} $text "\""]
	set text [regsub -all {[\n]*\[(Code|Title|CITE|\[Page[^]]*\])[^\]]*\][\n]*} $text ""]
	set text [regsub -all {From the U.S. Government Printing Office via GPO Access} $text ""]
	set text [regsub -all {\n\n+} $text "\n"]
	
	set prefix ""
	set newTitle ""
	set toc ""
	set titlePattern {TITLE [0-9]*--[^\n]*\n}
	regexp "(.*)\[ \t\n\]*($titlePattern)(.*)" $text all prefix newTitle text
	if {$prefix == "" || $newTitle == ""} {
		puts $text
	}
	set newChapter ""
	set chapterPattern {CHAPTER [^-]*--[^\n]*\n}
	regexp "\[ \t\n\]*($chapterPattern)(.*)" $text all newChapter text
	set partPattern {PART [0-9]*[-_]+[A-Z ]*}
	set newPart ""
	regexp "\[ \t\n\]*($partPattern)\[^\n\]*\n(.*)" $text all newPart text
	set newPart "[regsub "_+" $newPart "--"]\n"
	set newSubpart ""
	set subpartPattern {Subpart [^-_]*[-_]+[^\n]*\n}
	regexp "^\[ \t\n\]*($subpartPattern)(.*)" $text all newSubpart text
	set newSubpart [regsub "_+" $newSubpart "--"]
	set newSection ""
	set sectionPattern {Sec. [ 0-9]+[.][0-9]+[^.]*[.]\n}
	regexp "^(Table|of|Contents|\[ \t\n\])+($sectionPattern)(.*)" $text all junk newSection text
	set newSection [regsub "Sec. " $newSection "\xa7"]

	set tocPattern {Sec.[ \t]*\n[^^]*(\^\[[^\]]*\]\n)*(\^[ \t]*Authority:)*[^^]*(\^[ \t]*Source:)*[^[^]*noted.\n(\^[ \t]*Editorial Note:[^([]*.\n)*}
	regexp "^((Subpart\[^\n\]*\n)|\[ \t\n\])*($tocPattern)(.*)" $text all junk1 junk2 toc junkNote junkAuth junkSource junkEdit text
	
	set prefix [regsub -all {\n[\n \t]+} $prefix "\n"]
	set text [regsub -all "Sec. " $text "\xa7"]
	set text [regsub "Alabama\n$" $text ""]
	set toc [regsub -all {\^} $toc ""]
	$CFRText configure -state normal
	$CFRText insert end "$prefix" CFRAnnotation
	if {$newTitle != $currentCFRTitle} {
		$CFRText insert end $newTitle CFRTitle
		set currentCFRTitle $newTitle
	}
	if {$newChapter != $currentCFRChapter} {
		$CFRText insert end $newChapter CFRChapter
		set currentCFRChapter $newChapter
	}
	if {$newPart != $currentCFRPart} {
		$CFRText insert end $newPart CFRPart
		set currentCFRPart $newPart
	}
	if {$toc != ""} {
		$CFRText insert end "Table of Contents\n" CFRtoc
		if {$newSubpart != ""} {
			$CFRText insert end "$newSubpart\n" CFRtoc
		}
		$CFRText insert end "$toc\n" CFRtoc
	}
	if {$newSubpart != $currentCFRSubpart} {
		$CFRText insert end "$newSubpart\n" CFRSubpart
		set currentCFRSubpart $newSubpart
	}
	$CFRText insert end $newSection CFRSection
	$CFRText insert end "$text\n"
	$CFRText configure -state disabled
}

#----------------------------------
proc CFRClicked {CFRTag CFRIndex} {
	global coastPilot CFRMap
	
	set range [$coastPilot tag prevrange $CFRTag current]
	if {$range == ""} return
	foreach {startPos endPos} $range {}
	CFRClear
	CFRSetTitle "CFR Section(s) [regsub {, chapter 2} [$coastPilot get $startPos $endPos] {}]"
	. configure -cursor wait
	set oldCursor [lindex [$coastPilot configure -cursor] end]
	$coastPilot configure -cursor wait
	update
	foreach section $CFRMap($CFRIndex) {
		postCFRSection $section
		update
	}
	. configure -cursor ""
	$coastPilot configure -cursor $oldCursor
}

#----------------------------------
proc httpSetup {} {
	global httpIsSetup
	
	if {$httpIsSetup != 0} return
	
	package require http;
	
	#http::config -proxyhost {}
	#http::config -proxyport {}

	global GeoCoastPilotVersion
	http::config -useragent "GeoCoastPilot $GeoCoastPilotVersion (Tcl http client)"
	
	set httpIsSetup 1
}

#----------------------------------
proc getCFRURL {section} {
	#Sample web filename of a section such as 33:165.103 is 33cfr165.103.htm
	#&&&New way?  http://frwebgate.access.gpo.gov/cgi-bin/get-cfr.cgi?TITLE=33&PART=165&SECTION=103&TYPE=TEXT
	#Sample address: Section 15:922.21 would be http://edocket.access.gpo.gov/cfr_2008/janqtr/15cfr922.21.htm
	#Titles 1-16 Jan;  Titles 17-27 Apr;  Titles 28-41 Jul;  Titles 42- Oct
	set webBase "http://edocket.access.gpo.gov/cfr_"
	
	set now [clock seconds]
	set year [clock format $now -format "%Y"]
	set month [string trimleft [clock format $now -format "%m"] "0"]
	
	set title ""
	regexp {([^:]*):} $section all title
	set address ""
	
	foreach {lastTitle qname qnum} {16 janqtr 1
									27 aprqtr 4
									41 julqtr 7
									50 octqtr 10} {
		if {$title <= $lastTitle} {
			if {$month <= $qnum} {
				incr year -1; #use previous year if the month hasn't occurred yet
			}
			set address "$webBase$year/$qname/[regsub : $section cfr].htm"
			break; #We don't want to keep going if we're in the right range!
		}
	}
	return $address
}

#----------------------------------
proc postCFRSection {section} {
	global appdir
	
	set filename "[regsub : $section cfr].txt"
	set directory [file join $appdir CFR]
	set cachedFile [open [file join $directory $filename] "r"]
	set contents [read $cachedFile]
	close $cachedFile
	CFRAddText $contents
}

#----------------------------------
proc updateCFRsFromWeb {} {	
	global CFRTokenLookup CFRList
	
	#Sample filename of a section such as 33:165.103 is 33cfr165.103.htm
	httpSetup
	foreach section $CFRList {
		set url [getCFRURL $section]
		
		if {$url == ""} {
			puts "Could not convert $section to a URL"
			return;
		}

		if {[catch {set token [http::geturl $url -command handleCFRGet -timeout 5000]}]} {
			puts "Unable to connect to server to update CFR cache."
			return;
		} else {
			set CFRTokenLookup($token) $section
		}
	}
}

#----------------------------------
proc handleCFRGet {token} {
	global CFRTokenLookup CFRTokenLookup appdir
	upvar #0 $token state

	set section $CFRTokenLookup($token)
	set filename "[regsub : $section cfr].txt"
	set directory [file join $appdir CFR]
	
	set contents ""
	if {$state(status) != "ok"} {
		puts "$state(status) while trying to get CFR: [http::error $token]"
	} else {
		set rawContents $state(body)
		regexp ".*<DOC>(.*)</PRE>.*" $rawContents all contents
		set contents [regsub {\n[ \t]*Authority:} $contents "\n^    Authority:"]
		set contents [regsub {\n[ \t]*Source:} $contents "\n^    Source:"]
		set contents [regsub {\n[ \t]*Editorial Note:} $contents "\n^    Editorial Note:"]
		set contents [regsub {\n[ \t]+ Alabama[^^]*} $contents "\n^\[States Table of Contents removed for brevity\]\n"]
		set outfile [file join $directory $filename]
		if {$contents != "" || [file exists $outfile] == 0} {
			set cachedFile [open $outfile "w"]
			puts $cachedFile $contents
			close $cachedFile
		}
	}
	http::cleanup $token
}