/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef DXF_THING_H
#define DXF_THING_H

#include"DXF_reader.h"
#include"engine/Persistence.h"

///\ingroup DXFGroup
class DXF_thing:public DXF_reader
{
public:
	DXF_thing();
	virtual ~DXF_thing();
	void setHandle(char * h);
	void setHandle(int h);
	int getHandle(){return handle;}
        std::string const &getLayer() const {return layer;};
        std::string const &getName() const {return name;};
	virtual int processEntry();

protected:
	int handle;
	std::string layer;
        std::string name;
};

#endif
