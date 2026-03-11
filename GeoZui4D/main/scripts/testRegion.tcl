# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

set boundaries {Eastern Eastern_northside Mid_southern Northern Northern_eastside Northwestern Southern Southwestern}

foreach boundary $boundaries {
	gz loadobject "$appdir/scripts/test/$boundary.boundary"
}

source "$appdir/scripts/support/region.tcl"
region::init

region::create bounce1 {Southern_boundary Eastern_boundary Northern_eastside_boundary Southwestern_boundary}
region::create bounce2 {Southern_boundary Eastern_northside_boundary Northern_boundary Southwestern_boundary}
region::create bounce3 {Mid_southern_boundary Eastern_northside_boundary Northern_boundary Northwestern_boundary}

gz event handler create crossedOut for regionCrossedOut "myfunc %V %aM"
proc myfunc {region posable} {
	puts "$posable crossed out of $region"
}
gz tellObject .EventMediator.region push crossedOut

region::addPosable Window.main

region::createComplex AND & {bounce1 bounce2}
region::createComplex OR | {bounce1 bounce2 bounce3}
region::createComplex XOR ^ {bounce1 bounce2 bounce3}
region::createComplex NAND !& {bounce1 bounce2}
region::createComplex NOR !| {bounce1 bounce2 bounce3}
region::createComplex NXOR !^ {bounce1 bounce2 bounce3}
