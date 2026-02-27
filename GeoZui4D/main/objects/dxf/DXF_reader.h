/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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

