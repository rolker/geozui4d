// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_READER_H
#define DXF_READER_H

#include<stdio.h>
#include<map>
#include <string>

class DXF_entity;

///\ingroup DXFGroup
class DXF_reader
{
public:
	DXF_reader();
protected:
	int readNext();
	void savePosition();
	void restorePosition();
	DXF_entity * processEntity();

	long fileposition;
	
	static char valueString[260];
	static int groupCode;
	static FILE * dxf;
	
	static float red[256];
	static float green[256];
	static float blue[256];

	static float angbase; ///< Base for angle meseurments. 0 is east.
	static int clockwise; ///< Angle direction flag.
	
        static std::map<std::string,bool> warnings;
};

#endif

