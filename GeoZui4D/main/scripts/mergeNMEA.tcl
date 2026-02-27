#------------------------------------------------------------------------------
# mergeNMEA.tcl -- merge two NMEA files into one
#
# Created by mdp 6/22/2005
#------------------------------------------------------------------------------
global appdir
source "$appdir/scripts/support/scriptSupport.tcl"

#------------------------------------------------------------------------------
script mergeNMEA {
	variable variables {filename1	""					"Input file 1"			file
						filename2	""					"Input file 2"			file
						outfilename	"nmea.txt"			"Output file"			text
						stripGZtags	"0"					"Strip @GZ tags"		bool
					   }

	trace variable filename1 w [namespace code {set outfilename "[file rootname $filename1]_merge.txt";#}]

	conversionToplevel $variables $scriptName "Merge NMEA Files"

	proc findGZTOF {infile} {
		set inline [gets $infile]
		while {[regexp {@GZTOF,(.*)} $inline match value] == 0} {
			if {[eof $infile] == 1} {
				return {}
			}
			set inline [gets $infile]
		}
		return $value
	}

	proc findGZTIM {infile} {
		set inline [gets $infile]
		while {[regexp {@GZTIM,(.*)} $inline match value] == 0} {
			if {[eof $infile] == 1} {
				return {}
			}
			set inline [gets $infile]
		}
		return $value
	}

	proc getGZTIM {inline} {
		if {[regexp {@GZTIM,(.*)} $inline match value] == 0} {
			return {}
		}
		return $value
	}

	#---Conversion procedure---
	proc convert {} {
		variable variables
		variable scriptName

		set linesRead 0

		foreach {var val lab type} $variables {
			variable $var
		}

		set infile1 [open "$filename1" "r"]
		set infile2 [open "$filename2" "r"]
		set outfile [open "$outfilename" "w"]

		update

		set timeOffset1 [findGZTOF $infile1]
		set timeOffset2 [findGZTOF $infile2]

		if {$timeOffset1 == {} || $timeOffset2 == {}} {
			.$scriptName.status configure -text "Missing GZTOF line in one or both files!"
			update
			bell
			return
		}

		set offset [findGZTIM $infile1]
		if {$offset == {}} {
			.$scriptName.status configure -text "Missing GZTIM lines in $filename1!"
			update
			bell
			return
		}
		set cTime1 [expr $offset + $timeOffset1]

		set offset [findGZTIM $infile2]
		if {$offset == {}} {
			.$scriptName.status configure -text "Missing GZTIM lines in $filename2!"
			update
			bell
			return
		}
		set cTime2 [expr $offset + $timeOffset2]

		if {$timeOffset1 <= $timeOffset2} {
			set offset $timeOffset1
		} else {
			set offset $timeOffset2
		}

		if {!$stripGZtags} {
			puts $outfile "@GZTOF,$offset"
		}

		#---Merge the files---
		while {[eof $infile1] != 1 && [eof $infile2] != 1} {
			if {($cTime1 <= $cTime2 && [eof $infile1] != 1) || ([eof $infile2] == 1)} {
				set inline [gets $infile1]
				set val [getGZTIM $inline]
				if {$val != {}} {
					set cTime1 [expr $val + $timeOffset1]
					if {!$stripGZtags} {
						if {$cTime1 <= $cTime2} {
							puts $outfile "@GZTIM,[format "%.3f" [expr $cTime1 - $offset]]";
						} else {
							puts $outfile "@GZTIM,[format "%.3f" [expr $cTime2 - $offset]]"; #set up to switch to file2
						}
					}
					continue;
				}
			} else {
				set inline [gets $infile2]
				set val [getGZTIM $inline]
				if {$val != {}} {
					set cTime2 [expr $val + $timeOffset2]
					if {!$stripGZtags} {
						if {$cTime2 < $cTime1} {
							puts $outfile "@GZTIM,[format "%.3f" [expr $cTime2 - $offset]]";
						} else {
							puts $outfile "@GZTIM,[format "%.3f" [expr $cTime1 - $offset]]"; #set up to switch to file1
						}
					}
					continue;
				}
			}
			incr linesRead
			if {[regexp {(\$[^\$]*)(\$[^\$]*)} $inline match line1 line2] == 1} {
				puts $outfile "$line1"
				puts $outfile "$line2"
			} else {
				puts $outfile "$inline"
			}
			if {[expr $linesRead % 1000] == 0} {
				.$scriptName.status configure -text "Processed $linesRead NMEA lines"
				update
			}
		}
		close $infile1
		close $infile2
		close $outfile

		.$scriptName.status configure -text "Ready"
		update
	}
}