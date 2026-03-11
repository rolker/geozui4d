// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef S57_OBJECT_H
#define S57_OBJECT_H

#include<gz4d/glo.h>
#include"engine/Object.h"
#include <vector>
#include <map>
#include <string>

class Scene3D;

struct S57_LayerInfo
{
    std::string acronym;
    std::string name;
    const ObjectRegistrationEntry **reg;
    std::string defaultLabel;
    S57_LayerInfo();
    S57_LayerInfo(std::string const &a, std::string const &n, const ObjectRegistrationEntry **r, std::string const &dl);
};

class S57_LayerMap
{
    std::map<std::string,S57_LayerInfo> info;
    public:
        S57_LayerMap();
        S57_LayerInfo const &operator[](std::string const &a);
};

class S57_object : public Object
{
public:
    typedef std::map<std::string, S57_object *> LnamTable;
	S57_object();
	~S57_object();
	DERIVE_OBJECT(S57_object);
	
	static S57_LayerInfo const &getLayerInfo(std::string const &acronym);

	virtual void DrawLowRes(float degradation, Window3D * win);
	virtual void ShowInfo(GLuint *names = NULL, int numNames = 0);
	virtual void ShowS57Info(Scene3D *scene);

	void DecodeColour(int code, Point3f &colour);
	void SetColour(Point3f c);
	virtual void InitializeFor(std::string const &acronym) {};
	virtual void SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber);

//geometry
        void ProcessGeometry(gz4d::ogr::Geometry geo, gz4d::ogr::SpatialReference spatialRef);


//bookkeeping
    std::string const &getLNAM() {return lnam;}
    virtual void resolveReferences(LnamTable &lnam_table, std::string const &s57file);

protected:
    void parseIntList(std::string listAsStrings, std::vector<int> &listAsInts);
    bool generateSpecialPurposeString(char *targetString); //returns true if generated something

    static const char colorName[14][8];
    GLboolean useColor;
	Point3f colour;
    int colorCode;
    int colorPattern;
    std::vector<int> categoryList; //CATLIT, CATFOG, CATSPM, etc...  May have multiple values
    std::vector<int> colorCodeList;
    int status, conspicuous, radarConspicuous;
    int condition, construction, product, waterLevel;
	double pointSize;
	double lineWidth;



        std::vector<gz4d::Point<float> > exteriorRing;
        bool exteriorRingClockwise;
        bool isClosed;
        bool isLine;

        std::vector<std::vector<gz4d::Point<float> > > interiorRings;
        std::vector<bool> interiorRingClockwise;


	std::string inform;
    std::string textDescription; //TXTDSC field (starts as filename, ends up with file contents)

    //naming (LNAM, LNAM_REFS fields)
    //unsigned long long lnam;
    //std::vector<uint64_t> lnam_refs;
    std::string lnam;
    std::vector<std::string> lnam_refs;

    float scamin;

    double height;

    static S57_LayerMap layerInfo;
};


#endif
