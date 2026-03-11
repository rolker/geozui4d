// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// FishSchool.h
/*---------------------------------------------------------------------------*/

/** 
 * This class represents a collection of sonar targets.  I have named this
 * class FishSchool because its main purpose is to display fish.  In the
 * future some of the functionality in this class may be extracted to a base
 * class.
 *
 *	Created by blocke June 9, 2006
 */
#ifndef FISH_SCHOOL_H
#define FISH_SCHOOL_H

#include"engine/Object.h"
#include <gz4d/Point.h>
#include <gz4d/time.h>

class FishSchool: public Object
{
public:
    /**
    * This structure represents a single fish object.  It contains position and
    * time offsets.  When combined with base position and time in the FishSchool 
    * the offset give the UTM coordinates and UTC time of the fish or target.
    */
#pragma pack(1)
    struct Fish
    {
    /// These are position offset coordinates in UTM.
        GLfloat position[3];

    // This is the UTC time offset.
        float time;

    /// This is extra info about the point.
    /// The default is to interpret it as RGB color
    /// but it can also be used to specify a normal vector.
        GLubyte extra[3];

    /// This is relative size parameter.
        GLubyte size;
    };
#pragma pack()

    /**
    * Default Constructor.
    */
    FishSchool(void);

    /**
     * Constructor with supplied filename.
     * Used by XYZStream to instantiate FishSchool to access the data
     * rather than as a visible object.
     */
    FishSchool(std::string const &fname);

    /**
        * Destructor.
        */
    ~FishSchool(void);
    
    /**
        * I have no idea what this does.  Will someone please document it?
        */
    DERIVE_OBJECT(FishSchool);
    
    /**
        * @see Object
        * The NetCDF file that contains the fish must be specified in the XML file,
        * using the Object's XML attribute 'targetfile'.  The time window can also
        * be specified using the attribute 'timewindow'.
        */
    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    void DrawLowRes(float degradation, Window3D *win);
    void Idle(double time);
    int perform(int argc, const char **argv, Tcl_Interp *interp);

    void AddFish(double lat, double lon, gz4d::Point<float> const &p, gz4d::Time t, float value);
    void AddFish(double e, double n, float z, gz4d::Time t, unsigned char e1, unsigned char e2, unsigned char e3, unsigned char size);

    long GetFishCount() {return fish.size();};
    double GetBaseTime() {return baseTime.secondsSinceEpoch().asSeconds();};
    double GetBaseNorthing() {return baseNorthing;};
    double GetBaseEasting() {return baseEasting;};
    Fish const & operator[](long index) {return fish[index];};

    void HandlePacket(std::shared_ptr<gz4d::Socket::Packet> &data);
    void getHtmlInfo(std::stringstream &html, short level);

private:
    enum extraType
    {
        COLOR,
        NORMAL
    };

    /// Common initlizations tasks for constructors.
    void init();


    /// Resize the array to accomadate requested size or more.
    //bool Resize(unsigned long size);

    /// Name of the NetCDF file where the fish data are stored.
    fs::path fishFilename;

    /// The array of fish.
    std::vector<Fish> fish;

    /// Index of the first fish that should be drawn.
    unsigned long fishBeginIndex;

    /// Index of the last fish that should be drawn.
    unsigned long fishLimitIndex;

    /// The base time for the fish.
    gz4d::Time baseTime;

    /// The last GeoZui idle time.
    double idleTime;

    /// This is the time window (s) used to determine which targets will drawn.
    double timeWindow;

    /// The UTM zone.
    int baseZone;

    /// The base easting in the UTM zone.
    double baseEasting;

    /// The base northing in the UTM zone.
    double baseNorthing;

    /**
    * This method initializes the fish data variables from a NetCDF file.
    * Presently, all of the target data in the NetCDF file is read into an array.
    * As you might imagine, this strategy will become unweildy as the number of
    * targets grow.  So, this will have to be reworked.
    */
    void initFishData();

    /// If this variable is set to true, then lighting should be enabled.
    bool enableLighting;

    bool useTextureColor; ///< Indicates if clut is used for color lookup.
    bool usePoly;         ///< Indicates if polygons are used instead of points.
    GLfloat baseSize;       ///< Base size (pixels for points, or meters for polygons)
    GLfloat sizeScale;      ///< Scales the data's size value. Set to 0 for constant size of baseSize

    GLuint displayLists;     ///< ID of first display list used to render polygon version of data.
    bool needListNames;    ///< Indicates that display list names still need to be generated.
    bool listsNeedBuild;   ///< Indicates the display lists need to be rebuilt
    //GLuint *listOffsets;   ///< Array of offsets used by glCallLists to specify subset of display lists to call.
    std::vector<GLuint> listOffsets;

    extraType extraFormat; ///< Specifies how the extra information is interpreted.
};

#endif
