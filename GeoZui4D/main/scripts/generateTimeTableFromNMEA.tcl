#------------------------------------------------------------------------------
# generateTimeTableFromNMEA.tcl -- convert NMEA files to spreadsheet layouts
#
# Created by mdp 6/3/2005
#------------------------------------------------------------------------------
global appdir
source "$appdir/scripts/support/scriptSupport.tcl"

#------------------------------------------------------------------------------
script generateTimeTableFromNMEA {
	variable variables {filename	""					"Input file"			file
						outfilename	"nmea.log"			"Output file"			text
						timeKey		"GGA"				"Key time from message"	text
						timeStep	"1"					"Keyed time step"		number
						generateDat	"1"					"Generate .data file"	bool
						currentDir	"0"					"Direction of current"	number
						currentStr	"0"					"Strength of current"	number
						forecastLen	"10"				"Forecast length in .data file"	number
						generateSPSS "1"				"Generate SPSS files"	bool
						filterSPSS	"sog >= 3 & rudder <= 15 & rudder >=  - 15"			"SPSS filter"			text
					   }

	trace variable filename w [namespace code {set outfilename "[file rootname $filename].log";#}]
	trace variable generateDat w [namespace code {set generateSPSS $generateDat;#}]

	conversionToplevel $variables $scriptName "Generate time table from NMEA"

	#---Conversion procedure---
	proc convert {} {
		variable variables
		variable scriptName
		set linesRead 0
		set match 0
		set currentTime 0
		set messages ""
		set vars ""

		set fieldsCare "GGA HDT RSA VTG"
		set fields(HDT) "heading"
		set nFieldsCare(HDT) 1
		set fields(RSA) "rudder"
		set nFieldsCare(RSA) 1
		set fields(RMC) "junk junk lat junk lon junk sog cog"
		set nFieldsCare(RMC) 8
		set fields(GGA) "time lat junk lon"
		set nFieldsCare(GGA) 4
		set fields(VTG) "cog junk junk junk sog"
		set nFieldsCare(VTG) 5

		set degreeFields "heading rudder cog"

		foreach {var val lab type} $variables {
			variable $var
		}
		set infile [open "$filename" "r"]

		update

		#---Pass 1: find all NMEA strings used---
		while {[eof $infile] != 1} {
			set inline [gets $infile]
			incr linesRead
			if {[eof $infile] == 1} break;
			if {[expr $linesRead % 1000] == 0} {
				.$scriptName.status configure -text "Pass 1: Processing line $linesRead"
				update
			}
			if {[regexp {\$(..)(...),.*} $inline match source message]} {
				if {([lsearch $fieldsCare $message] != -1)} {
					if {[lsearch $messages $message] == -1} {
						lappend messages $message
						set sources($message) $source
					} else {
						if {[lsearch $sources($message) $source] == -1} {
							lappend sources($message) $source
						}
					}
				}
			}
		}
		close $infile

		# fill vars with all the variables we need to reset
		foreach message $messages {
			foreach var $fields($message) {
				if {[lsearch $vars $var] == -1} {
					lappend vars $var
				}
			}
			set pattern($message) "regexp {"
			for {set i 0} {$i < $nFieldsCare($message)} {incr i} {
				append pattern($message) "(\[^,\$\]*),"
			}
			append pattern($message) "} \$values match " $fields($message)
		}
		foreach var $vars {
			set $var 0;  #basic
			set t$var 0; #totals
			set n$var 0; #count
			set last$var 0; #previous
		}
		set linesRead 0

		# Ask user which messages to use when there are duplicates
		set messageSpec {}
		foreach message $messages {
			set len [llength $sources($message)]
			if {$len < 2} continue;
			append messageSpec "$message {$sources($message)} $message radio "
		}
		if {$messageSpec != {}} {
			parameterToplevel $messageSpec $scriptName "Select duplicate messages to use" "Some NMEA strings are being produced by multiple NMEA sources.  Please select which source you would like to use for each string.  If you do not make a selection, all sources for that string will be used."
			#after parameterToplevel goes away, the sources we want to keep are here!
			foreach {var val lab type} $messageSpec {
				variable $var
				eval "set sources($lab) \$$var"
				#puts "sources($lab) is $sources($lab)"
			}
		}

		#---Pass 2: Process the file
		set infile [open "$filename" "r"]
		set outfile [open "$outfilename" "w"]
		set outline "t"
		foreach var $vars {
			if {$var != "junk"} {
				append outline " $var"
			}
		}
		puts $outfile "$outline"
		while {[eof $infile] != 1} {
			set inline [gets $infile]
			incr linesRead
			if {[eof $infile] == 1} break;
			if {[expr $linesRead % 1000] == 0} {
				.$scriptName.status configure -text "Pass 2: Processing line $linesRead"
				update
			}
			if {[regexp {\$(..)(...),(.*)} $inline match source message values]} {
				if {[lsearch $fieldsCare $message] == -1} continue;
				if {[lsearch $sources($message) $source] == -1} {
					#puts "ignoring $inline"
					continue;
				}
				if {[eval $pattern($message)]} {
					foreach var $fields($message) {
						if {$var == "junk"} continue;
						eval "set val \$$var"
						if {([string index $val 0] == "0") && ([string index $val 1] != ".")} {
							set val [string trimleft $val "0"]
						}
						if {([string index $val 0] == "-") && ([string index $val 1] == "0") && ([string index $val 2] != ".")} {
							set val "-[string trimleft [string trimleft $val "-"] "0"]"
						}
						if {$val == "-" || $val == ""} {
							set val 0
						}
						if {[lsearch $degreeFields $var] >= 0} {
							#This will hopefully get rid of any strange averages across the 360 degree boundary
							eval "set pval \$last$var"
							#puts "Attempting to take ($pval - $val), at time $time, field $var of message $message"
							if {($pval - $val) > 180} {
								set $val [expr $val + 360]
							} elseif {($pval - $val) < -180} {
								set $val [expr $val - 360]
							}
						}
						set $var $val
						#increment total and counter
						if {[catch {set t$var [eval "expr \$$var + \$t$var"]}] == 0} {
							incr n$var
						}
					}
				}
				if {$message == $timeKey} {
					#dump out a line of info
					incr currentTime
					set outline "$currentTime"
					foreach var $vars {
						if {$var == "junk"} continue;
						if {[eval "expr \$n$var + 0"] > 0} {
							set last$var [eval "expr \$t$var / \$n$var"]
						}
						lappend outline [eval "expr \$last$var"]
						set t$var 0
						set n$var 0
					}
					puts $outfile $outline
				}
			}
		}
		close $infile
		close $outfile

		if {$generateDat} {
			set predictors {cog heading dist dcog}; #dcog must come after dist if it appears at all
			set dataFilename "[file rootname $outfilename].data"
			set currentX [expr $currentStr * sin($currentDir * 3.1415927 / 180.0)]
			set currentY [expr $currentStr * cos($currentDir * 3.1415927 / 180.0)]
			set columnNames [expandToDat "$outfilename" "$dataFilename" $forecastLen $predictors $currentX $currentY]
			if {$generateSPSS} {
				createSPSSscripts $dataFilename $columnNames $filterSPSS $forecastLen $predictors
			}
		}
		.$scriptName.status configure -text "Ready"
		update
	}

	proc angdiff {a1 a2} {
		set diff [expr $a1 - $a2]
		if {$diff > 180} {
			return [expr $diff - 360]
		} elseif {$diff < -180} {
			return [expr $diff + 360]
		}
		return $diff
	}

	proc decimalDegrees {val} {
		if {$val == ""} {return 0}
		set intPart [expr int($val / 100)]
		set remainder [expr $val - ($intPart * 100)]
		return [expr $intPart + ($remainder / 60.0)]
	}

	proc expandToDat {infilename outfilename fLen predictors {currentX 0} {currentY 0}} {
		variable scriptName
		set infile [open "$infilename" "r"]
		set outfile [open "$outfilename" "w"]
		set cogVel 0
		
		#grab the headings and find out which column special ones are in
		set headings [gets $infile]
		foreach h {rudder cog heading lat lon sog time} {
			set index_$h [lsearch $headings $h]
		}

		#output new headings
		set outline "$headings x y dist rudder_m1 rudder_m2 Inst_HAV EstCOGv EstCOGa"
		foreach h $predictors {
			for {set i 1} {$i <= $fLen} {incr i} {
				append outline " $h\_D$i"
			}
		}
		puts $outfile "$outline"
		set retval "$outline"

		set linesRead 0
		set hasDistPredictor 0
		set hasDcogPredictor 0

		#initialize the table of values
		set pi 3.1415927
		for {set i 0} {$i <= $fLen} {incr i} {
			if {$i == 0} {
				set line($i) [gets $infile]
				#initialize distance-calculation variables
				gz projection init
				set utmZone [gz projection longToUTM [decimalDegrees [lindex $line(0) $index_lon]]]
				foreach {xOffset yOffset} [gz projection apply $utmZone [decimalDegrees [lindex $line(0) $index_lon]] [decimalDegrees [lindex $line(0) $index_lat]]] {}
				set lastX 0
				set lastY 0
				set a_x(0) $xOffset
				set a_y(0) $yOffset
				set a_dcog(0) 0
				set lastTime [lindex $line(0) $index_time]
			} else {
				set dt 0.0
				while {$dt < 0.95} {
					set line($i) [gets $infile]; #skip to every second for our our purposes here
					set newTime [lindex $line($i) $index_time]
					set dt [expr fmod($newTime - $lastTime, 60)]
					if {[eof $infile] == 1} break
				}
				set lastTime $newTime
				foreach h $predictors {
					set iMinus1 [expr $i - 1]
					if {$h == "dist"} {
						#'a' signifies absolute, 'd' signifies delta
						set hasDistPredictor 1
						foreach {ax ay} [gz projection apply $utmZone [decimalDegrees [lindex $line($i) $index_lon]] [decimalDegrees [lindex $line($i) $index_lat]]] {}
						set a_x($i) [expr $ax - ($currentX * $dt)]
						set a_y($i) [expr $ay - ($currentY * $dt)]
						set dx [format "%.2f" [expr $ax - $a_x(0)]]
						set dy [format "%.2f" [expr $ay - $a_y(0)]]
						set d_dist($iMinus1) [format "%.2f" [expr sqrt($dx * $dx + $dy * $dy)]]
					} elseif {$h == "dcog"} {
						set hasDcogPredictor 1
						set a_dcog($i) [format "%.2f" [expr atan2($dx, $dy) * -180 / $pi]]
						set d_dcog($iMinus1) [angdiff $a_dcog($i) $a_dcog($iMinus1)]
					} else {
						eval "set index \$index_$h"
						eval "set d_$h\($iMinus1) [angdiff [lindex $line($i) $index] [lindex $line($iMinus1) $index]]"
					}
				}
			}
			set line(-1) $line(0)
			incr linesRead
		}

		while {[eof $infile] != 1} {
			#bump table of values up and read in the next line
			for {set i -2} {$i < $fLen} {incr i} {
				set line($i) $line([expr $i + 1])
				if {($i >= 0) && ($i < $fLen - 1)} {
					set iPlus1 [expr $i + 1]
					set iMinus1 [expr $i - 1]
					foreach h $predictors {
						eval "set d_$h\($i) \$d_$h\($iPlus1)"
						if {$h == "dist"} {
							set dt [expr fmod([lindex $line($i) $index_time] - [lindex $line($iMinus1) $index_time], 60)]
							set a_x($i) $a_x($iPlus1)
							set a_y($i) $a_y($iPlus1)
							#we need to recalculate d_dist each time because it is a 2D summary value.
							set dx [format "%.2f" [expr $a_x($i) - $a_x(0)]]
							set dy [format "%.2f" [expr $a_y($i) - $a_y(0)]]
							set d_dist($iMinus1) [format "%.2f" [expr sqrt($dx * $dx + $dy * $dy)]]
						} elseif {$h == "dcog"} {
							set a_dcog($i) $a_dcog($iPlus1)
						}
					}
				}
			}
			if {$hasDistPredictor} {
				set a_x([expr $fLen - 1]) $a_x($fLen)
				set a_y([expr $fLen - 1]) $a_y($fLen)
			}
			if {$hasDcogPredictor} {
				set a_dcog([expr $fLen - 1]) $a_dcog($fLen)
			}
			set dt 0.0
			while {$dt < 0.95} {
				set line($fLen) [gets $infile]; #skip to every second for our our purposes here
				set newTime [lindex $line($fLen) $index_time]
				set dt [expr fmod($newTime - $lastTime, 60)]
				if {[eof $infile] == 1} break
			}
			set lastTime $newTime
			foreach h $predictors {
				set iMinus1 [expr $fLen - 1]
				if {$h == "dist"} {
					#'a' signifies absolute, 'd' signifies delta
					foreach {ax ay} [gz projection apply $utmZone [decimalDegrees [lindex $line($fLen) $index_lon]] [decimalDegrees [lindex $line($fLen) $index_lat]]] {}
					set dt [expr fmod([lindex $line($i) $index_time] - [lindex $line($iMinus1) $index_time], 60)]
					set a_x($i) [expr $ax - ($currentX * $dt)]
					set a_y($i) [expr $ay - ($currentY * $dt)]
					set dx [format "%.2f" [expr $ax - $a_x(0)]]
					set dy [format "%.2f" [expr $ay - $a_y(0)]]
					set d_dist($iMinus1) [format "%.2f" [expr sqrt($dx * $dx + $dy * $dy)]]
				} elseif {$h == "dcog"} {
					set a_dcog($fLen) [format "%.2f" [expr atan2($dx, $dy) * -180 / $pi]]
					set d_dcog($iMinus1) [angdiff $a_dcog($fLen) $a_dcog($iMinus1)]
					if {abs($d_dcog($iMinus1)) >= 15} {
						#This is done because the POS/MV seems to correct position by enough every 60 seconds to really throw off the COG direction
						set d_dcog($iMinus1) $d_dcog([expr $iMinus1 - 1])
					}
				} else {
					eval "set index \$index_$h"
					eval "set d_$h\($iMinus1) [angdiff [lindex $line($fLen) $index] [lindex $line($iMinus1) $index]]"
				}
			}
			incr linesRead
			if {[expr $linesRead % 100] == 0} {
				.$scriptName.status configure -text "Generating .data file: processing line $linesRead"
				update
			}
			set outline $line(0)
			foreach {x y} [gz projection apply $utmZone [decimalDegrees [lindex $line(0) $index_lon]] [decimalDegrees [lindex $line(0) $index_lat]]] {}
			set x [format "%.2f" [expr $x - $xOffset]]
			set y [format "%.2f" [expr $y - $yOffset]]
			set dx [expr $x - $lastX]
			set dy [expr $y - $lastY]
			set lastX $x
			set lastY $y
			append outline " $x $y"; #x and y in UTM as an offset from the first position
			append outline " [format "%.2f" [expr sqrt($dx * $dx + $dy * $dy)]]"; #distance travelled
			append outline " [lindex $line(-1) $index_rudder]"; #rudder at t - 1
			append outline " [lindex $line(-2) $index_rudder]"; #rudder at t - 2
			set val [angdiff [lindex $line(0) $index_heading] [lindex $line(-1) $index_heading]]
			append outline " $val"; #angular velocity of heading
			set val [angdiff [lindex $line(0) $index_cog] [lindex $line(-1) $index_cog]]
			append outline " $val"; #angular velocity of cog
			set acc [angdiff $val $cogVel]
			set cogVel $val
			append outline " $acc"; #angular acceleration of cog
			foreach h $predictors {
				eval "set val \$d_$h\(0)"
				append outline " $val"; #put the first-level difference out
				for {set i 1} {$i < $fLen} {incr i} {
					if {$h == "dist"} {
						set val $d_dist($i)
					} else {
						eval "set val \[expr $val + \$d_$h\($i)\]"
					}
					append outline " $val"
				}
			}
			puts $outfile "$outline"
		}

		close $infile
		close $outfile

		return $retval
	}

	proc createSPSSscripts {infilename columnNames filter fLen predictors} {
		set syntaxFile "[file rootname $infilename].jnl"
		set saveFile "[file rootname $infilename].sav"
		set batchFile "[file rootname $infilename].spp"

		#---First, create the syntax file---
		set sFile [open $syntaxFile "w"]
		puts $sFile "GET DATA  /TYPE = TXT"
		puts $sFile " /FILE = '$infilename'"
		puts $sFile " /DELCASE = LINE"
		puts $sFile " /DELIMITERS = \" \""
		puts $sFile " /ARRANGEMENT = DELIMITED"
		puts $sFile " /FIRSTCASE = 2"
		puts $sFile " /IMPORTCASE = ALL"
		puts $sFile " /VARIABLES ="
		foreach var $columnNames {
			if {$var == "time"} {
				puts $sFile " $var F8.2"
			} elseif {$var == "lat" || $var == "lon"} {
				puts $sFile " $var F12.2"
			} else {
				puts $sFile " $var F3.2"
			}
		}
		puts $sFile " ."
		puts $sFile "CACHE."
		puts $sFile "EXECUTE."
		puts $sFile "USE ALL."
		puts $sFile "COMPUTE filter_\$=($filter)."
		puts $sFile "VARIABLE LABEL filter_\$ '$filter (FILTER)'."
		puts $sFile "VALUE LABELS filter_\$  0 'Not Selected' 1 'Selected'."
		puts $sFile "FORMAT filter_\$ (f1.0)."
		puts $sFile "FILTER BY filter_\$."
		puts $sFile "EXECUTE ."
		puts $sFile "SAVE OUTFILE='$saveFile'"
		puts $sFile " /COMPRESSED."
		foreach h $predictors {
			for {set i 1} {$i <= $fLen} {incr i} {
				puts $sFile "REGRESSION"
				puts $sFile " /MISSING LISTWISE"
				puts $sFile " /STATISTICS COEFF OUTS CI R ANOVA"
				puts $sFile " /CRITERIA=PIN(.05) POUT(.10)"
				puts $sFile " /NOORIGIN"
				puts $sFile " /DEPENDENT $h\_D$i"
				if {$h == "cog" || $h == "dist" || $h == "dcog"} {
					puts $sFile " /METHOD=ENTER rudder rudder_m1 rudder_m2 Inst_HAV EstCOGv EstCOGa  ."
				} elseif {$h == "heading"} {
					puts $sFile " /METHOD=ENTER rudder rudder_m1 rudder_m2 Inst_HAV  ."
				}
			}
		}
		close $sFile

		#---Now, create the production batch file---
		set bFile [open $batchFile "w"]
		puts $bFile "*Creator/owner: generateTimeTableFromNMEA."
		puts $bFile "*Date: [clock format [clock seconds] -format %m/%d/%Y]."
		puts $bFile ""
		puts $bFile "SET MXCELLS=AUTOMATIC ."
		puts $bFile "INSERT FILE='$syntaxFile' SYNTAX=BATCH ERROR=STOP."
		puts $bFile ""
		puts $bFile "*Comments:."
		puts $bFile "* ."
		puts $bFile ""
		puts $bFile "*Output Folder: [file dirname $infilename]."
		puts $bFile "*Exported Chart Format: JPEG File."
		puts $bFile "*Exported File Format: 2."
		puts $bFile "*Export Objects: 1."
		puts $bFile "*Output Type: 0."
		puts $bFile "*Print output on completion: Off."
		puts $bFile ""
		puts $bFile "*PublishToWeb Cfg GUID: ."
		puts $bFile "*PublishToWeb Objects: 6."
		puts $bFile "*PublishToWeb Table: 1."
		puts $bFile "*PublishToWeb UsrID: ."
		puts $bFile "*PublishToWeb Authentication: ."
		close $bFile
	}
}