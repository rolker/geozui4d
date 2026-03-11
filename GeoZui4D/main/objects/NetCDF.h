// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef NETCDF_H
#define NETCDF_H

#include"engine/ObjectCollection.h"

class NetCDF : public ObjectCollection
{
public:
	DERIVE_OBJECT(NetCDF);
	
	NetCDF();
	
	int Load();
	
	Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
	int save(char * path) {return TCL_OK;};
private:
	void DumpHeader();

	gz4d::netcdf::File ncf;
};

#endif
