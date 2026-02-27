#include "GZincludes.h"
#include"S57_object.h"
#include"S57.h"
#include <fstream>
#include "engine/ViewManager.h"
#include "engine/PosableStateBase.h"
#include "utilities/Utilities.h"
#include <gz4d/glo/PushAttrib.h>
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include <gz4d/geo/Projection.h>
#include <gz4d/glo/Vertex.h>

extern MainWindow * mainWindow;


REGISTER_OBJECT(S57_object,"");
KNOW_OBJECT(S57_area);
KNOW_OBJECT(S57_beacon);
KNOW_OBJECT(S57_bridge);
KNOW_OBJECT(S57_building);
KNOW_OBJECT(S57_buoy);
KNOW_OBJECT(S57_land);
KNOW_OBJECT(S57_landmark);
KNOW_OBJECT(S57_pipe);
KNOW_OBJECT(S57_sounding);
KNOW_OBJECT(S57_supplemental);

#define ADD_LAYER(acronym, name, createClass, prefix) info[acronym]=(S57_LayerInfo(acronym, name, &S57_##createClass##RegistrationEntry, prefix))
static const ObjectRegistrationEntry *S57_0RegistrationEntry = NULL;


S57_LayerInfo::S57_LayerInfo()
        :reg(NULL)
{
}

S57_LayerInfo::S57_LayerInfo(std::string const &a, std::string const &n, const ObjectRegistrationEntry **r, std::string const &dl)
        :acronym(a),name(n),reg(r),defaultLabel(dl)
{
}

S57_LayerMap S57_object::layerInfo;

S57_LayerMap::S57_LayerMap()
{
	ADD_LAYER("ADMARE","Administration area (Named)",0,"");
	ADD_LAYER("AIRARE","Airport / airfield",0,"");
	ADD_LAYER("ACHBRT","Anchor berth",0,"");
	ADD_LAYER("ACHARE","Anchorage area",area,"Anchor");
	ADD_LAYER("BCNCAR","Beacon, cardinal",beacon,"Cardinal beacon");
	ADD_LAYER("BCNISD","Beacon, isolated danger",beacon,"Isolated danger beacon");
	ADD_LAYER("BCNLAT","Beacon, lateral",beacon,"Lateral beacon");
	ADD_LAYER("BCNSAW","Beacon, safe water",beacon,"Safewater beacon");
	ADD_LAYER("BCNSPP","Beacon, special purpose/general",beacon,"Special purpose beacon");
	ADD_LAYER("BERTHS","Berth",0,"");
	ADD_LAYER("BRIDGE","Bridge",bridge,"Bridge");
	ADD_LAYER("BUISGL","Building, single",building,"Building");
	ADD_LAYER("BUAARE","Built-up area",0,"");
	ADD_LAYER("BOYCAR","Buoy, cardinal",buoy,"Unnamed buoy");
	ADD_LAYER("BOYINB","Buoy, installation",buoy,"Unnamed buoy");
	ADD_LAYER("BOYISD","Buoy, isolated danger",buoy,"Unnamed buoy");
	ADD_LAYER("BOYLAT","Buoy, lateral",buoy,"Unnamed buoy");
	ADD_LAYER("BOYSAW","Buoy, safe water",buoy,"Unnamed buoy");
	ADD_LAYER("BOYSPP","Buoy, special purpose/general",buoy,"Unnamed buoy");
	ADD_LAYER("CBLARE","Cable area",area,"Cable area");
	ADD_LAYER("CBLOHD","Cable, overhead",0,"");
	ADD_LAYER("CBLSUB","Cable, submarine",0,"");
	ADD_LAYER("CANALS","Canal",0,"");
	ADD_LAYER("CANBNK","Canal bank",0,"");
	ADD_LAYER("CTSARE","Cargo transshipment area",0,"");
	ADD_LAYER("CAUSWY","Causeway",0,"");
	ADD_LAYER("CTNARE","Caution area",area,"Caution Area");
	ADD_LAYER("CHKPNT","Checkpoint",0,"");
	ADD_LAYER("CGUSTA","Coastguard station",0,"");
	ADD_LAYER("COALNE","Coastline",0,"");
	ADD_LAYER("CONZNE","Contiguous zone",0,"");
	ADD_LAYER("COSARE","Continental shelf area",0,"");
	ADD_LAYER("CTRPNT","Control point",0,"");
	ADD_LAYER("CONVYR","Conveyor",0,"");
	ADD_LAYER("CRANES","Crane",0,"");
	ADD_LAYER("CURENT","Current - non - gravitational",0,"");
	ADD_LAYER("CUSZNE","Custom zone",0,"");
	ADD_LAYER("DAMCON","Dam",0,"");
	ADD_LAYER("DAYMAR","Daymark",supplemental,"Daymark");
	ADD_LAYER("DWRTCL","Deep water route centerline",0,"");
	ADD_LAYER("DWRTPT","Deep water route part",0,"");
	ADD_LAYER("DEPARE","Depth area",0,"");
	ADD_LAYER("DEPCNT","Depth contour",0,"");
	ADD_LAYER("DISMAR","Distance mark",0,"");
	ADD_LAYER("DOCARE","Dock area",0,"");
	ADD_LAYER("DRGARE","Dredged area",area,"Dredged Area");
	ADD_LAYER("DRYDOC","Dry dock",0,"");
	ADD_LAYER("DMPGRD","Dumping ground",0,"");
	ADD_LAYER("DYKCON","Dyke",0,"");
	ADD_LAYER("EXEZNE","Exclusive Economic Zone",0,"");
	ADD_LAYER("FAIRWY","Fairway",0,"");
	ADD_LAYER("FNCLNE","Fence/wall",0,"");
	ADD_LAYER("FERYRT","Ferry route",0,"");
	ADD_LAYER("FSHZNE","Fishery zone",0,"");
	ADD_LAYER("FSHFAC","Fishing facility",0,"");
	ADD_LAYER("FSHGRD","Fishing ground",0,"");
	ADD_LAYER("FLODOC","Floating dock",0,"");
	ADD_LAYER("FOGSIG","Fog signal",supplemental,"Fog signal");
	ADD_LAYER("FORSTC","Fortified structure",0,"");
	ADD_LAYER("FRPARE","Free port area",0,"");
	ADD_LAYER("GATCON","Gate",0,"");
	ADD_LAYER("GRIDRN","Gridiron",0,"");
	ADD_LAYER("HRBARE","Harbour area (administrative)",0,"");
	ADD_LAYER("HRBFAC","Harbour facility",0,"");
	ADD_LAYER("HULKES","Hulk",0,"");
	ADD_LAYER("ICEARE","Ice area",0,"");
	ADD_LAYER("ICNARE","Incineration area",0,"");
	ADD_LAYER("ISTZNE","Inshore traffic zone",area,"Inshore traffic");
	ADD_LAYER("LAKARE","Lake",0,"");
	ADD_LAYER("LAKSHR","Lake shore",0,"");
	ADD_LAYER("LNDARE","Land area",land,"Land");
	ADD_LAYER("LNDELV","Land elevation",0,"");
	ADD_LAYER("LNDRGN","Land region",0,"");
	ADD_LAYER("LNDMRK","Landmark",landmark,"Landmark");
	ADD_LAYER("LIGHTS","Light",supplemental,"Light");
	ADD_LAYER("LITFLT","Light float",0,"");
	ADD_LAYER("LITVES","Light vessel",0,"");
	ADD_LAYER("LOCMAG","Local magnetic anomaly",0,"");
	ADD_LAYER("LOKBSN","Lock basin",0,"");
	ADD_LAYER("LOGPON","Log pond",0,"");
	ADD_LAYER("MAGVAR","Magnetic variation",0,"");
	ADD_LAYER("MARCUL","Marine farm/culture",0,"");
	ADD_LAYER("MIPARE","Military practice area",0,"");
	ADD_LAYER("MORFAC","Mooring/warping facility",0,"");
	ADD_LAYER("NAVLNE","Navigation line",0,"");
	ADD_LAYER("OBSTRN","Obstruction",0,"");
	ADD_LAYER("OFSPLF","Offshore platform",0,"");
	ADD_LAYER("OSPARE","Offshore production area",0,"");
	ADD_LAYER("OILBAR","Oil barrier",0,"");
	ADD_LAYER("PILPNT","Pile",0,"");
	ADD_LAYER("PILBOP","Pilot boarding place",0,"");
	ADD_LAYER("PIPARE","Pipeline area",area,"Pipeline area");
	ADD_LAYER("PIPOHD","Pipeline, overhead",0,"");
	ADD_LAYER("PIPSOL","Pipeline, submarine/on land",0,"");
	ADD_LAYER("PONTON","Pontoon",0,"");
	ADD_LAYER("PRCARE","Precautionary area",0,"");
	ADD_LAYER("PRDARE","Production / storage area",0,"");
	ADD_LAYER("PYLONS","Pylon/bridge support",0,"");
	ADD_LAYER("RADLNE","Radar line",0,"");
	ADD_LAYER("RADRNG","Radar range",0,"");
	ADD_LAYER("RADRFL","Radar reflector",0,"");
	ADD_LAYER("RADSTA","Radar station",0,"");
	ADD_LAYER("RTPBCN","Radar transponder beacon",0,"");
	ADD_LAYER("RDOCAL","Radio calling-in point",0,"");
	ADD_LAYER("RDOSTA","Radio station",0,"");
	ADD_LAYER("RAILWY","Railway",0,"");
	ADD_LAYER("RAPIDS","Rapids",0,"");
	ADD_LAYER("RCRTCL","Recommended route centerline",0,"");
	ADD_LAYER("RECTRC","Recommended track",0,"");
	ADD_LAYER("RCTLPT","Recommended Traffic Lane Part",0,"");
	ADD_LAYER("RSCSTA","Rescue station",0,"");
	ADD_LAYER("RESARE","Restricted area",area,"Restricted Area");
	ADD_LAYER("RETRFL","Retro-reflector",0,"");
	ADD_LAYER("RIVERS","River",0,"");
	ADD_LAYER("RIVBNK","River bank",0,"");
	ADD_LAYER("ROADWY","Road",0,"");
	ADD_LAYER("RUNWAY","Runway",0,"");
	ADD_LAYER("SNDWAV","Sand waves",0,"");
	ADD_LAYER("SEAARE","Sea area / named water area",0,"");
	ADD_LAYER("SPLARE","Sea-plane landing area",0,"");
	ADD_LAYER("SBDARE","Seabed area",0,"");
	ADD_LAYER("SLCONS","Shoreline Construction",0,"");
	ADD_LAYER("SISTAT","Signal station, traffic",0,"");
	ADD_LAYER("SISTAW","Signal station, warning",0,"");
	ADD_LAYER("SILTNK","Silo / tank",0,"");
	ADD_LAYER("SLOTOP","Slope topline",0,"");
	ADD_LAYER("SLOGRD","Sloping ground",0,"");
	ADD_LAYER("SMCFAC","Small craft facility",0,"");
	ADD_LAYER("SOUNDG","Sounding",sounding,"Sounding group");
	ADD_LAYER("SPRING","Spring",0,"");
	ADD_LAYER("SQUARE","Square",0,"");
	ADD_LAYER("STSLNE","Straight territorial sea baseline",0,"");
	ADD_LAYER("SUBTLN","Submarine transit lane",0,"");
	ADD_LAYER("SWPARE","Swept Area",0,"");
	ADD_LAYER("TESARE","Territorial sea area",0,"");
	ADD_LAYER("TS_PRH","Tidal stream - harmonic prediction",0,"");
	ADD_LAYER("TS_PNH","Tidal stream - non-harmonic prediction",0,"");
	ADD_LAYER("TS_PAD","Tidal stream panel data",0,"");
	ADD_LAYER("TS_TIS","Tidal stream - time series",0,"");
	ADD_LAYER("T_HMON","Tide - harmonic prediction",0,"");
	ADD_LAYER("T_NHMN","Tide - non-harmonic prediction",0,"");
	ADD_LAYER("T_TIMS","Tidal stream - time series",0,"");
	ADD_LAYER("TIDEWY","Tideway",0,"");
	ADD_LAYER("TOPMAR","Top mark",supplemental,"Topmark");
	ADD_LAYER("TSELNE","Traffic Separation Line",0,"");
	ADD_LAYER("TSSBND","Traffic Separation Scheme  Boundary",0,"");
	ADD_LAYER("TSSCRS","Traffic Separation Scheme Crossing",0,"");
	ADD_LAYER("TSSLPT","Traffic Separation Scheme  Lane part",0,"");
	ADD_LAYER("TSSRON","Traffic Separation Scheme  Roundabout",0,"");
	ADD_LAYER("TSEZNE","Traffic Separation Zone",0,"");
	ADD_LAYER("TUNNEL","Tunnel",0,"");
	ADD_LAYER("TWRTPT","Two-way route  part",0,"");
	ADD_LAYER("UWTROC","Underwater rock / awash rock",0,"");
	ADD_LAYER("UNSARE","Unsurveyed area",0,"");
	ADD_LAYER("VEGATN","Vegetation",0,"");
	ADD_LAYER("WATTUR","Water turbulence",0,"");
	ADD_LAYER("WATFAL","Waterfall",0,"");
	ADD_LAYER("WEDKLP","Weed/Kelp",0,"");
	ADD_LAYER("WRECKS","Wreck",0,"");
	ADD_LAYER("TS_FEB","Tidal stream - flood/ebb",0,"");
	ADD_LAYER("M_ACCY","Accuracy of data",0,"");
	ADD_LAYER("M_CSCL","Compilation scale of data",0,"");
	ADD_LAYER("M_COVR","Coverage",0,"");
	ADD_LAYER("M_HDAT","Horizontal datum of data",0,"");
	ADD_LAYER("M_HOPA","Horizontal datum shift parameters",0,"");
	ADD_LAYER("M_NPUB","Nautical publication information",0,"");
	ADD_LAYER("M_NSYS","Navigational system of marks",0,"");
	ADD_LAYER("M_PROD","Production information",0,"");
	ADD_LAYER("M_QUAL","Quality of data",0,"");
	ADD_LAYER("M_SDAT","Sounding datum",0,"");
	ADD_LAYER("M_SREL","Survey reliability",0,"");
	ADD_LAYER("M_UNIT","Units of measurement of data",0,"");
	ADD_LAYER("M_VDAT","Vertical datum of data",0,"");
	ADD_LAYER("C_AGGR","Aggregation",0,"");
	ADD_LAYER("C_ASSO","Association",0,"");
	ADD_LAYER("C_STAC","Stacked on/stacked under",0,"");
	ADD_LAYER("$AREAS","Cartographic area",0,"");
	ADD_LAYER("$LINES","Cartographic line",0,"");
	ADD_LAYER("$CSYMB","Cartographic symbol",0,"");
	ADD_LAYER("$COMPS","Compass",0,"");
	ADD_LAYER("$TEXTS","Text",0,"");
	ADD_LAYER("", "", 0, "");
}

S57_LayerInfo const &S57_LayerMap::operator[](std::string const &a)
{
    if(info.count(a))
        return info[a];
    return info[""];
}

const char S57_object::colorName[14][8] = {"<none>", "White", "Black", "Red", "Green",
                                         "Blue", "Yellow", "Grey", "Brown", "Amber",
                                         "Violet", "Orange", "Magenta", "Pink"
                                        };

static const char colorPatternName[][32] = {"<none>",
                                            "Horizontal stripes",
                                            "Vertical stripes",
                                            "Diagonal stripes",
                                            "Squared",
                                            "Stripes (direction unknown)",
                                            "Border stripe"
                                           };

static const char statusName[][32] = {"<none>",
                                      "Permanent",
                                      "Occasional",
                                      "Recommended",
                                      "Not in use",
                                      "Periodic/intermittent",
                                      "Reserved",
                                      "Temporary",
                                      "Private",
                                      "Mandatory",
                                      "Destroyed/ruined",
                                      "Extinguished",
                                      "Illuminated",
                                      "Historic",
                                      "Public",
                                      "Synchronized",
                                      "Watched",
                                      "Un-watched",
                                      "Existence doubtful"
                                     };

static const char specialPurposeMarkName[][48] = {"<none>",
                                                  "Firing danger area mark",
                                                  "Target mark",
                                                  "Marker ship mark",
                                                  "Degaussing range mark",
                                                  "Barge mark",
                                                  "Cable mark",
                                                  "Spoil ground mark",
                                                  "Outfall mark",
                                                  "ODAS (Ocean-Data-Acquisition-System)",
                                                  "Recording mark",
                                                  "Seaplane anchorage mark",
                                                  "Recreation zone mark",
                                                  "Private mark",
                                                  "Mooring mark",
                                                  "LANBY (Large Automatic Navigational Buoy)",
                                                  "Leading mark",
                                                  "Measured distance mark",
                                                  "Notice mark",
                                                  "TSS mark (Traffic Separation Scheme)",
                                                  "Anchoring prohibited mark",
                                                  "Berthing prohibited mark",
                                                  "Overtaking prohibited mark",
                                                  "Two-way traffic prohibited mark",
                                                  "'Reduced wake' mark",
                                                  "Speed limit mark",
                                                  "Stop mark",
                                                  "General warning mark",
                                                  "'Sound ship's siren' mark",
                                                  "Restricted vertical clearance mark",
                                                  "Maximum vessel's draught mark",
                                                  "Restricted horizontal clearance mark",
                                                  "Strong current warning mark",
                                                  "Berthing permitted mark",
                                                  "Overhead power cable mark",
                                                  "'Channel edge gradient' mark",
                                                  "Telephone mark",
                                                  "Ferry crossing mark",
                                                  "Marine traffic lights",
                                                  "Pipeline mark",
                                                  "Anchorage mark",
                                                  "Clearing mark",
                                                  "Control mark",
                                                  "Diving mark",
                                                  "Refuge beacon",
                                                  "Foul ground mark",
                                                  "Yachting mark",
                                                  "Heliport mark",
                                                  "GPS mark",
                                                  "Seaplane landing mark",
                                                  "Entry prohibited mark",
                                                  "Work in progress mark",
                                                  "Mark with unknown purpose",
                                                  "Wellhead mark",
                                                  "Channel separation mark",
                                                  "Marine farm mark",
                                                  "Artificial reef mark"
                                                 };

static const char conditionName[][32] = {"<none>",
                                         "Under construction",
                                         "Ruined",
                                         "Under reclamation",
                                         "Wingless",
                                         "Planned construction"
                                        };

static const char constructionName[][32] = {"<none>",
                                            "Masonry",
                                            "Concreted",
                                            "Loose boulders",
                                            "Hard surfaced",
                                            "Unsurfaced",
                                            "Wooden",
                                            "Metal",
                                            "Glass reinforced plastic (GRP)",
                                            "Painted"
                                           };

static const char productName[][32] = {"<none>",
                                       "Oil",
                                       "Gas",
                                       "Water",
                                       "Stone",
                                       "Coal",
                                       "Ore",
                                       "Chemicals",
                                       "Drinking water",
                                       "Milk",
                                       "Bauxite",
                                       "Coke",
                                       "Iron ingots",
                                       "Salt",
                                       "Sand",
                                       "Timber",
                                       "Sawdust/wood chips",
                                       "Scrap metal",
                                       "Liquified natural gas (LNG)",
                                       "Liquified petroleum gas (LPG)",
                                       "Wine",
                                       "Cement",
                                       "Grain"
                                      };

static const char waterLevelName[][36] = {"<none>",
                                          "Partly submerged at high water",
                                          "Always dry",
                                          "Always under water/submerged",
                                          "Covers and uncovers",
                                          "Awash",
                                          "Subject to inundation or flooding",
                                          "Floating"
                                         };

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
S57_object::S57_object()
    :isClosed(false),isLine(false)
{
	colour[0] = 1.0;
	colour[1] = 1.0;
	colour[2] = 1.0;
	pointSize = 1.0;
	lineWidth = 2.0;

	height = 0.0;
    status = 0;
    conspicuous = 0;
    radarConspicuous = 0;
    condition = 0;
    construction = 0;
    product = 0;
    waterLevel = 0;
	useColor = GL_TRUE;
        scamin = 0.0;
}

/*---------------------------------------------------------------------------*/
S57_object::~S57_object()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void S57_object::ProcessGeometry(gz4d::ogr::Geometry geo, gz4d::ogr::SpatialReference spatialRef)
{
        spatialReference = spatialRef;
        if(geo.getSpatialReference())
            spatialReference = geo.getSpatialReference();
        gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());

        gz4d::ogr::Polygon polygon = geo;
        if(polygon)
        {
            isClosed = true;
            isLine = true;
            gz4d::ogr::LinearRing ring = polygon.getExteriorRing();
            exteriorRingClockwise = ring.isClockwise();
            gz4d::Point<double> gr = proj(*(ring.begin()));
            SetPosition(gr);

            for(gz4d::ogr::LinearRing::const_iterator p = ring.begin(); p != ring.end(); ++p)
            {
                gz4d::Point<double> pp = proj(*p);
                exteriorRing.push_back(gz4d::Point<float>(pp-gr));
            }

            interiorRings.resize(polygon.getNumInteriorRings());
            for(unsigned int i=0; i<interiorRings.size(); ++i)
            {
                ring = polygon.getInteriorRing(i);
                interiorRingClockwise.push_back(ring.isClockwise());

                for(gz4d::ogr::LinearRing::const_iterator p = ring.begin(); p != ring.end(); ++p)
                {
                    gz4d::Point<double> pp = proj(*p);
                    pp[2] = -pp[2];
                    interiorRings[i].push_back(gz4d::Point<float>(pp-gr));
                }
            }
        }

        gz4d::ogr::Point point = geo;
        if(point)
        {
            isLine = false;
            SetPosition(proj(point.getY(),point.getX()));
        }

        gz4d::ogr::LineString lineString = geo;
        if(lineString)
        {
            isClosed = false;
            isLine = true;
            gz4d::Point<double> gr = proj(*(lineString.begin()));
            SetPosition(gr);

            for(gz4d::ogr::LinearRing::const_iterator p = lineString.begin(); p != lineString.end(); ++p)
            {
                gz4d::Point<double> pp = proj(*p);
                pp[2] = -pp[2];
                exteriorRing.push_back(gz4d::Point<float>(pp-gr));
            }
        }

        gz4d::ogr::MultiPoint multiPoint = geo;
        if(multiPoint)
        {
            isClosed = false;
            isLine = false;
            gz4d::Point<double> gr = proj(multiPoint.begin()->getY(),multiPoint.begin()->getX());
            SetPosition(gr);

            for(gz4d::ogr::MultiPoint::const_iterator p = multiPoint.begin(); p != multiPoint.end(); ++p)
            {
                gz4d::Point<double> pp = proj(p->getY(),p->getX(),-p->getZ());
                exteriorRing.push_back(gz4d::Point<float>(pp-gr));
            }

        }

}

/*---------------------------------------------------------------------------*/
void S57_object::DrawLowRes(float deg,Window3D * win)
{
    float scale, pitch = 0;
    PosableStateBase *winPose = NULL;

    gz4d::glo::PushAttrib attrib(GL_POINT_BIT|GL_LIGHTING_BIT);

    if (win)
    {
        winPose = win->getViewInfo()->getPosable();
        scale = winPose->getScale();
        if (scale <= scamin)
            return;
        pitch = winPose->getOrientationAxis()[Pitch];
    }

    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0, 0.0, 4.5f);
    if (useColor)
    {
        glColor3f(colour[0]*S57::dimFactor(),colour[1]*S57::dimFactor(),colour[2]*S57::dimFactor());
    }
    if(isLine)
    {
        if (selectedFlag)
            glLineWidth(lineWidth + 2);
        else
            glLineWidth(lineWidth);
        if (height <= 0.0 || (89.0 <= pitch && pitch <= 91.0))
        {
            if(isClosed)
                glBegin(GL_LINE_LOOP);
            else
                glBegin(GL_LINE_STRIP);
            for(std::vector<gz4d::Point<float> >::iterator p = exteriorRing.begin(); p != exteriorRing.end(); ++p)
            {
                gz4d::glo::Vertex(*p);
            }
            glEnd();

            for(unsigned int i = 0; i < interiorRings.size(); ++i)
            {
                if(!interiorRings[i].empty())
                {
                    glBegin(GL_LINE_LOOP);
                    for(std::vector<gz4d::Point<float> >::iterator p = interiorRings[i].begin(); p != interiorRings[i].end(); ++p)
                        gz4d::glo::Vertex(*p);
                    glEnd();
                }
            }
        }
        else
        {
            glBegin(GL_QUAD_STRIP);
            for(std::vector<gz4d::Point<float> >::iterator p = exteriorRing.begin(); p != exteriorRing.end(); ++p)
            {
                glVertex3f((*p)[0],(*p)[1],0.0);
                glVertex3f((*p)[0],(*p)[1],height);
            }
            if(isClosed)
            {
                glVertex3f((*exteriorRing.begin())[0],(*exteriorRing.begin())[1],0.0);
                glVertex3f((*exteriorRing.begin())[0],(*exteriorRing.begin())[1],height);
            }
            glEnd();
        }
    }
    else
    {
        if (renderingForPicking)
            glPointSize(3 * pointSize);
        else
        {
            if (selectedFlag)
                glPointSize(pointSize + 2);
            else
                glPointSize(pointSize);
        }
        if (height <= 0.0)
        {
            glBegin(GL_POINTS);
            glVertex2f(0.0,0.0);
            glEnd();
        }
        else
        {
            if (selectedFlag)
                glLineWidth(lineWidth + 2);
            else
                glLineWidth(lineWidth);
            glBegin(GL_LINES);
            glVertex3d(0.0, 0.0, 0.0);
            glVertex3d(0.0, 0.0, height);
            glEnd();
            glBegin(GL_POINTS);
            glVertex3d(0.0,0.0, height);
            glEnd();
        }
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/
void S57_object::SetColour(Point3f c)
{
	colour[0] = c[0];
	colour[1] = c[1];
	colour[2] = c[2];
}


/*---------------------------------------------------------------------------*/
S57_LayerInfo const &S57_object::getLayerInfo(std::string const &acronym)
{
    return layerInfo[acronym];
}

/*---------------------------------------------------------------------------*/
void S57_object::SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber)
{
    Point3f c;
    int tempScale;

    lnam = item["LNAM"].GetValue<std::string>();

    if(item["LNAM_REFS"])
        lnam_refs = item["LNAM_REFS"].GetValue<std::vector<std::string> >();

	// color
    if(item["COLOUR"])
    {
	colorCode = item["COLOUR"].GetValue<int>();
	if (colorCode) //added by mdp to allow different objects to have different defaults.
	{
		DecodeColour(colorCode,c);
		SetColour(c);
	}
    }

	// color pattern
        if(item["COLPAT"])
        {
            colorPattern = atoi(item["COLPAT"].GetValue<std::string>().c_str());
            if (colorPattern) //There may be more part to the color!
                colorCodeList = item["COLOUR"].GetValue<std::vector<int> >();
        }

    // label
        if(item["OBJNAM"])
            SetLabel(item["OBJNAM"].GetValue<std::string>());
	else if (!info.defaultLabel.empty())
	{
            SetLabel(info.defaultLabel+'_'+boost::lexical_cast<std::string>(objectNumber));
	}

        if(item["HEIGHT"])
            height = item["HEIGHT"].GetValue<double>();

	// inform
        if(item["INFORM"])
            inform = item["INFORM"].GetValue<std::string>();

        if(item["TXTDSC"])
            textDescription = item["TXTDSC"].GetValue<std::string>();

        if(item["SCAMIN"])
        {
            tempScale = item["SCAMIN"].GetValue<int>();
        //std::cerr << tempScale << std::endl;
	if (tempScale > 0)
		scamin = 0.9999f / tempScale; //subtract that little bit extra for roundoff
	else
		scamin = 0;
        }

    // status
        if(item["STATUS"])
            status = item["STATUS"].GetValue<int>();

    // visually conspicuous?
        if(item["CONVIS"])
            conspicuous = item["CONVIS"].GetValue<int>();

    // conspicuous to radar?
        if(item["CONRAD"])
            radarConspicuous = item["CONRAD"].GetValue<int>();


    // condition
        if(item["CONDTN"])
            condition = item["CONDTN"].GetValue<int>();


    // nature of construction
        if(item["NATCON"])
            construction = item["NATCON"].GetValue<int>();

    // product
        if(item["PRODCT"])
            product = item["PRODCT"].GetValue<int>();

    // water level
        if(item["WATLEV"])
            waterLevel = item["WATLEV"].GetValue<int>();

}

/*---------------------------------------------------------------------------*/
void S57_object::parseIntList(std::string listAsStrings, std::vector<int> &listAsInts)
{
    std::string::size_type split, index = 0;

    for (split = listAsStrings.find(',', index);
         split != std::string::npos;
         split = listAsStrings.find(',', index))
    {
        listAsInts.push_back(atoi(listAsStrings.substr(index, split).c_str()));
        index = split + 1;
    }
    listAsInts.push_back(atoi(listAsStrings.substr(index).c_str()));
}

/*---------------------------------------------------------------------------*/
void S57_object::DecodeColour(int code, Point3f &colour)
{
	switch (code)
	{
	case 1:
		//cerr << "white\n";
		colour[0]=1.0f;
		colour[1]=1.0f;
		colour[2]=1.0f;
		break;
	case 2:
		//cerr << "black\n";
		colour[0]=0.0f;
		colour[1]=0.0f;
		colour[2]=0.0f;
		break;
	case 3:
		//cerr << "red\n";
		colour[0]=0.9f;
		colour[1]=0.1f;
		colour[2]=0.01f;
		break;
	case 4:
		//cerr << "green\n";
		colour[0]=0.1f;
		colour[1]=0.8f;
		colour[2]=0.1f;
		break;
	case 5:
		//cerr << "blue\n";
		colour[0]=0.2f;
		colour[1]=0.2f;
		colour[2]=0.9f;
		break;
	case 6:
		//cerr << "yellow\n";
		colour[0]=0.9f;
		colour[1]=0.8f;
		colour[2]=0.1f;
		break;
	case 7:
		//cerr << "grey\n";
		colour[0]=0.50f;
		colour[1]=0.50f;
		colour[2]=0.50f;
		break;
	case 8:
		//cerr << "brown\n";
		colour[0]=0.40f;
		colour[1]=0.30f;
		colour[2]=0.0f;
		break;
	case 9:
		//cerr << "amber\n";
		colour[0]=0.70f;
		colour[1]=0.60f;
		colour[2]=1.0f;
		break;
	case 10:
		//cerr << "violet\n";
		colour[0]=0.30f;
		colour[1]=0.0f;
		colour[2]=0.50f;
		break;
	case 11:
		//cerr << "orange\n";
		colour[0]=1.0f;
		colour[1]=0.50f;
		colour[2]=0.10f;
		break;
	case 12:
		//cerr << "magenta\n";
		colour[0]=0.9f;
		colour[1]=0.1f;
		colour[2]=0.9f;
		break;
	case 13:
		//cerr << "pink\n";
		colour[0]=1.0f;
		colour[1]=0.50f;
		colour[2]=0.70f;
		break;
	default:
		colour[0]=1.0f;
		colour[1]=1.0f;
		colour[2]=1.0f;
	}
}

/*---------------------------------------------------------------------------*/
/**	Display info about this object in the info window on the GUI.
 */
void S57_object::ShowInfo(GLuint *names, int numNames)
{
	char infoLine[512];
	Scene3D *scene;

	scene = mainWindow->getScene3D();

    if (MainWindow::getInterfaceType() == GeoCoastPilot)
    {
        if (!isLine)
        {
            //output my lat/lon
            double lat, lon;
            gz4d::geo::Projection proj(spatialReference,scene->getSpatialReference());
            if (proj)
            {
                gz4d::geo::Position<double> p = proj.inverse(pose->getPosition()[X], pose->getPosition()[Y]);
                lat = p.latitude.value();
                lon = p.longitude.value();
            }
            else
            {
                lon = pose->getPosition()[X];
                lat = pose->getPosition()[Y];
            }
            //lat and lon in decimal degree notation
            DecimalDegrees dd(lat,lon,"\\\"",false);
            //proj->formatDecimalDegrees(lat, latD,latStr, 3);
            //proj->formatDecimalDegrees(lon, lonD,lonStr, 2);
            sprintf(infoLine, "Lat.:\t%d\\xb0 %s     ", abs(int(lat)), dd.lat_string.c_str());
            scene->appendInfo(infoLine);
            sprintf(infoLine, "Lon.:  %d\\xb0 %s\n", abs(int(lon)), dd.lon_string.c_str());
            scene->appendInfo(infoLine);
        }
    }
    else
    {
	    sprintf(infoLine, "Type:\t%s\n", getRegistrationEntry()->className.c_str());
	    scene->appendInfo(infoLine);
	    sprintf(infoLine, "Name:\t%s\n", GetLabel().c_str());
	    scene->appendInfo(infoLine);
	    sprintf(infoLine, "Base Position:\t%f, %f, %f\n", GetPosition(0), GetPosition(1), GetPosition(2));
	    scene->appendInfo(infoLine);
    	scene->appendInfo("--------------------\n");
    }
/*	if (elevation > 0)
	{
		sprintf(infoLine, "Elev.:\t\t%0.2fm\n", elevation);
		scene->appendInfo(infoLine);
	}
*/
	ShowS57Info(scene);
}

/*---------------------------------------------------------------------------*/
void S57_object::ShowS57Info(Scene3D *scene)
{
	char infoLine[512];
    if (colorCode > 0 && colorCode <= 13)
    {
        scene->appendInfo("Color:\t");
        if (colorPattern > 0 && colorPattern <= 6)
        {
            int size = colorCodeList.size();
            const char *format;
            for (int i = 0; i < size; i++)
            {
                if (i == 0)
                    format = "%s";
                else
                    format = ", %s";
                sprintf(infoLine, format, colorName[colorCodeList[i]]);
                scene->appendInfo(infoLine);
            }
            sprintf(infoLine, " (%s)", colorPatternName[colorPattern]);
            scene->appendInfo(infoLine);
        }
        else
        {
            sprintf(infoLine, "%s", colorName[colorCode]);
            scene->appendInfo(infoLine);
        }
        scene->appendInfo("\n");
    }
	if (height > 0)
	{
		sprintf(infoLine, "Height:\t%0.0f ft (%0.1f m)\n", height * 3.2808399, height);
		scene->appendInfo(infoLine);
	}
    if (status > 0 && status <= 18)
    {
        sprintf(infoLine, "Status:\t%s\n", statusName[status]);
        scene->appendInfo(infoLine);
    }
	
    if (waterLevel > 0 && waterLevel <= 7)
    {
        sprintf(infoLine, "Water level effect:\t%s\n", waterLevelName[waterLevel]);
        scene->appendInfo(infoLine);
    }
    if (product > 0 && product <= 22)
    {
        sprintf(infoLine, "Product:\t%s\n", productName[product]);
        scene->appendInfo(infoLine);
    }
    if (conspicuous > 0)
    {
        sprintf(infoLine, "%sisually conspicuous\n", (conspicuous == 2)? "Not v": "V");
        scene->appendInfo(infoLine);
    }
    if (radarConspicuous > 0)
    {
        sprintf(infoLine, "%sonspicuous to radar%s\n", (radarConspicuous == 2)? "Not c": "C", (radarConspicuous == 3)? " (has radar reflector)": "");
        scene->appendInfo(infoLine);
    }
    if (condition > 0 && condition <= 5)
    {
        sprintf(infoLine, "Condition:\t%s\n", conditionName[condition]);
        scene->appendInfo(infoLine);
    }
    if (construction > 0 && construction <= 9)
    {
        sprintf(infoLine, "Construction:\t%s\n", constructionName[construction]);
        scene->appendInfo(infoLine);
    }
    if (!inform.empty())
	{
        scene->appendInfo("Info:\t");
		scene->appendInfo(inform);
        if (inform[inform.size()-1] != '\n')
            scene->appendInfo("\n");
	}
    if (!textDescription.empty())
    {
        if (!inform.empty()) //separate from the information line, if there.
            scene->appendInfo("\n");
        scene->appendInfo("Description:\n");
        scene->appendInfo(textDescription);
        if (textDescription[textDescription.size()-1] != '\n')
            scene->appendInfo("\n");
    }
}

/*---------------------------------------------------------------------------*/
bool S57_object::generateSpecialPurposeString(char *targetString)
{
    bool retval = false;
    int i, size, val;

    size = categoryList.size();
    for (i = 0; i < size; i++)
    {
        val = categoryList[i];
        if (val > 0 && val <= 56)
        {
            if (retval) //not first time through
                strcat(targetString, ", ");
            else //first time through
            {
                strcpy(targetString, "Purpose:\t");
                retval = true;
            }
            strcat(targetString, specialPurposeMarkName[val]);
        }
    }

    if (retval)
        strcat(targetString, "\n");

    return retval;
}

/*---------------------------------------------------------------------------*/
void S57_object::resolveReferences(LnamTable &lnam_table, std::string const &s57file)
{
    if (textDescription.empty())
        return;

    bfs::path textPath = bfs::path(s57file)/textDescription;

    std::ifstream contents(textPath.string().c_str());
    if (!contents)
    {
        std::cerr << "Unable to open " << textPath.string().c_str() << std::endl;
        return;
    }

    // Determine stream size 
    contents.seekg(0, std::ios::end); 
    size_t size = contents.tellg(); 
    contents.seekg(0);    

    std::vector<char> tmp(size);
    contents.read(&tmp.front(),size);
    textDescription = std::string(tmp.begin(),tmp.end());
}
