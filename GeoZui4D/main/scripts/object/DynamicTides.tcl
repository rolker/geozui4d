# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#-----------------------------------------------------------------------------
# DynamicTides procedures for updating tides via http
#
# Created by mdp 7/16/2004
#-----------------------------------------------------------------------------

package require http; #We're expecting at least 2.4, but don't bother checking

#If you have a proxy, uncomment the following lines and insert the appropriate
#info as necessary

#http::config -proxyhost {}
#http::config -proxyport {}

global GeoZuiVersion
http::config -useragent "GeoZui4D $GeoZuiVersion (Tcl http client)"


global dtURL dtOptions dtObject

#co-ops.nos.noaa.gov
set dtURL(Raw) "http://140.90.121.76/cgi-bin/co-ops_qry_direct.cgi?"
set dtURL(Predicted) "http://140.90.121.76/cgi-bin/co-ops_qry_wlpred.cgi?"

#note: shift of 0 indicates UTC (GMT) time, 1 indicates LST
#note: I assume a unit of 0 indicates meters, 1 indicates feet
#note: for raw, ssid indicates which sensor is being queried
#note: for predicted, mins indicates the interval of reporting in minutes
set dtOptions(Raw) { stn 8418150
				   dcp 1
				   ssid A1
				   pc W1
				   datum MLLW
				   unit 0
				   bdate 20040711
				   edate 20040712
				   date 3
				   shift 0
				   level -1
				   form 0
				   addr 132.177.44.65
				   data_type pwl
				   format View+Data}
set dtOptions(Predicted) { stn 8418150
						 unit 0
						 shift 0
						 mins 6
						 datum MLLW
						 bdate 20040710
						 edate 20040713
						 dcp 1
						 ssid PR
						 pc W1
						 format View+Data}

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
proc dtGetRawTides {callingObject days_ago args} {
	if {$days_ago < 0} {
		error "days_ago ($days_ago) cannot be negative for retrieving raw tides"
	}
	dtGetTides Raw $callingObject $days_ago 0 $args
}

#------------------------------------------------------------------------------
proc dtGetPredictedTides {callingObject days_ago days_from_today args} {
	if {-$days_ago > $days_from_today} {
		error "-days_ago ([expr -$days_ago]) cannot be greater than days_from_today ($days_from_today)"
	}
	dtGetTides Predicted $callingObject $days_ago $days_from_today $args
}

#------------------------------------------------------------------------------
# dtGetTides makes a url request for raw or predicted tide information, based
# on whether "Raw" or "Predicted" is passed in as the type.
# The args argument will eventually be used to allow other fields besides
# the date fields to be modified...
#------------------------------------------------------------------------------
proc dtGetTides {type callingObject days_ago days_from_today args} {
	global dtURL dtOptions dtObject

	set secondsInDay 86400
	set url $dtURL($type)
	set firstTime 1

	set now [clock seconds]
	set start [expr $now - ($days_ago * $secondsInDay)]
	set end [expr $now + ($days_from_today * $secondsInDay)]
	set bdate [clock format $start -format "%Y%m%d" -gmt 1]
	set edate [clock format $end -format "%Y%m%d" -gmt 1]

	foreach {field value} $dtOptions($type) {
		if {$firstTime} {
			set firstTime 0
		} else {
			append url "&"
		}
		if {$field == "bdate"} {
			append url "bdate\=$bdate"
		} elseif {$field == "edate"} {
			append url "edate\=$edate"
		} else {
			append url "$field\=$value"
		}
	}

	set command "dt$type"
	append command "TidesCallback"
	set token [http::geturl $url -command $command -timeout 90000]; #60-second timeout
	set dtObject($token) $callingObject
#	puts "requesting $url"
#	set token [http::geturl $url -timeout 60000]; #60-second timeout
#	dtPredictedTidesCallback $token
}

#------------------------------------------------------------------------------
# dtPredictedTidesCallback handles information retrieved from an http request
#------------------------------------------------------------------------------
proc dtPredictedTidesCallback {token} {
	global dtObject
	upvar #0 $token state

	if {$state(status) != "ok"} {
		puts "$state(status) while getting predited tides: [http::error $token]"
		#puts "Got so far: $state(body)"
		return
	}

	#grab the data portion of the page we got
#puts "----predicted:----\n$state(body)"
	if {[regexp "\n</B>(\n.*)" $state(body) match body] != 1} {
		puts "Unexpected reply from server.  First 200 characters:\n [string range $state(body) 0 200]"
		return
	}
	#Now that we have it, find starting info from it
	regexp {\n[0-9]*.([0-9]*).([0-9]*).([0-9]*)} $body match year month day
	set startTime [clock scan "$month/$day/$year" -gmt 1]
	#Now massage the body so it fits the expected format
	set numlines [regsub -all {\n[0-9]*.[0-9/]*.[0-9:]*[ \t]+} $body "\n" body]
	regsub -all {<[^>]*>} $body "" body
	regsub -all {\n\n+} $body "\n" body
	set outfile [open "[gz tellObject $dtObject($token) config predicted]" w]
	puts $outfile $numlines
	puts $outfile $startTime
	puts -nonewline $outfile 360.0
	puts $outfile $body
	close $outfile
	http::cleanup $token

	gz tellObject $dtObject($token) update predicted
	unset dtObject($token)
}

#------------------------------------------------------------------------------
# dtRawTidesCallback handles information retrieved from an http request
#------------------------------------------------------------------------------
proc dtRawTidesCallback {token} {
	global dtObject
	upvar #0 $token state

	if {$state(status) != "ok"} {
		puts "$state(status) while getting raw tides: [http::error $token]"
		return
	}

	#grab the data portion of the page we got
#puts "----raw:----\n$state(body)"
	if {[regexp "\n</B>(\n.*)" $state(body) match body] != 1} {
		puts "Unexpected reply from server.  First 200 characters:\n [string range $state(body) 0 200]"
		return
	}
	#Now that we have it, find starting info from it
	regexp {\n[0-9]*......([0-9]*).([0-9]*).([0-9]*)} $body match year month day
	set startTime [clock scan "$month/$day/$year" -gmt 1]
	#Now massage the body so it fits the expected format
	set numlines [regsub -all {\n[0-9]*......[0-9/]*.[0-9:]*[ \t]+} $body "\n" body]
	regsub -all {[ \t]+[0-9\. ]*\n} $body "\n" body
	regsub -all {<[^>]*>} $body "" body
	regsub -all {\n\n+} $body "\n" body
	set outfile [open "[gz tellObject $dtObject($token) config raw]" w]
	puts $outfile $numlines
	puts $outfile $startTime
	puts -nonewline $outfile 360.0
	puts $outfile $body
	close $outfile
	http::cleanup $token

	gz tellObject $dtObject($token) update raw
	unset dtObject($token)
}
