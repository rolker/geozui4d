// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef TEXT_H
#define TEXT_H

#include"engine/Object.h"

class DXF;

#define TEXT_SPACE_WIDTH 0.5
#define TEXT_INTER_CHAR_WIDTH 0.1
#define TEXT_LINEFEED_HEIGHT 1.5

class Text: public Object
{
public:
	Text();
	~Text();
	DERIVE_OBJECT(Text);
	
	Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
	void DrawLowRes(float degradation, Window3D *win);
        void setText(std::string t);
private:
	void loadLetter(char * fname,char i);

	enum Justification{Left,Right,Center};
		
        std::string text;
	Point3d color;
	float * lineLengths;
	int lineCount;
	
	int justification;

	static DXF * ascii[128];
	static int asciiInitialized;
	
};

#endif
