#-----------------------------------------------------------------------------
# region.tcl -- Support procedures for creating events whenever a region is
#				entered or exited.
#
# Short example for use:
#	region::init										;#must be called before
#														 #anything else
#	region::create myRegion {north south east west}		;#boundaries must be of
#														 #class Boundary
#	region::create myRegion2 {north2 south2 east west}
#	region::createComplex NANDregion !& {myRegion myRegion2} ;#valid operators:
#														;# &, |, ^, !&, !|, !^
#	gz event handler create crossedOut for regionCrossedOut "myfunc %V %aM"
#	#the regionCrossedIn/Out events yield a region/boundary list string in %V
#	#and a the address of the posable that crossed in %M.
#
#	region::addPosable $trackedPosable					;#Start tracking
#-----------------------------------------------------------------------------

namespace eval region {

	#namespace arrays & variables
	variable superRegionsOf
	variable regionsOf
	variable boundariesOf
	variable boundaryList {}
	variable regionList {}
	variable stateOfBoundary
	variable stateOfRegion
	variable posableList {}
	variable operationOf
	variable seedForOperation

	#============================================
	# Public interface functions
	#============================================
	#--------------------------------------------
	# Initializes the region detection code
	#--------------------------------------------
	proc init {} {
		variable seedForOperation

		set seedForOperation(|) 0
		set seedForOperation(&) 1
		set seedForOperation(^) 0
		gz event handler create boundaryCrossedIn for boundaryCrossedIn "region::boundary 1 %V %M"
		gz event handler create boundaryCrossedOut for boundaryCrossedOut "region::boundary 0 %V %M"
		gz event create regionCrossedIn
		gz event create regionCrossedOut
		gz event mediator create region
		gz event handler create regionCrossedIn for regionCrossedIn "region::region 1 %V %aM"
		gz event handler create regionCrossedOut for regionCrossedOut "region::region 0 %V %aM"
		region::enable
	}

	#--------------------------------------------
	# Enables region detection
	#--------------------------------------------
	proc enable {} {
		gz tellObject .EventMediator.user push boundaryCrossedIn
		gz tellObject .EventMediator.user push boundaryCrossedOut
		gz tellObject .EventMediator.region push regionCrossedIn
		gz tellObject .EventMediator.region push regionCrossedOut
	}

	#--------------------------------------------
	# Disables region detection
	#--------------------------------------------
	proc disable {} {
		gz tellObject .EventMediator.user remove boundaryCrossedOut
		gz tellObject .EventMediator.user remove boundaryCrossedIn
		gz tellObject .EventMediator.region remove regionCrossedOut
		gz tellObject .EventMediator.region remove regionCrossedIn
	}

	#--------------------------------------------
	# Creates a region determined by the passed
	# boundaries (formerly establishRegion)
	#--------------------------------------------
	proc create {region boundaries} {
		variable boundariesOf
		variable regionsOf
		variable operationOf
		variable regionList
		variable superRegionsOf

		set boundariesOf($region) $boundaries
		set operationOf($region) {}
		set superRegionsOf($region) {}
		lappend regionList $region
		foreach boundary $boundaries {
			lappend regionsOf($boundary) $region
			addBoundary $boundary
		}
		initializeRegionState $region
	}

	#--------------------------------------------
	# Creates a complex region determined by the
	# passed operation on the passed regions
	#--------------------------------------------
	proc createComplex {complexRegion operation regions} {
		variable boundariesOf
		variable regionsOf
		variable operationOf
		variable regionList
		variable superRegionsOf

		set boundariesOf($complexRegion) {}
		set regionsOf($complexRegion) $regions
		set operationOf($complexRegion) $operation
		set superRegionsOf($complexRegion) {}
		lappend regionList $complexRegion
		foreach region $regions {
			lappend superRegionsOf($region) $complexRegion
		}
		initializeRegionState $complexRegion
	}

	#--------------------------------------------
	# Adds a posable to the internal bookkeeping.
	# If it is already there, it is ignored.
	# (formerly toRegions add)
	#--------------------------------------------
	proc addPosable {posable} {
		variable posableList
		variable boundaryList

		#get fully-qualified name
		set posable [gz tellObject $posable getName]
		set index [lsearch -exact $posableList $posable]
		if {$index < 0} {
			lappend posableList $posable
			foreach boundary $boundaryList {
				gz tellObject .Posable.Object.$boundary manage $posable
			}
			initializeBoundaryStates $posable
			initializeRegionStates $posable
		}
	}

	#--------------------------------------------
	# Removes a posable from the internal
	# bookkeeping.  (formerly toRegions remove)
	#--------------------------------------------
	proc removePosable {posable} {
		variable posableList
		variable boundaryList

		set index [lsearch -exact $posableList $posable]
		if {$index > 0} {
			lreplace posableList $index $index
			foreach boundary $boundaryList {
				gz tellObject .Posable.Object.$boundary relinquish $posable
			}
		}
	}

	#--------------------------------------------
	# Returns "in" if the passed posable is in the
	# passed region, and "out" otherwise.  The
	# posable does not have to be registered.
	#--------------------------------------------
	proc stateOf {region posable {recalc 1}} {
		variable boundariesOf
		variable operationOf
		variable regionsOf
		variable seedForOperation
		variable stateOfRegion

		if {$operationOf($region) == ""} {
			foreach boundary $boundariesOf($region) {
				if {[gz tellObject .Posable.Object.$boundary regionOf $posable] != "in"} {
					return "out"
				}
			}
			return "in"
		} else {
			set operation $operationOf($region)
			if {[string index $operation 0] == "!"} {
				set not 1
				set op [string index $operation 1]
			} else {
				set not 0
				set op $operation
			}
			set result $seedForOperation($op)
			foreach subregion $regionsOf($region) {
				if {$recalc} {
					if {[stateOf $subregion $posable] == "in"} {
						set val 1
					} else {
						set val 0
					}
				} else {
					set val $stateOfRegion($subregion$posable)
				}
				set result [expr $result $op $val]
			}
			if {$not == 1} {
				set result [expr 1 - $result]
			}
			if {$result == 1} {
				return "in"
			} else {
				return "out"
			}
		}
	}

	#--------------------------------------------
	# Prints out a list of all registered
	# boundaries and their states
	#--------------------------------------------
	proc listBoundaryStates {} {
		variable boundaryList
		variable posableList
		variable stateOfBoundary

		foreach boundary $boundaryList {
			puts "$boundary:"
			foreach posable $posableList {
				puts "  $posable $stateOfBoundary($boundary$posable)"
			}
		}
	}

	#--------------------------------------------
	# Prints out a list of all regions and their
	# states
	#--------------------------------------------
	proc listRegionStates {} {
		variable regionList
		variable stateOfRegion

		foreach region $regionList {
			puts "$region:"
			foreach posable $posableList {
				puts "  $posable $stateOfRegion($region$posable)"
			}
		}
	}

	#============================================
	# Internal procedures
	#============================================
	#--------------------------------------------
	# Internal bookkeeping procedure for adding a
	# boundary to the tally, while making sure it
	# doesn't get added twice.
	#--------------------------------------------
	proc addBoundary {boundary} {
		variable boundaryList
		variable posableList

		if {[lsearch -exact $boundaryList $boundary] < 0} {
			lappend boundaryList $boundary
			foreach posable $posableList {
				gz tellObject .Posable.Object.$boundary manage $posable
			}
			initializeBoundaryState $boundary
		}
	}

	#--------------------------------------------
	# Internal bookkeeping procedure for updating
	# current knowledge about where the passed
	# posables are with respect to the passed
	# boundary.  If no posables are passed, the
	# procedure is run on all registered posables.
	#--------------------------------------------
	proc initializeBoundaryState {boundary {posables ""}} {
		variable stateOfBoundary
		variable posableList

		if {$posables == ""} {
			set posables $posableList
		}
		foreach posable $posables {
			if {[gz tellObject .Posable.Object.$boundary regionOf $posable] == "in"} {
				set stateOfBoundary($boundary$posable) 1
			} else {
				set stateOfBoundary($boundary$posable) 0
			}
		}
	}

	#--------------------------------------------
	# Internal bookkeeping procedure for updating
	# current knowledge about where the passed
	# posables are with respect to each boundary.
	# If no posables are passed, the procedure is
	# run on all registered posables.
	#--------------------------------------------
	proc initializeBoundaryStates {{posables ""}} {
		variable boundaryList
		variable stateOfBoundary
		variable posableList

		if {$posables == ""} {
			set posables $posableList
		}
		foreach boundary $boundaryList {
			initializeBoundaryState $boundary $posables
		}
	}

	#--------------------------------------------
	# Internal bookkeeping procedure for updating
	# current knowledge about where the passed
	# posables are with respect to the passed
	# region.  If no posables are passed, the
	# procedure is run on all registered posables.
	#--------------------------------------------
	proc initializeRegionState {region {posables ""}} {
		variable stateOfRegion
		variable posableList

		if {$posables == ""} {
			set posables $posableList
		}
		foreach posable $posables {
			if {[stateOf $region $posable] == "in"} {
				set stateOfRegion($region$posable) 1
			} else {
				set stateOfRegion($region$posable) 0
			}
		}
	}

	#--------------------------------------------
	# Internal bookkeeping procedure for updating
	# current knowledge about where the passed
	# posables are with respect to each region.
	# If no posables are passed, the procedure is
	# run on all registered posables.
	#--------------------------------------------
	proc initializeRegionStates {{posables ""}} {
		variable regionList
		variable stateOfRegion
		variable posableList

		if {$posables == ""} {
			set posables $posableList
		}
		foreach region $regionList {
			initializeRegionState $region $posables
		}
	}

	#============================================
	# CALLBACKS
	#============================================
	#--------------------------------------------
	# Internally registered callback code for when
	# a region's boundary has been crossed.
	# (formerly boundaryCrossed). Generates a
	# regionCrossed event.  %V is a list containing
	# the region and boundary, %aM is the address
	# of the crossing posable.
	#--------------------------------------------
	proc boundary {state boundary posable} {
		variable boundariesOf
		variable stateOfBoundary
		variable regionsOf
		variable stateOfRegion

		set boundaryName [string range $boundary [expr [string last . $boundary] + 1] end]
		set stateOfBoundary($boundaryName$posable) $state

		#---Check to see which regions have been crossed into/out of---
		foreach region $regionsOf($boundaryName) {
			set previousValue $stateOfRegion($region$posable)
			set inSoFar 1
			foreach bound $boundariesOf($region) {
				if {$stateOfBoundary($bound$posable) == 0} {
					if {$previousValue == 1} {
						gz tellObject .EventMediator.region trigger regionCrossedOut -s "{[list $region $boundary]}" -a $posable
					}
					set inSoFar 0
					break
				}
			}
			if {$inSoFar != 0} {
				if {$previousValue == 0} {
					gz tellObject .EventMediator.region trigger regionCrossedIn -s "{[list $region $boundary]}" -a $posable
				}
			}
		}
	}

	#--------------------------------------------
	# Internally registered callback code for when
	# a region has been crossed into or out of.
	#--------------------------------------------
	proc region {state regionList posable} {
		variable stateOfRegion
		variable superRegionsOf

		set region [lindex $regionList 0]
		set stateOfRegion($region$posable) $state
		foreach complexRegion $superRegionsOf($region) {
			set oldVal $stateOfRegion($complexRegion$posable)
			if {[stateOf $complexRegion $posable] == "in"} {
				set stateOfRegion($complexRegion$posable) 1
				if {$oldVal != 1} {
					gz tellObject .EventMediator.region trigger regionCrossedIn -s "{[linsert $regionList 0 $complexRegion]}" -a $posable
				}
			} else {
				set stateOfRegion($complexRegion$posable) 0
				if {$oldVal != 0} {
					gz tellObject .EventMediator.region trigger regionCrossedOut -s "{[linsert $regionList 0 $complexRegion]}" -a $posable
				}
			}
			initializeRegionState $complexRegion $posable
		}
	}
}
