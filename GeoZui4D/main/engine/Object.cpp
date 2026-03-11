// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
// Object.cpp: implementation of the Object class.
//
//////////////////////////////////////////////////////////////////////

#include "utilities/ObjectHistory.h" //I don't understand why, but this has to be first or else Line.h doesn't get included...
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "engine/Object.h"
#include "widgets/Sv_Standards.h"
#include "utilities/DottedAddress.h"
#include "utilities/Addressable.h"
#include "engine/TranslatableState.h"
#include "engine/ObjectPoseState.h"
#include "engine/ViewManager.h"
#include "utilities/Addressable.h"
#include "interactors/PointHandle.h"
#include "tclLinkage.h"
#include "interactors/DoubleClickInteractor.h"
#include "utilities/SmartValue.h"
#include "engine/gzCommandInterface.h"
#include <assert.h>
#ifdef LOCK_TO_GROUPS
#include "CompositeObject.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/WindowPoseState.h"
//#include "utilities/LinkedList.h"
#endif
//following includes added by mdp for file saving
#ifdef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <errno.h>
#include <tcl.h>
#include <typeinfo>
#include <ctype.h>
#include "engine/PosableState.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "utilities/disc.h"
#include "utilities/Utilities.h"
#include <iomanip>
#include "utilities/TimePieces.h"
#include <gz4d/util/Utilities.h>

#include "engine/ObjectCollection.h"
#include "engine/TextureMaps/TextureMap.h"
#include "engine/TextureMaps/Clut.h"
#include "engine/TimeControl.h"
#include <gz4d/geo/Projection.h>

//using namespace std;

extern MainWindow * mainWindow;
extern PointHandle *pointHandle;

/*These are sorted by rgb value, r having highest priority*/
Point3f Object::palette[NUM_COLOR_CODES] =
                                {
                                    {0.0f, 0.0f, 0.0f}, //Black (K)
                                    {0.1f, 0.5f, 0.1f}, //Dark Green (D)
                                    {0.2f, 0.8f, 0.2f}, //Green (G)
                                    {0.2f, 0.9f, 0.9f}, //Cyan (C)
                                    {0.3f, 0.3f, 1.0f}, //Blue (B)
                                    {0.6f, 0.6f, 0.6f}, //Elephant Grey (E)
                                    {0.7f, 0.1f, 0.7f}, //Magenta (M)
                                    {0.8f, 0.6f, 0.2f}, //Auburn (brown--A)
                                    {0.8f, 0.8f, 1.0f}, //Lt. Periwinkle (default)
                                    {0.9f, 1.0f, 0.0f}, //Yellow (Y)
                                    {1.0f, 0.2f, 0.2f}, //Red (R)
                                    {1.0f, 1.0f, 1.0f}}; //White (W)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ObjectRegistrationEntry **Object::objectRegistry = NULL;
int Object::numEntries = 0;
int Object::maxEntries = 0;
GLfloat Object::lastHeightExaggeration = 1.0;
const float Object::invalidHeight = -10000000.0;
const EventType *Object::objectClickedInputType =
            Interactor::newEventType("objectClicked", Interactor::DataAsAddress);
const EventType *Object::objectSelectedInputType =
            Interactor::newEventType("objectSelected", Interactor::DataAsAddress);
const EventType *Object::objectPoseChangedInputType =
            Interactor::newEventType("objectPoseChanged", Interactor::DataAsAddress);
const EventType *Object::objectStabilizedInputType =
            Interactor::newEventType("objectStabilized", Interactor::DataAsAddress);

Addressable ObjectNeighborhoodAnchor;
DottedAddress *ObjectNeighborhood = NULL; //new DottedAddress(&ObjectNeighborhoodAnchor, "Object"); //changed to be Scene3D's ObjectCollection by mdp 11/2/2004

std::shared_ptr<std::shared_ptr<TextureMap> >  Object::defaulttextureptr = std::shared_ptr<std::shared_ptr<TextureMap> >(new std::shared_ptr<TextureMap>);

Object *Object::selectedObject = NULL;
Object *Object::frameOfReference = NULL;
int Object::settingOffset = GL_FALSE;
TimeVaryingObject **Object::tide = NULL;
NavigationPath **Object::navPath = NULL;

double *Object::commonRange = NULL;
int Object::rangesLeft = 0;

#define REGISTRY_GROWTH_STEP 50

static const char *ObjectAttributeNames[] = {OBJECT_ATTRIBUTES};
static const char *DefaultObjectNames[] = {OBJECT_NAMES, ""};

/*---------------------------------------------------------------------------*/
Object::Object(GLboolean allocatePosable, int pickingTypes)
: Pickable(pickingTypes)
{
    if (!keytable)
        SetKeywordTable(DefaultObjectNames);

    customtextureid = 0;
    //  defaulttextureptr = NULL;
    visible = 1;
    visibilityRanges = NULL;
    doneFlag = 0;
    degradationFlag = ON;
    garbageFlag = GL_FALSE;
    editedFlag = GL_FALSE;
    isEditableFlag = false;
    projectionRequired = GL_FALSE;
    reversedCoordinates = GL_FALSE;
    selectedFlag = GL_FALSE;
    usingInternalHandleFlag = GL_FALSE;
    canUseInternalHandleFlag = GL_FALSE;
    respondToPress = GL_FALSE;
    respondToPress2 = GL_FALSE;
    respondToClick = GL_TRUE;
    centerGiven = GL_FALSE;
    requiresFullReload = false;
    renderingForPicking = GL_FALSE;
    if (allocatePosable)
        pose = new TranslatableState(this);
    else
        pose = NULL;
    showAsWireframe = false;
    bbox.start = bbox.end = 0;
    qstart = qend = 0;
    useTextures = 1; //By default, if an object calls EnableTexture in rendering loops, it should use it
    clutMode = TEXTURE_COORD_POSABLE;
    treePath = NULL;
    scalePriority = 0.0;

    numAttributes = NumObjectAttributes;
    attributeNames = ObjectAttributeNames;

    timeIndex = -1;
    hideOutsideTimeBounds=true;
    preShowTime=10; 
    postShowTime=10; 
    interpolatePositions=1;

    transparencyLevel = 0;

    onlyWin = NULL;
    published = 0;
    
    referable = 0;

    parent = NULL;
    lockToParent = NULL;

    spheroidFlag =false;
    spheroid_x = spheroid_y = spheroid_z = 0.0;

    hudFlag = false;

    acousticSignature = NULL;
    calculateCOG = 0;
    allowBaseExternalEdit = true;
    outsideTimeBounds = 0;

    lastIdleTime = 0.0;
    bbox.reset();

}

/*---------------------------------------------------------------------------*/
Object::~Object()
{
    if (editedFlag)
        Scene3D::objectEdited(GL_FALSE);
    if (selectedObject == this)
        setSelectedObject(NULL);
    if (visibilityRanges)
    {
        if (visibilityRanges->destroyCommonRange)
            delete [] visibilityRanges->commonRange;
        delete visibilityRanges;
    }
}


/*---------------------------------------------------------------------------*/
void Object::replace(Object *ob)
{
    id = ob->id;
    delete ob;
}

/*---------------------------------------------------------------------------*/
bool Object::SetLabel(std::string const &l)
{
    bool hadLabel = false;

    if (!GetLabel().empty() && pose->getAddress())
        hadLabel = GL_TRUE;

    Persistence::SetLabel(l);
    
    SanitizeLabel();

    if (!pose)
        return false;

    if (hadLabel && pose)
        pose->getAddress()->assumeName(GetLabel());
    else
        registerLabel();

    return true;
}

/*---------------------------------------------------------------------------*/
/* This function opens this object's file and reads the information in it.
*/
int Object::Load()
{
    int retval;

    if(Open())
    return 1;

    retval = readFrom(in);

    Close();

    registerLabel();
    return retval;
}

/*---------------------------------------------------------------------------*/
/// Registers the label usually after loading.
void Object::registerLabel()
{
    SanitizeLabel();

    pose->publishName(GetLabel());
    if (parent)
        parent->getAddress()->addChild(pose->getAddress());
    else if (ObjectNeighborhood && pose->getAddress())
        ObjectNeighborhood->addChild(pose->getAddress());
    initializingLabel();
}

/*---------------------------------------------------------------------------*/
/** Processes the fields understood at each level in the inheritence hierarchy of an object.
    Each child is meant to have its processField function call this one (usually in the default
    section of its switch).
    @return 0 for a match, -1 match ending the header, >0 indicates an error
*/
int Object::processField(int option, char **fields)
{
    int i = 1;
    int numArgs;
    Point3d gr;

    switch(option)
    {
        case REVERSED_XY:
            reversedCoordinates = GL_TRUE;
            break;
        case DEGRADATION:
            degradationFlag = Lookup(fields[i]);
            if (degradationFlag < 0)
                return 1;
            break;
        case LOCAL_ORIGIN:
            ConvertDoublePoint(gr, i, reversedCoordinates);
            numArgs = getNumFields();
            if (numArgs >= 5)
            {
                if (strcmp(fields[i++], "IN") == 0)
                {
                    std::cerr << "IN ... not longer supported, use SPATIAL_REFERENCE" << std::endl;
                    return 1;

/*                    std::string proj_name = fields[i];
                    if (proj_name.substr(0,4) == "ZONE")
                        proj = gz4d::geo::UTMProjection(boost::lexical_cast<int>(proj_name.substr(4)));
                    else
                        std::cerr << "Unsupported projection: " << fields[i] << std::endl;

                    scene3d = mainWindow->getScene3D();
                    if (scene3d->setProjection(proj) != TCL_OK)
                    {
                        cerr << "Warning: conflicting projections." << endl;
                    }*/
                }
            }
            SetPosition(gr);
            return -1; //end of header
            break;
        case SPATIAL_REFERENCE:
        {
            ++fields;
            std::string p;
            while(*fields)
            {
                p += std::string(*fields);
                ++fields;
                if(*fields)
                    p += " ";
            }
            spatialReference = gz4d::ogr::SpatialReference();
            spatialReference.SetFromUserInput(p);

        }
            break;
        case PROJECT:
            std::cerr << "PROJECT no longer supported, use SPATIAL_REFERENCE" << std::endl;
            return 1;
/*            numArgs = getNumFields();
            if (numArgs < 2)
            {
                cerr << "Missing argument(s) on PROJECT line.  Should be\n"
                    << "PROJECT <projection>\n";
                return 1;
            }
            if (strcmp(fields[i], "AUTO") != 0)
            {
                std::string proj_name = fields[i];
                if (proj_name.substr(0,4) == "ZONE")
                    proj = gz4d::geo::UTMProjection(boost::lexical_cast<int>(proj_name.substr(4)));
                else
                    std::cerr << "Unsupported projection: " << fields[i] << std::endl;

            }
            i++;
            projectionRequired = GL_TRUE;
            if (i<getNumFields() && strcmp(fields[i++], "CENTER") == 0)
            {
                ConvertDoublePoint(gr, i, reversedCoordinates);
                centerGiven = GL_TRUE;
                retval = autoProjection(gr[X]);
                gz4d::Point<double> p = proj(gr[Y],gr[X]);
                gr[X] = p[0];
                gr[Y] = p[1];
                if (retval)
                    return 1; //error in projection
            }
            return -1; //header done
            break;*/
        default:
            std::cout << "Invalid keyword " << fields[0] << std::endl;
            return 1; //error--couldn't find it.
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/** The readFrom function reads the header for all derived classes.
*   It should be called by each child's own readFrom function.  All fields
*   passed back to the child by a call to processField.
* \param file file open for reading.
* \retval 0(TCL_OK) success.
* \retval nonzero failure.
*/
int Object::readFrom(FILE *file)
{
    int item;
    char **fields = NULL;
    int i;
    
    std::cout << "Loading " << GetFilename() << " ..." << std::endl;

    for(fields = NextLine(GL_FALSE); fields != NULL; fields = NextLine(GL_FALSE))
    {
        i = 0;
        item = Lookup(fields[i++]);
        if (item == -1)
            return 1;//Lookup already reports the error

        switch (processField(item, fields))
        {
            case 1:
                return 1;
                break;
            case 0:
                break; //handled.
            case -1:
                return 0; //done with header.
                break;
        }
    }

    return(0);
}

/*---------------------------------------------------------------------------*/
/** Projects the passed xVals and yVals using the Projection specified in the member variable proj.
*  If Object::proj is NULL, Object::proj is set to the  currently active GeoZui4D projection.
*  If that is also NULL, it assumes the UTM zone for the first x coordinate.
*  @param nVals number of pairs expected. If <= 0, nothing is done.
*   @param completesProjection If GL_FALSE, projectionRequired is unchanged, otherwise it is set to GL_TRUE.
*   @param xVals Array of nVals x values, or latitudes in signed decimal degrees.
*  @param yVals Array of nVals y values, or longitudes in signed decimal degrees.
*  @retval 0(TCL_OK) success (or when nVals is <= 0)
*  @retval !=0 Failure.  
*/
int Object::projectXY(int nVals, double *xVals, double *yVals, GLboolean completesProjection)
{
    int i, retval;
    double xOffset, yOffset;

    if (nVals <= 0 || !projectionRequired) //does nothing
        return 0;

    if (!centerGiven)
    {
        retval = autoProjection(xVals[0],yVals[0]);
        if (retval)
            return retval;
    }

    gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
    //determine our geoReference point

    gz4d::Point<double> gr;
    if (!centerGiven)
    {
        gr = proj(yVals[0], xVals[0]);
        SetPosition(gr);
        centerGiven=!centerGiven;
    }
    else
    {
        gr[0]=GetPosition(0);
        gr[1]=GetPosition(1);
    }
    xOffset = gr[X];
    yOffset = gr[Y];

    for (i = 0; i < nVals; i++)
    {
        gz4d::Point<double> ret = proj(yVals[i], xVals[i]);
        xVals[i] = (float)(ret[0] - xOffset);
        yVals[i] = (float)(ret[1] - yOffset);
    }
    if (completesProjection)
        projectionRequired = GL_FALSE;

    return 0;
}

/*---------------------------------------------------------------------------*/
int Object::projectXY(std::vector<gz4d::Point<double> > &vals, GLboolean completesProjection)
{
    int retval;
    double xOffset, yOffset;

    if (vals.empty() || !projectionRequired) //does nothing
        return 0;

    if (!centerGiven)
    {
        retval = autoProjection(vals[0][X],vals[0][Y]);
        if (retval)
            return retval;
    }

    gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());

    gz4d::Point<double> gr;
    if (!centerGiven)
    {
        gr = proj(vals[0][Y], vals[0][X]);
        SetPosition(gr);
    }
    xOffset = gr[X];
    yOffset = gr[Y];

    //project
    for (unsigned int i = 0; i < vals.size(); i++)
    {
        gz4d::Point<double> ret = proj(vals[i][Y], vals[i][X]);
        vals[i][X] = (float)(ret[0] - xOffset);
        vals[i][Y] = (float)(ret[1] - yOffset);
    }
    if (completesProjection)
        projectionRequired = GL_FALSE;

    return 0;
}

/*---------------------------------------------------------------------------*/
/** Sets the object's projection, if it is NULL.
*   If it needs to be set, it is set to GeoZui's current projection.
*  If GeoZui doesn't have a current projection, one will be determined from 
*  longitude, setting both this object's and GeoZui's current projection.
*   @param longitude used to determine an appropriate projection, if necessary
*  @return 0
*/
int Object::autoProjection(double longitude, double latitude)
{
    if(!mainWindow->getScene3D()->getSpatialReference())
    {
        if(spatialReference)
            mainWindow->getScene3D()->setSpatialReference(spatialReference);
        else
        {
            gz4d::ogr::SpatialReference sr;
            sr.SetUTM(gz4d::geo::UTMZone(longitude), latitude >= 0.0);
            mainWindow->getScene3D()->setSpatialReference(sr);
        }
    }

    if(!spatialReference)
    {
        spatialReference.SetWellKnownGeogCS("WGS84");
    }

    return 0;
}

/// Set the object's UTM projection.
/// Also sets GZ's current projection if it wasn't already set.
/// @param zone Projection's UTM zone.
/// @return true if GZ's current projection matches this object's projection.
bool Object::setUTMProjection( int zone, bool north)
{
    spatialReference = gz4d::ogr::SpatialReference();
    spatialReference.SetUTM(zone,north);
    if(mainWindow->getScene3D()->getSpatialReference())
    {
        if(mainWindow->getScene3D()->getSpatialReference().IsSame(spatialReference))
            return true;
    }
    else
    {
        mainWindow->getScene3D()->setSpatialReference(spatialReference);
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/
/** This procedure attempts to save the object through the derived writeTo
*   function.  If no filename exists, it attempts to make a filename using
*   a name of the form [label].[ext].  If no label exists, it attempts to
*   come up with an unused filename of the form [type]####.[ext].
*/
int Object::SaveAs(std::string const &fn)
{
    SetFilename(fn);
    return save();
}


void Object::SaveObject(int id, std::string const &fn)
{
    if(id == this->id)  
        SaveAs(fn);
}


/*---------------------------------------------------------------------------*/
/** This procedure attempts to save the object through the derived writeTo
*   function.  If no filename exists, it attempts to make a filename using
*   a name of the form [label].[ext].  If no label exists, it attempts to
*   come up with an unused filename of the form [type]####.[ext].
*/
int Object::save(std::string const &path)
{
    int uniqueLen = 4;
    //struct stat info;
    
    fs::path fullFilename;

    if (!GetFilename().empty())
        fullFilename = GetFilename();

    if (fullFilename.empty())
    {
        std::string label = GetLabel(), labelPortion;
;
        if (!label.empty())
        {
            unsigned i;
            for (i = 0; isalnum(label[i]); i++)
                /* just fast-forward */;
                if (i > 1)
                    labelPortion = label.substr(0,i-1);
        }
        
/*        char * tmpPath;
        mainWindow->Tcl_Eval("pwd", &tmpPath);
        path = tmpPath;*/
        
//         if (path[path.size()-1] != '/')
//             path.append("/");

        std::string typeName;
        GET_CLASS_NAME(this, typeName);
        unsigned unique = 0;
        
        if (!labelPortion.empty())
            labelPortion = "_"+labelPortion;
        do
        {
            std::stringstream uniqueStr;
            uniqueStr << std::setw(uniqueLen) << unique;
            fullFilename = fs::current_path();
            fullFilename/= (typeName+uniqueStr.str()+labelPortion+GetSuffix());
            unique++;
        }
        while (exists(fullFilename));
        
        if (fullFilename.empty())
            return TCL_ERROR;

        SetFilename(fullFilename.string());
    }//end of case where there is no fullFilename
    
    if (OpenWrite(fullFilename.string()))
        return TCL_ERROR;

    fprintf(out, "BEGIN %s %d.%d\n", GetType(), GetVersion(), GetRevision());
    int retval = writeTo(out);
    CloseWrite();
    
    if (retval == TCL_OK)
        setEdited(GL_FALSE);
    
    return retval;
}

/*---------------------------------------------------------------------------*/
void Object::setEdited(GLboolean val)
{
    if (editedFlag == val)
        return;

    editedFlag = val;
    Scene3D::objectEdited(val);
}

/*---------------------------------------------------------------------------*/
/** Derived classes should define this function to allow saving.  Returns
*   TCL_OK if successful, TCL_ERROR otherwise.  The Object::writeTo()
*   function should be called between the header and data by the child
*   for writing information common to all objects (including LOCAL_ORIGIN).
*/
int Object::writeTo(FILE *file)
{
    gz4d::Point<double> position = pose->getPosition();

    //DEGRADATION line
    if (degradationFlag != ON)
        fprintf(file, "%s %s\n", Keyword(DEGRADATION), Keyword(degradationFlag));

    fprintf(file, "%s %.6f %.6f %.6f", Keyword(LOCAL_ORIGIN),
        position[X], position[Y], position[Z]);
/*    if (proj)
        fprintf(file, " IN %s", proj->getAddress()->getAddress().c_str());*/
    fprintf(file, "\n");
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
std::string Object::GetSuffix()
{
    std::string ext;
    
    if (!GetFilename().empty())
        ext = GetExtension();

    if(!ext.empty())
        return ext;
    else
        return '.'+getRegistrationEntry()->extension;
}

/*---------------------------------------------------------------------------*/
void Object::SetVisible(int id)
{
    if(id == this->id || id == -1)
    {
        visible = 1;
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }
}

/*---------------------------------------------------------------------------*/
void Object::SetInvisible(int id)
{
    if(id == this->id || id == -1)
    {
        visible = 0;
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }
}

/*---------------------------------------------------------------------------*/
GLboolean Object::isVisible(Window3D *win)
{
    GLboolean retval = visible && (!win || !onlyWin || win == onlyWin);

    if (retval && visibilityRanges && win) //only check visibility ranges if otherwise visible
    {
        PosableStateBase *winPose = win->getPoseInfo();
        gz4d::Point<double> winPos = win->getViewInfo()->getEyePoint(); //we should do these things w.r.t the eyepoint!
        gz4d::Point<double> myPos;

        if (visibilityRanges->commonRange)
            myPos = visibilityRanges->commonRange;
        else
            myPos = pose->getPosition();
        //run the gamut to see if we're still visible (conditions ANDed together)
        if (visibilityRanges->useHeading) //minHeading <= myHeadingFromWin < maxHeading?
        {
            float myHeadingFromWin = atan2(myPos[X] - winPos[X], myPos[Y] - winPos[Y]);
            myHeadingFromWin = RAD2DEG(myHeadingFromWin);
            if (myHeadingFromWin < 0)
                myHeadingFromWin += 360.0;
            //cerr << myPos[X] << ", " << myPos[Y] << " against " << winPos[X] << ", " << winPos[Y] << " yields " << myHeadingFromWin << endl;
            float minHeading = visibilityRanges->minHeading;
            float maxHeading = visibilityRanges->maxHeading;
            if (maxHeading < minHeading) //check outer range
            {
                if (myHeadingFromWin < minHeading && myHeadingFromWin >= maxHeading)
                    return GL_FALSE;
            }
            else //normal range
            {
                if (myHeadingFromWin < minHeading || myHeadingFromWin >= maxHeading)
                    return GL_FALSE;
            }
        }
        if (visibilityRanges->useDistance) //minDistance <= myDistanceFromWin < maxDistance?
        {
            float myDistanceFromWin, x, y;
            x = myPos[X] - winPos[X];
            y = myPos[Y] - winPos[Y];
            myDistanceFromWin = sqrt(x*x + y*y);
            if (myDistanceFromWin < visibilityRanges->minDistance || myDistanceFromWin >= visibilityRanges->maxDistance)
                return GL_FALSE;
        }
        if (visibilityRanges->useScale) //minScale <= winScale < maxScale?
        {
            float winScale = winPose->getScale();
            if (winScale < visibilityRanges->minScale || winScale >= visibilityRanges->maxScale)
                return GL_FALSE;
        }
    }

    if (parent && retval)
        retval &= parent->isVisible(win);

    return retval;
}

/*---------------------------------------------------------------------------*/
void Object::startCommonRange(Point3d &pos)
{
    if (visibilityRanges)
        visibilityRanges->destroyCommonRange = true;
    else
        return; //can't do it!

    commonRange = new double[3];
    COPY_POINT(commonRange, pos);
    visibilityRanges->commonRange = commonRange;

    rangesLeft = -1;
}

/*---------------------------------------------------------------------------*/
void Object::endCommonRange(int afterNmore)
{
    rangesLeft = afterNmore;
    if (rangesLeft <= 0)
        commonRange = NULL;
}

/*---------------------------------------------------------------------------*/
void Object::setHeadingRange(float min, float max)
{
    if (!visibilityRanges)
        visibilityRanges = new VisibilityRanges;

    //move inputs into 0-360 range)
    while (min < 0.0)
        min += 360.0;
    while (min >= 360.0)
        min -= 360.0;
    while (max <= 0.0)
        max += 360.0;
    while (max > 360.0)
        max -= 360.0;

    if (min == max) // assume they want to see it always rather than not at all
    {
        min = 0;
        max = 360.0;
    }
    visibilityRanges->minHeading = min;
    visibilityRanges->maxHeading = max;
    visibilityRanges->useHeading = true;
    if (commonRange && rangesLeft--)
        visibilityRanges->commonRange = commonRange;
}

/*---------------------------------------------------------------------------*/
void Object::setDistanceRange(float min, float max)
{
    if (!visibilityRanges)
        visibilityRanges = new VisibilityRanges;

    //move everything into positive territory
    if (min < 0)
        min *= -1;
    if (max < 0)
        max *= -1;

    if (min < max) //swap if backward.
    {
        float temp = min;
        min = max;
        max = temp;
    }
    visibilityRanges->minDistance = min;
    visibilityRanges->maxDistance = max;
    visibilityRanges->useDistance = true;
    if (commonRange && rangesLeft--)
        visibilityRanges->commonRange = commonRange;
}

/*---------------------------------------------------------------------------*/
void Object::setScaleRange(float min, float max)
{
    if (!visibilityRanges)
        visibilityRanges = new VisibilityRanges;

    //move everything into positive territory
    if (min < 0)
        min *= -1;
    if (max < 0)
        max *= -1;

    if (min < max) //swap if backward.
    {
        float temp = min;
        min = max;
        max = temp;
    }
    visibilityRanges->minScale = min;
    visibilityRanges->maxScale = max;
    visibilityRanges->useScale = true;
    if (commonRange && rangesLeft--)
        visibilityRanges->commonRange = commonRange;
}

/*---------------------------------------------------------------------------*/
void Object::setRenderWindow(Window3D *win)
{
    if (win != onlyWin)
    {
        onlyWin = win;
        globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
    }
}

Object * Object::GetObjectPtr(int id)
{
    if(id == this->id)
        return this;
    return NULL;
}

Object * Object::GetObjectPtr(std::string const &l)
{
    if(!GetLabel().empty() && l == GetLabel())
        return this;
    return NULL;
}

/*---------------------------------------------------------------------------*/
/** Display info about this object in the info window on the GUI.
*/
void Object::ShowInfo(GLuint *names, int numNames)
{
    Scene3D *scene;
    std::shared_ptr<TextureMap> texture;

    if (MainWindow::getInterfaceType() == GeoCoastPilot)
        return; //don't show info for these objects.

    scene = mainWindow->getScene3D();

    scene->appendInfo("Type:\t"+getRegistrationEntry()->className+"\n");
    scene->appendInfo("Name:\t"+GetLabel()+"\n");

    scene->appendInfo("File:\t"+(GetFilename().empty())? "<none>" : GetFilename(),true);

    std::stringstream bp;
    bp << "Base Position:\t" << GetPosition(0) << ", " << GetPosition(1) << ", " << GetPosition(2) << "\n";
    scene->appendInfo(bp.str());

    texture = GetCustomTexture();
    scene->appendInfo("Texture:"+((texture)? GetCustomTexture()->GetLabel() : "<default>"),true);
    scene->appendInfo("--------------------\n");
}

void Object::getHtmlInfo(std::stringstream &html, short level)
{
    Scene3D *scene;
    int i;
    SmartValue a;

    scene = mainWindow->getScene3D();

    html << "<h" << level << ">Object:</h" << level << ">";
    if (getAddress())
        html << "<a href=\"/address/" << getAddress()->getAddress() << "\">" << getAddress()->getAddress() << "</a><br>";
    html << "<a href=\"/object/" << GetId() << "\">Object: " << GetId() << "</a><br>";

    if (visible)
        html << "Currently shown: <a href=\"/command/gz/tellObject/" << GetId() << "/hide\">hide</a><br>";
    else
        html << "Currently hidden: <a href=\"/command/gz/tellObject/" << GetId() << "/show\">show</a><br>";

    if (pose)
    {
        if(pose->getAddress())
            html << "Pose:  <a href=\"/address/" << pose->getAddress()->getAddress() << "\">" << pose->getAddress()->getAddress() << "</a><br>";
        else
            html << "Pose:  No address!<br>";

        html << "<form action=\"/command/gz/tellObject/" << GetId() << "/minimumPixelsPerMeter/i\">Minimum Pixels per Meter: <input type=\"text\" name=\"i\" value=\"" << pose->getMinimumPixelsPerMeter() <<"\"/>(Set to 0 to dissable)<input type=\"submit\" value=\"Set\"/></form>";

    }
    else
        html << "Pose:  NULL<br>";
    if (poseHistory)
        poseHistory->getHtmlInfo(html, level);
    html << "<table><tr>";
    html << "<th>Type</th>";
    html << "<th>Name</th>";
    html << "<th>File</th>";
    html << "<th>Base Position</th>";
    html << "<th>Texture</th></tr>";
    
    html << "<tr>";
    html << "<td>" << getRegistrationEntry()->className << "</td>";
    html << "<td>" << GetLabel() << "</td>";
    html << "<td>" << (GetFilename().empty() ? "<none>" : GetFilename()) << "</td>";
    html << "<td>" << GetPosition(0) << ", " << GetPosition(1) << ", " << GetPosition(2) << "</td>";
    html << "<td>" << (GetCustomTexture()? GetCustomTexture()->GetLabel() : "<default>") << "</td></tr></table>";
    if(numAttributes)
    {
        html << "<h" << level-2 << ">Attributes</h" << level-2 << ">";
        html << "<table>";
        for(int g = 0; g <= numAttributes/5;g++)
        {
            html << "<tr>";
            for(i=g*5;i<g*5+5 && i<numAttributes;i++)
                html << "<th>" << attributeNames[i] << "</th>";
            html << "</tr><tr>";
            for(i=g*5;i<g*5+5 && i<numAttributes;i++)
            {
                getAttribute(a,i);
                html << "<td>" << a.asString() << "</td>";
            }
            html << "</tr>";
        }
        html << "</table>";
    }
    
}

/*---------------------------------------------------------------------------*/
/** When an object is picked, we want to put some of its info in the info
*   window on the GUI.
*/
int Object::pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
    if (pickType == PICK_CLICK1 || (pickType == PICK_PRESS1 && respondToPress))
    {
        if (pickType & PICK_PRESS1)
        {
            Scene3D *scene;

            scene = mainWindow->getScene3D();
            scene->resetInfo();
        }

        ShowInfo(&names[1], names[0]);
        if ((pickType & PICK_PRESS1) && this == selectedObject)
        {
            return pointHandle->pickAction(pickType, inputInfo, names, hits);
        }

        if (pickType == PICK_CLICK1)
            highLevelEventMediator->Trigger(objectClickedInputType->value, pose, (void *) -1);

        setSelectedObject(this);
        highLevelEventMediator->Trigger(objectSelectedInputType->value, pose, (void *) -1);
        if (pickType & PICK_PRESS1) //only happens if we are responding to press events (not the default)
        {
            Window3D *window;

            DYNAMIC_CAST(window, Window3D*, inputInfo->window);
            pointHandle->dragObject(this, window);
            doubleClickInteractor->reset();
        }
        return GL_TRUE;
    }
    else if (pickType == PICK_PRESS2)
    {
        Window3D * currentWindow = Window3D::cast(inputInfo->window);
        ViewManager *currentView;
#ifdef LOCK_TO_GROUPS
        PosableStateRelationship *rel;
        RelationshipQueryResult *potentialGroups;
        CompositeObject *group = NULL;
        Object *intendedLevelOfSelection;
        PosableStateBase *groupPose = NULL;
        PosableStateInfo pickInfo, groupInfo;
        GLdouble screenPos[3], worldPos[3];
        float viewScale, groupScale, bestScale = 1000000.0f;
#endif

        if (!currentWindow)
            return GL_FALSE;

        currentView = currentWindow->getViewInfo();
        if (currentView)
        {
#ifndef LOCK_TO_GROUPS
            currentView->lockTo(this);
            setFrameOfReference(this);
#else
            //loop through relationships involving us to check scale,
            //and pick the one that's closest without going to large.
            //Assume they just mean this object in the simplest case.
            intendedLevelOfSelection = this;
            viewScale = currentView->getPosable()->getScale();
            screenPos[X] = (double)inputInfo->x.i;
            screenPos[Y] = (double)inputInfo->y.i;
            screenPos[Z] = -1.0;
            currentView->getWorldPoint(screenPos, worldPos);
            pickInfo.scale = viewScale;
            pickInfo.position[X] = (float)worldPos[X];
            pickInfo.position[Y] = (float)worldPos[Y];
            pickInfo.position[Z] = (float)worldPos[Z];
            potentialGroups = relationshipManager->getConcerning(pose);
            for (rel = potentialGroups->first(); rel; rel = potentialGroups->next())
            {
                DYNAMIC_CAST(group, CompositeObject *, rel);
                if (!group)
                    continue;
                groupPose = group->getPosable();
                if (!groupPose)
                    continue;
                groupScale = groupPose->getScale();
                if (groupScale >= viewScale && groupScale < bestScale)
                {
                    groupPose->getPoseInfo(groupInfo);
                    //make sure the group's center is within the future view's
                    //central region.
                    if (WindowPoseState::viewToObjectPoseInfo(pickInfo, groupInfo) <= 1.0)
                    {
                        bestScale = groupScale;
                        intendedLevelOfSelection = group;
                    }
                }
            }
//          cout << "Locking to " << intendedLevelOfSelection->getAddress()->getAddress() << endl;
            currentView->lockTo(intendedLevelOfSelection);
            setFrameOfReference(intendedLevelOfSelection);
#endif //LOCK_TO_GROUPS
        }
        return GL_TRUE;
    }
    else if (pickType == PICK_PRESS3)
    {
        setSelectedObject(this, GL_TRUE);
        return GL_TRUE;
    }
    return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void Object::setSelectedObject(Object *newSelection, GLboolean selectForMenu)
{
    if (!selectForMenu && newSelection && !newSelection->isEditable())
        newSelection = NULL;

    if (newSelection == selectedObject)
        return;

    if (selectedObject)
    {
        selectedObject->selectedFlag = GL_FALSE;
        selectedObject->usingInternalHandleFlag = GL_FALSE;
    }
    selectedObject = newSelection;
    if (selectedObject)
        selectedObject->selectedFlag = GL_TRUE;
    if (!selectedObject)
        pointHandle->reset();

    if (!selectForMenu || (selectedObject && selectedObject->isEditable()))
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/*---------------------------------------------------------------------------*/
void Object::setFrameOfReference(Object *newFoR)
{
    frameOfReference = newFoR;
}

/*---------------------------------------------------------------------------*/
/** When an object is picked, we want to put some of its info in the info
*   window on the GUI.
*/
void Object::renderPickableObject(Window3D *inWindow)
{
    GLboolean oldval = doneFlag;

    if (isVisible(inWindow) && !IsGarbage())
    {
        renderingForPicking = GL_TRUE;
        if (!selectedFlag || canUseInternalHandleFlag)
        {
            glPushName(0);
            glPushMatrix();
            glLoadIdentity();
            pose->applyTransforms(inWindow->getViewInfo());
            DrawLowRes(0.5, inWindow);
            glPopMatrix(); 
            glPopName();
        }
        if (selectedFlag && allowBaseExternalEdit)
            pointHandle->drawIn(inWindow, pose->getPosition(), usingInternalHandleFlag? 2 : 1);
        renderingForPicking = GL_FALSE;
        doneFlag = oldval; //since DrawLowRes might change it!
    }
}

/*---------------------------------------------------------------------------*/
void Object::postContextMenu(InputInfo *inputInfo)
{
    std::stringstream command;

    command << "postObjectMenu " << Togl_Ident(inputInfo->window->GetTogl()) << " " <<
        getRegistrationEntry()->className << " " << GetLabel() << " " << inputInfo->x.i
        << " " << inputInfo->y.i << "{}";
    inputInfo->window->Tcl_Eval(command.str());
    setSelectedObject(this, GL_TRUE);
}

void Object::PostMenu(int id)
{
    std::stringstream command;
    if(id == Object::id)
    {
        command << "global objframe; postObjectMenu $objframe.objects.listframe.objTree " << getRegistrationEntry()->className << " " << GetLabel() << " 0 0 {}";
        mainWindow->Tcl_Eval(command.str());
        setSelectedObject(this, GL_TRUE);
    }

}

/*---------------------------------------------------------------------------*/
int Object::tellSelectedObject(int argc, const char **argv, Tcl_Interp *interp)
{
    int retval = TCL_OK;

    if (selectedObject)
        retval = selectedObject->getPosable()->perform(argc, argv, interp);
    if (selectedObject && !selectedObject->isEditable())
        setSelectedObject(NULL, GL_TRUE);

    return retval;
}

/*---------------------------------------------------------------------------*/
int Object::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    char retval[8] = "";
    GLboolean badSyntax = GL_FALSE;
    int returnval;
    Point3d pos;
    OrientationInfo o;
    double t;
    
    if (argc >= 1)
    {
        std::string message = argv[0];
        if (message == "setPositionOrientationTime")
        {
            if(argc != 8)
            {
                Tcl_AppendResult(interp, "Usage: setPositionOrientationTime x y z heading pitch roll time", NULL);
                return TCL_ERROR;
            }
            pos[0] = atof(argv[1]);
            pos[1] = atof(argv[2]);
            pos[2] = atof(argv[3]);
            if(projectionRequired)
            {
                autoProjection(pos[X],pos[Y]);
                gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                gz4d::Point<double> ret = proj(pos[Y],pos[X]);
                pos[X] = ret[X];
                pos[Y] = ret[Y];
            }
            o.axis[Yaw] = atof(argv[4]);
            o.axis[Pitch] = atof(argv[5]);
            o.axis[Roll] = atof(argv[6]);
            t = atof(argv[7]);
            SetPositionOrientation(pos[0],pos[1],pos[2],o,t);
            return TCL_OK;
        }
        if (message == "setPosition")
        {
            if(argc != 4)
            {
                Tcl_AppendResult(interp, "Usage: setPosition x y z", NULL);
                return TCL_ERROR;
            }
            SetPosition(atof(argv[1]),atof(argv[2]),atof(argv[3]));
            return TCL_OK;
        }   
        if (message == "setPositionTime")
        {
            if(argc != 5)
            {
                Tcl_AppendResult(interp, "Usage: setPosition x y z t", NULL);
                return TCL_ERROR;
            }
            SetPosition(atof(argv[1]),atof(argv[2]),atof(argv[3]),atof(argv[4]));
            return TCL_OK;
        }   
        if (message ==  "setRT")
        {
            mainWindow->getScene3D()->timeControl->setRTsource(this);
            mainWindow->getScene3D()->timeControl->useRT();
            return TCL_OK;
        }
        if (message ==  "hide")
        {
            SetInvisible();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
        if (message ==  "show")
        {
            SetVisible();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
        if (message ==  "info")
        {
            ShowInfo();
            return TCL_OK;
        }
        if (message ==  "save")
        {
            
            return TCL_OK;
        }
        if (message ==  "reload")
        {
            mainWindow->getScene3D()->ReloadObject(this);
            return TCL_OK;
        }
        if (message == "delete")
        {
            //mainWindow->getScene3D()->RemoveObject(this, interp);
            Unload();  // rja - the above doesn't remove it from the "gz posable list Object" list
            return TCL_OK;
        }
        if (message == "wireframe")
        {
            if (argc >= 2)
            {
                showAsWireframe = isTrue(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            sprintf(retval, "%d", showAsWireframe);
            Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "minimumPixelsPerMeter")
        {
            if (argc >= 2)
            {
                pose->setMinimumPixelsPerMeter(atof(argv[1]));
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            sprintf(retval, "%f", pose->getMinimumPixelsPerMeter());
            Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "path")
        {
            if (poseHistory)
                return poseHistory->perform(argc-1,&argv[1],interp);
            Tcl_SetResult(interp,const_cast<char*>("This object doesn't have a path."),TCL_VOLATILE);
            return TCL_ERROR;
        }
        if (message == "hud")
        {
            if (argc >= 2)
            {
                hudFlag = isTrue(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            sprintf(retval, "%d\n", hudFlag);
            Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "clutMode")
        {
            if (argc >= 2)
            {
                clutMode = atoi(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            sprintf(retval, "%d", clutMode);
            Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "edit")
        {
            if (argc >= 2)
            {
                isEditableFlag = atoi(argv[1]);
            }
            if (isEditableFlag)
            {
                setSelectedObject(NULL, GL_TRUE);
                setSelectedObject(this);
            }
            else if (selectedFlag)
                setSelectedObject(NULL);
            return TCL_OK;
        }
        if (message == "transparencyLevel")
        {
            if (argc >= 2)
            {
                transparencyLevel = atoi(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            sprintf(retval, "%d", transparencyLevel);
            Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strncmp(message.c_str(), "respondToPress", 14))
        {
            GLboolean *pressVar;
            char c = message[14];

            if (c == '\0' || c == '1')
                pressVar = &respondToPress;
            else if (c == '2')
                pressVar = &respondToPress2;
            else
                return TellableInterface::perform(argc, argv, interp);

            if (argc >= 2)
            {
                int newVal;
                newVal = atoi(argv[1]);

                if (*pressVar != newVal)
                    *pressVar = newVal;

                if (c == '\0' || c == '1')
                {
                    if (respondToPress)
                    {
                        respondTo(PICK_PRESS1);
                        ignore(PICK_CLICK1);
                    }
                    else
                    {
                        respondTo(PICK_CLICK1);
                        ignore(PICK_PRESS1);
                    }
                }
                else if (c == '2')
                {
                    if (respondToPress2 == GL_TRUE)
                        respondTo(PICK_PRESS2);
                    else
                        ignore(PICK_PRESS2);
                }
            }

            sprintf(retval, "%d", *pressVar);
            Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "respondToClick")
        {
            if (argc >= 2)
            {
                int newVal;
                newVal = atoi(argv[1]);

                if (respondToClick != newVal)
                    respondToClick = newVal;
                else
                    return TCL_OK;

                if (respondToClick)
                {
                    respondTo(PICK_CLICK1);
                }
                else
                {
                    ignore(PICK_CLICK1);
                }
            }

            sprintf(retval, "%d", respondToClick);
            Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "renderWindows")
        {
            Window3D *win = NULL;
            Addressable *address = NULL;

            badSyntax = GL_TRUE;
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "all"))
                {
                    setRenderWindow(NULL);
                    return TCL_OK;
                }
                if (argc >= 3)
                {
                    if (!strcmp(argv[1], "only"))
                    {
                        returnval = gzCommandInterface::findWindow(argv[2], address, win);
                        if (returnval != TCL_OK)
                            return returnval;
                        setRenderWindow(win);
                        return TCL_OK;
                    }
                }
            }
            if (badSyntax)
            {
                Tcl_AppendResult(interp, "Usage: renderWindows {all | only <window>}", NULL);
                return TCL_ERROR;
            }

        }
        if(message == "goto")
        {
            mainWindow->getViewInfo()->getPosable()->setPosition(getPreferredOffset());
            globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
            return TCL_OK;
        }
        if (message ==  "connect")
        {
            gz4d::UDPSocket socket;
            socket.Bind(gz4d::IPSocket::Address());
            gz4d::IPSocket::Address la;
            socket.GetLocalAddress(la);
            socketThread = gz4d::SocketReceiver::Ptr(new gz4d::SocketReceiver(socket));
            socketQueue = gz4d::ThreadedQueue<std::shared_ptr<gz4d::Socket::Packet> >::Ptr(new gz4d::ThreadedQueue<std::shared_ptr<gz4d::Socket::Packet> >);
            socketThread->AddOutQueue(socketQueue);
            Tcl_SetResult(interp, const_cast<char *>(boost::lexical_cast<std::string>(la.GetPort()).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "projection")
        {
            if (argc == 2 && !strcmp(argv[1],"Geographic"))
                projectionRequired = true;
            return TCL_OK;
        }
        if (message == "setTexture")
        {
            if (argc == 2)
            {
                SetCustomTexture(mainWindow->getScene3D()->GetTexturePtr(argv[1]));
                return TCL_OK;
            }
        }
        if (message == "select")
        {
            if (argc == 2)
            {
                if (atoi(argv[1]))
                    setSelectedObject(this, true);
                else
                    setSelectedObject(NULL);
            }
            sprintf(retval, "%d", selectedFlag);
            Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
            return TCL_OK;
        }
    }
    return PosableStateUser::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Object::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "hide show info reload delete wireframe edit clutMode transparencyLevel respondToPress respondToPress2 respondToClick renderWindows goto select", NULL);
    return PosableStateUser::list(interp);
}

/*---------------------------------------------------------------------------*/
BBox Object::GetBoundingBox()
{
    if(poseHistory)
    {
        BBox ret = bbox;
        ret.end += poseHistory->getTimeOffset();
        ret.start += poseHistory->getTimeOffset();
        return ret;
    }
    return bbox;
}

BBox* Object::GetBoundingBoxPtr()
{
return & bbox;
}


/*---------------------------------------------------------------------------*/
void Object::SetDefaultTexture(std::shared_ptr<std::shared_ptr<TextureMap> > &ptr)
{
    defaulttextureptr = ptr;
//  if (customtextureptr == NULL) //no need to update if we have a custom one.
//      UpdateTexture(*ptr);
}

/*---------------------------------------------------------------------------*/
void Object::SetCustomTexture(std::shared_ptr<TextureMap> &ptr)
{
    customtextureptr = ptr;
    UpdateTexture();
}

void Object::SetCustomTexture(int oid, std::shared_ptr<TextureMap> &t)
{
    if(oid == id)
        SetCustomTexture(t);
}

/*---------------------------------------------------------------------------*/
void Object::UpdateTexture()
{
    if (customtextureptr)
        UpdateTexture(customtextureptr);
    else if (defaulttextureptr)
        UpdateTexture(*defaulttextureptr);
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<TextureMap> &Object::GetCurrentTexture()
{
    if (customtextureptr)
        return customtextureptr;
    else
        return *defaulttextureptr;
}

/*---------------------------------------------------------------------------*/
void Object::EnableTexture(int mode)
{
    std::shared_ptr<TextureMap> currentTexture;
    Point3d offset;

    if (!customtextureptr)
        currentTexture = *defaulttextureptr;
    else
        currentTexture = customtextureptr;

    if(useTextures)
    {
        if (mode == TEXTURE_COORD_USER)
            mode = clutMode;
        if(mode == TEXTURE_COORD_POSABLE)
        {
            GetPosition(offset);
            currentTexture->Enable(offset);
        }
        else
            currentTexture->Enable();  //User must specify texture coordinates
    }
}

/*---------------------------------------------------------------------------*/
void Object::DisableTexture()
{
    if (customtextureptr == NULL)
    {
        (*defaulttextureptr)->Disable();
    }
    else
    {
        customtextureptr->Disable();
    }
}

/*---------------------------------------------------------------------------*/
//const Point4ub *Object::GetColorAtValue(float val)
gz4d::Pixel<gz4d::RGBA8> Object::GetColorAtValue(float val) const
{
    std::shared_ptr<TextureMap> texture;
    std::shared_ptr<Clut> clut;
    //const Point4ub *retval = NULL;
    //static Point4ub nonColor = {255, 255, 255, 255};

    if (!customtextureptr)
        texture = *defaulttextureptr;
    else
        texture = customtextureptr;

    clut = std::dynamic_pointer_cast<Clut>(texture);

    boost::optional<gz4d::Pixel<gz4d::RGBA8> > retval;
    
    if (clut)
        retval = clut->GetColorAtValue(val);

    if (retval)
        return *retval;
//     else
//         return &nonColor;
    return gz4d::Pixel<gz4d::RGBA8>();
}

/*---------------------------------------------------------------------------*/
void Object::UpdateCustomTexture()
{
    if (customtextureid == 0)
    {
        customtextureptr.reset();
    }
    else
    {
        customtextureptr = mainWindow->getScene3D()->GetTexturePtr(customtextureid - 1);
    }
}

void Object::SetTidePtr(TimeVaryingObject ** t)
{
    tide = t;
}


void Object::SetNavPathPtr(NavigationPath ** n)
{
    navPath = n;
}

/*--------------------------------------------------------------------------*/
/** This function resets this object to start over drawing high res the next
*   time DrawHighRes is called.
*/
void Object::ResetHighRes()
{
doneFlag = 0;
}

/*--------------------------------------------------------------------------*/
/** This function returns the height at the passed location.  Returns
*   invalidHeight if a height is not found at this location.  If a Point3f
*   is passed, it is filled with the result for an x and y it knows more
*   exactly.
*/
double Object::GetHeightAt(double x, double y, Point3d coord, GLboolean secondary)
{
return invalidHeight;
}

/*--------------------------------------------------------------------------*/
/** This function returns a height summary of the specified area surrounding
*   the passed location.  Defaults to the maximum height (Object::height_max)
*   but can also do Object::height_min and Object::height_average.  Returns
*   -1.0 if a height is not found near this location or if the range doesn't
*   cross any cells in the GUTM.  The area is always a box with the length
*   of twice the rangeMeters parameter on each side.
*   Function created by mdp 1/3/2006 to support vessel shadow functionality.
*/
double Object::GetHeightNear(double x, double y, float rangeMeters, int heightType)
{
    return invalidHeight;
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<TextureMap> &Object::GetCustomTexture()
{
    return customtextureptr;
}

/*---------------------------------------------------------------------------*/
void Object::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
    if (!editedFlag && isEditableFlag == GL_TRUE) //speed the check
        setEdited();
    if (respondToPress2 == GL_FALSE && isEditableFlag != GL_TRUE && !settingOffset)
    {
        respondTo(PICK_PRESS2);
        respondToPress2 = GL_TRUE;
    }
    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    highLevelEventMediator->Trigger(objectPoseChangedInputType->value, whichPose, (void *)whatChanged);
    if(parent)
        parent->setBBoxStale();
}

/*---------------------------------------------------------------------------*/
void Object::PoseContinuouslyChanging(PosableStateBase *whichPose,
                                    GLboolean isChanging)
{
    if (!isChanging && (whichPose == pose))
        highLevelEventMediator->Trigger(objectStabilizedInputType->value, whichPose);
}

/*---------------------------------Class Registration------------------------*/
/*---------------------------------------------------------------------------*/
const ObjectRegistrationEntry *Object::registerObject(const char *classname,
                            const char *extension,
                            Object *(*instantiating_fn)(),
                            const char *ctextMenu)
{
int i;
ObjectRegistrationEntry *newEntry, **newRegistry;

//create the new entry item.
newEntry = new ObjectRegistrationEntry;
newEntry->className = classname;
newEntry->extension = extension;
newEntry->create = instantiating_fn;
newEntry->contextMenu = ctextMenu;


//expand the registry if needed
if (numEntries >= maxEntries)
    {
    maxEntries += REGISTRY_GROWTH_STEP;
    newRegistry = new ObjectRegistrationEntry*[maxEntries];
    for (i = 0; i < numEntries; i++)
    newRegistry[i] = objectRegistry[i];
    delete [] objectRegistry;
    objectRegistry = newRegistry;
    }

//add the item.
objectRegistry[numEntries++] = newEntry;

return newEntry;
}

/*---------------------------------------------------------------------------*/
ObjectRegistrationEntry *Object::getRegistryEntry(std::string const &classname)
{
    int i;
    ObjectRegistrationEntry *currentEntry;

    for (i = 0; i < numEntries; i++)
    {
        currentEntry = objectRegistry[i];
        if (classname == currentEntry->className)
            return currentEntry;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
ObjectRegistrationEntry *Object::getRegistryEntryFromExt(std::string const &ext)
{
    int i;
    ObjectRegistrationEntry *currentEntry;

    for (i = 0; i < numEntries; i++)
    {
        currentEntry = objectRegistry[i];
        if (ext == ('.'+currentEntry->extension))
            return currentEntry;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
Object *Object::createFromClassname(std::string const &classname)
{
    Object *ob;

    ObjectRegistrationEntry *entry = getRegistryEntry(classname);
    if (entry)
    {
        ob = entry->create();
        if (!entry->aliasFor.empty())
            ob->initializeAs(classname);
        return ob;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
Object *Object::createFromExtension(std::string const &ext)
{
    Object *ob;

    ObjectRegistrationEntry *entry = getRegistryEntryFromExt(ext);
    if (entry)
    {
        ob = entry->create();
        if (!entry->aliasFor.empty())
            ob->initializeAs(entry->className);
        return ob;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
const ObjectRegistrationEntry *Object::registerAlias(const char *aliasname,
                            const char *classname, const char *extension)
{
    int i;
    ObjectRegistrationEntry *newEntry, **newRegistry, *aliased;
    
    aliased = getRegistryEntry(classname);
    assert(aliased); //somebody mispelled the classname for the alias!
    
    //create the new entry item.
    newEntry = new ObjectRegistrationEntry;
    newEntry->className = aliasname;
    if (extension)
        newEntry->extension = extension;
    else
        newEntry->extension = aliased->extension;
    newEntry->create = aliased->create;
    newEntry->contextMenu = aliased->contextMenu;
    newEntry->aliasFor = classname;
    
    //expand the registry if needed
    if (numEntries >= maxEntries)
    {
        maxEntries += REGISTRY_GROWTH_STEP;
        newRegistry = new ObjectRegistrationEntry*[maxEntries];
        for (i = 0; i < numEntries; i++)
            newRegistry[i] = objectRegistry[i];
        delete [] objectRegistry;
        objectRegistry = newRegistry;
    }
    
    //add the item.
    objectRegistry[numEntries++] = newEntry;
    
    return newEntry;
}

void Object::HandlePacket(std::shared_ptr<gz4d::Socket::Packet> &data)
{
    gz4d::binary::Navigation::Position6DOF::Geographic *nav = reinterpret_cast<gz4d::binary::Navigation::Position6DOF::Geographic*>(&data->front());
    if(nav->IsValid() && !gz4d::IsNan(nav->longitude) && !gz4d::IsNan(nav->latitude))
    {
        Point3d pos;
        OrientationInfo o;

        pos[0] = nav->longitude;
        pos[1] = nav->latitude;
        if(gz4d::IsNan(nav->z))
            pos[2] = 0.0;
        else
            pos[2] = nav->z;
        if(projectionRequired)
        {
            autoProjection(pos[X],pos[Y]);
            gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
            gz4d::Point<double> ret = proj(pos[Y],pos[X]);
            pos[X] = ret[X];
            pos[Y] = ret[Y];
        }
        o.axis[Yaw] = nav->heading;
        o.axis[Pitch] = nav->pitch;
        o.axis[Roll] = nav->roll;
        SetPositionOrientation(pos[0],pos[1],pos[2],o,nav->timestamp.secondsSinceEpoch().asSeconds());
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Object::Idle(double time)
{
    double height;

    if(!pendingCustomTexture.empty())
    {
        SetCustomTexture(mainWindow->getScene3D()->GetTexturePtr(pendingCustomTexture));
        pendingCustomTexture.erase();
    }

    while(socketQueue && !socketQueue->empty())
    {
        std::shared_ptr<gz4d::Socket::Packet> data = socketQueue->pop();
        HandlePacket(data);
    }

    if(!fileQueue.empty() && !currentFile)
    {
        currentFile = std::shared_ptr<std::ifstream>(new std::ifstream);
        std::cerr << fileQueue.front() << std::endl;
        currentFile->open(fileQueue.front().string().c_str(),std::ios::in|std::ios::binary);
        fileQueue.pop();
    }

    if(currentFile)
    {
        currentFile->clear();
        for(int c = 0; !currentFile->eof() && c < 50; ++c)
        {
            std::shared_ptr<std::vector<char> > data(new std::vector<char>(sizeof(gz4d::binary::Packet)));
            currentFile->read(&data->front(),sizeof(gz4d::binary::Packet));
            if(currentFile->eof())
                break;
            std::size_t size = reinterpret_cast<gz4d::binary::Packet const *>(&data->front())->size;
            data->resize(size);
            currentFile->read(&((*data)[sizeof(gz4d::binary::Packet)]),size-sizeof(gz4d::binary::Packet));
            if(currentFile->eof())
                break;
            HandlePacket(data);
        }
        if(currentFile->eof() && !fileQueue.empty())
            currentFile.reset();
    }

    float scale = mainWindow->getScene3D()->timeControl->getScale();
    if(lastIdleTime > (time-scale/1000.0) && lastIdleTime < (time+scale/1000.0))
    {
        return;
    }
        
    lastIdleTime = time;
    
    gz4d::Point<double> ip;
    gz4d::Point<double> pp , p2;
    OrientationInfo o;
    const OrientationInfo *op = NULL;
    double alt;


    if(poseHistory)// && !mainWindow->getScene3D()->timeControl->isRT())
    {
        poseHistory->line->Idle(time + poseHistory->getTimeOffset() );

        if(hideOutsideTimeBounds)
        {
            if(poseHistory->getStartTime() > time + poseHistory->getTimeOffset() + preShowTime)
                outsideTimeBounds = 1;
            else if(poseHistory->getEndTime() < time + poseHistory->getTimeOffset() - postShowTime)
                outsideTimeBounds = 1;
            else
                outsideTimeBounds = 0;
        }

        if(interpolatePositions)
        {
            if (poseHistory->getPositionOrientation(time, ip, o, alt))
            {
                if(alt >= 0)
                    // are we deeper than we are high off the bottom?
                    if(fabs(ip[2])>alt)
                    {
                        height = mainWindow->getScene3D()->GetHeightAt(ip[0],ip[1]);
                        if(height != invalidHeight)
                            ip[2] = height+alt;
                    }
                pp = ip;
                op = &o;
            }
        }
        else if (poseHistory->isNext(time + poseHistory->getTimeOffset() ))
        {
            pp = poseHistory->getPosition();
            op = poseHistory->getOrientation();
        }

        if (pp.IsValid())
        {
            pose->AccumulateNotifications();
            pose->setPosition(pp);
            if(calculateCOG)
            {
                poseHistory->next();
                p2 = poseHistory->getPosition();
                if (p2.IsValid())
                {
                    cog=atan2((p2[Y]-pp[Y]),(p2[X]-pp[X]));
                    cog*=180.0f/3.14f;
                    cog = 90-cog;
                    pose->setOrientation(0.0,cog);
                }
            }
            else if (op)
                pose->setOrientation(*op);
            pose->FlushNotifications();
        }
    }

    return;
}

void Object::SaveXML(FILE * out, std::string const &path)
{
    fprintf(out,"<Object ");
    if(!GetFilename().empty())
        fprintf(out,"filename=\"%s\" ",(fs::path(path)/GetFilename()).string().c_str());
    SaveXMLattributes(out);
    fprintf(out,"/>\n");
}

void Object::SaveXMLattributes(FILE * out)
{
    if(!GetLabel().empty())
        fprintf(out,"label=\"%s\" ",GetLabel().c_str());
    if(customtextureptr)
        fprintf(out,"texture=\"%s\" ",customtextureptr->GetLabel().c_str());
}

Object * Object::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    std::string text;
    Point3d gr;
    ObjectCollection *newParent = NULL;
    Object *ob;
    Addressable *address;
    Scene3D *scene3d = mainWindow->getScene3D();

    pendingCustomTexture = xml.get("<xmlattr>.texture",pendingCustomTexture);

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.usetexture"))
    {
        if (gz4d::IsTrue(*attribute))
            UseTextures();
        else
            DontUseTextures();
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.rt"))
        if(gz4d::IsTrue(*attribute))
        {
            mainWindow->getScene3D()->timeControl->setRTsource(this);
            mainWindow->getScene3D()->timeControl->useRT();
        }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.parent"))
    {
        text = *attribute;
        if (gzCommandInterface::findObject(text, address, ob, GL_FALSE) == TCL_OK)
        {
            newParent = dynamic_cast<ObjectCollection *>(ob);
            //DYNAMIC_CAST(newParent, ObjectCollection *, ob);
            if (newParent)
                setParent(newParent);
        }
        if (newParent == NULL)
        {
            newParent = scene3d->ensureObjectCollectionPath(text);
            if (newParent)
                setParent(newParent);
        }
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.label"))
        SetLabel(*attribute);
    
    boost::optional<int> int_attrib;
    if(int_attrib = xml.get_optional<int>("<xmlattr>.utm_zone"))
        setUTMProjection(*int_attrib,true);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.spatial_reference"))
    {
        spatialReference = gz4d::ogr::SpatialReference();
        spatialReference.SetFromUserInput(*attribute);
        scene3d->setSpatialReference(spatialReference);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.local_origin"))
    {
        std::stringstream s(*attribute);
        s >> gr[0];
        s >> gr[1];
        s >> gr[2];
        SetPosition(gr);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.orientation"))
    {
        OrientationInfo orient;
        std::stringstream ss(*attribute);
        ss >> orient.axis[Roll] >> orient.axis[Pitch] >> orient.axis[Yaw];
        pose->setOrientation(orient);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.lockview"))
        if (gz4d::IsTrue(*attribute))
            mainWindow->getViewInfo()->lockTo(this);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.referable"))
        referable = gz4d::IsTrue(*attribute);

    transparencyLevel = xml.get("<xmlattr>.transparency",transparencyLevel);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.spheroid"))
        spheroidFlag = gz4d::IsTrue(*attribute);

    spheroid_x = xml.get("<xmlattr>.spheroidx",spheroid_x);
    spheroid_y = xml.get("<xmlattr>.spheroidy",spheroid_y);
    spheroid_z = xml.get("<xmlattr>.spheroidz",spheroid_z);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.spheroidcolor"))
    {
        std::stringstream sc(*attribute);
        sc >> spheroid_color[0] >> spheroid_color[1] >> spheroid_color[2];
    }

    boost::optional<double> double_attrib;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.path"))
        if(gz4d::IsTrue(*attribute))
        {
            if (!poseHistory)
                poseHistory = std::shared_ptr<ObjectHistory>(new ObjectHistory(this));

            poseHistory->setVisible();

            gz4d::Color color;

            if(attribute = xml.get_optional<std::string>("<xmlattr>.pathColor"))
            {
                std::stringstream c(*attribute);
                c >> color[0] >> color[1] >> color[2];
                poseHistory->line->setColor(color);
            }

            if(attribute = xml.get_optional<std::string>("<xmlattr>.pathHighLightColor"))
            {
                std::stringstream c(*attribute);
                c >> color[0] >> color[1] >> color[2];
                poseHistory->line->setHighlightColor(color);
            }

            

            if(double_attrib = xml.get_optional<double>("<xmlattr>.pathStartOffset"))
                poseHistory->setStartShow(*double_attrib);

            if(double_attrib = xml.get_optional<double>("<xmlattr>.pathEndOffset"))
                poseHistory->setEndShow(*double_attrib);

            if(double_attrib = xml.get_optional<double>("<xmlattr>.pathStartHighlight"))
                poseHistory->setStartHighlight(*double_attrib);

            if(double_attrib = xml.get_optional<double>("<xmlattr>.pathEndHighlight"))
                poseHistory->setEndHighlight(*double_attrib);
        }

    if(double_attrib = xml.get_optional<double>("<xmlattr>.minimumPixelsPerMeter"))
        pose->setMinimumPixelsPerMeter(*double_attrib);


    float as_radius = xml.get<float>("<xmlattr>.acoustic_signature",0.0);
    if(as_radius>0.0)
    {
        acousticSignature = new disc(20,as_radius);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.visible"))
        visible = gz4d::IsTrue(*attribute);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.respondToClick"))
    {
        respondToClick = gz4d::IsTrue(*attribute);
        if (respondToClick)
        {
            respondTo(PICK_CLICK1);
            ignore(PICK_PRESS1);
        }
        else
            ignore(PICK_CLICK1);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.respondToPress"))
    {
        respondToPress = gz4d::IsTrue(*attribute);
        if (respondToPress)
        {
            respondTo(PICK_PRESS1);
            ignore(PICK_CLICK1);
        }
        else
            ignore(PICK_PRESS1);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.respondToPress2"))
    {
        respondToPress2 = gz4d::IsTrue(*attribute);
        if (respondToPress2 == GL_TRUE)
            respondTo(PICK_PRESS2);
        else
            ignore(PICK_PRESS2);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.wireframe"))
        showAsWireframe = gz4d::IsTrue(*attribute);

    preShowTime = xml.get("<xmlattr>.preshowtime",preShowTime);
    postShowTime = xml.get("<xmlattr>.postshowtime",postShowTime);

    scalePriority = xml.get("<xmlattr>.priority",scalePriority);

    if (parent && (scalePriority == 0.0))
        scalePriority = parent->getPriority();

    for(bpt::ptree::const_iterator f = xml.begin(); f != xml.end(); ++f)
        if(f->first == "File")
            fileQueue.push(fs::path(path)/f->second.data());
    
    // seems to be some methods not yet commited to libgz4d!!!
/*    if(xml.getFloatTuple(min, max, "headingRange"))
        setHeadingRange(min, max);
    if(xml.getFloatTuple(min, max, "scaleRange"))
        setScaleRange(min, max);
    if(xml.getFloatTuple(min, max, "distanceRange"))
        setDistanceRange(min, max);*/
    // the following two must be below the heading/scale/distanceRange checks.


/*    if(xml.getDoubleTriple(gr, "commonRangeStart"))
        startCommonRange(gr);
    if(xml.getInt(val, "commonRangeEndAfter"))
        endCommonRange(val);*/
    return this;
}

/** If rendering uses textures, this will enable them. */
void Object::UseTextures()
{
    useTextures = 1;
}

/** Disables the use of textures. Overides EnableTexture call for this object instance in the rendering loop.  */
void Object::DontUseTextures()
{
    useTextures = 0;
}

void Object::MergeBBox(BBox b)
{
    if(bbox.merge(b) && parent)
    {
        parent->setBBoxStale();
    }
}

/** This sets the path where this object adds itself on the object tree widget*/
void Object::SetTreePath(char const * tpath)
{
    if(treePath)
    {
        delete[] treePath;
    }
    treePath = new char[strlen(tpath)+1];
    strcpy(treePath,tpath);
}

// *** pose stuff ***

/*---------------------------------------------------------------------------*/
int Object::getAttribute(SmartValue &val, int attributeIndex)
{
    switch (attributeIndex)
    {
        case Attr_wireframe:
            val.set(showAsWireframe);
            break;
        case Attr_visible:
            val.set(visible);
            break;
        case Attr_clutMode:
            val.set(clutMode);
            break;
        case Attr_useTextures:
            val.set(useTextures);
            break;
        case Attr_isEditable:
            val.set(isEditableFlag);
            break;
        case Attr_respondToPress:
            val.set(respondToPress);
            break;
        case Attr_respondToPress2:
            val.set(respondToPress2);
            break;
        case Attr_degradation:
            if (keytable)
                val.set(Keyword(degradationFlag));
            else
                val.set((const char *)"OFF");
            break;
        default:
            return TCL_ERROR;
            break;
    }
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
int Object::setAttribute(SmartValue &val, int attributeIndex)
{
    int lookupVal;

    switch (attributeIndex)
    {
        case Attr_wireframe:
            showAsWireframe = val.asBool();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            break;
        case Attr_visible:
            visible = val.asInteger();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            break;
        case Attr_clutMode:
            clutMode = val.asInteger();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            break;
        case Attr_useTextures:
            useTextures = val.asInteger();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            break;
        case Attr_isEditable:
            isEditableFlag = val.asBool();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            break;
        case Attr_respondToPress:
            respondToPress = val.asInteger();
            break;
        case Attr_respondToPress2:
            respondToPress2 = val.asInteger();
            break;
        case Attr_degradation:
            lookupVal = Lookup(val.asString(), GL_TRUE, GL_TRUE);
            if (lookupVal < 0)
                return TCL_ERROR;
            degradationFlag = lookupVal;
            break;
        default:
            return TCL_ERROR;
            break;
    }
    return TCL_OK;
}

int Object::DrawHighRes(int eye, Window3D *win)
{
    if (doneFlag)
        return doneFlag;

    DrawLowRes(0, win);
    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
            doneFlag = 1;
    return doneFlag;
}

void Object::SetPosition(gz4d::Point<double> const & p)
{
    pose->setPosition(p);
}

void Object::SetPosition(const Point3d p)
{
    pose->setPosition(p);
}

void Object::SetPosition(double x, double y, double z)
{
    Point3d p;

    p[X] = x;
    p[Y] = y;
    p[Z] = z;
    SetPosition(p);
}

void Object::SetPosition(double x, double y, double z, double t, float alt)
{
    OrientationInfo o;
    o.axis[Yaw] = 0.0;
    o.axis[Pitch] = 0.0;
    o.axis[Roll] = 0.0;
    SetPositionOrientation(x,y,z,o,t,alt);
}

void Object::SetLatLon(double lat, double lon)
{
    autoProjection(lon,lat);
    gz4d::ogr::SpatialReference wgs84;
    wgs84.SetWellKnownGeogCS("WGS84");
    gz4d::geo::Projection proj(wgs84, mainWindow->getScene3D()->getSpatialReference());
    SetPosition(proj(lat,lon));
}

void Object::SetLatLon(double lat, double lon, double heading, double t, double pitch, double roll)
{
    autoProjection(lon,lat);
    gz4d::ogr::SpatialReference wgs84;
    wgs84.SetWellKnownGeogCS("WGS84");
    gz4d::geo::Projection proj(wgs84, mainWindow->getScene3D()->getSpatialReference());
    gz4d::Point<double> p = proj(lat,lon);
    OrientationInfo o;
    o.axis[Yaw] = heading;
    o.axis[Pitch] = pitch;
    o.axis[Roll] = roll;
    SetPositionOrientation(p[0],p[1],0,o,t);
}

double Object::GetPosition(int i)
{
    return pose->getPosition()[i];
}

gz4d::Point<double> Object::GetPosition()
{
    return pose->getPosition();
}


void Object::GetPosition(Point3d &p)
{
    p[0] = pose->getPosition()[0];
    p[1] = pose->getPosition()[1];
    p[2] = pose->getPosition()[2];
}

void Object::SetPositionOrientation(double x, double y, double z, OrientationInfo o, double t, float alt)
{
    if (!poseHistory)
        poseHistory = std::shared_ptr<ObjectHistory>(new ObjectHistory(this));

    poseHistory->insert(t, x, y, z, o, alt);
    
    currentTime = t;

    expandBBoxTime(t);
}


void Object::publish(int parent)
{
    std::stringstream tclstr;
    
    if(published)
        return;

    if (MainWindow::getInterfaceType() == GeoNav3D)
    {
        published = true;
        return;
    }

    tclstr << "addobj end ";
    if(!parent)// 0 parent is root
        tclstr << "root"; 
    else
        tclstr << parent;
    tclstr << " " << GetId() << " " << GetLabel() << " " << getRegistrationEntry()->className;

    if (ToglWindow::interp)
    {
        mainWindow->Tcl_Eval(tclstr.str());
        published = true;
    }
}

void Object::setParent(ObjectCollection * p)
{
    DottedAddress *myAddress = getAddress(), *pAddress = p->getAddress(), *oAddress = NULL;

    if (myAddress)
        oAddress = myAddress->getParent();

    if (oAddress && myAddress && pAddress)
        myAddress->reparent(pAddress);
    else if (pAddress && myAddress)
        pAddress->addChild(myAddress);

    parent = p;
    if(!lockToParent)
    {
        lockToParent = new PosableStateLock(this,parent->getPosable(),pose);
        lockToParent->lock(PosableStateLock::LockLocalOwnerDynamic);
    }
    else
    {
        
    }
    if (parent && (scalePriority == 0.0))
        scalePriority = parent->getPriority();
}

void Object::expandBBoxTime(double t)
{
    bbox.expandTime(t);
    if(parent)
        parent->setBBoxStale();
}

/*---------------------------------------------------------------------------*/
void Object::drawObjectExtras(float degradation, Window3D *win)
{
    if (spheroidFlag)
    {
        //glPushMatrix();
        glColor3fv(spheroid_color);
        glScalef(spheroid_x,spheroid_y,spheroid_z);
        glutWireSphere(1.0,10,10);
        //glPopMatrix();
    }

    if(acousticSignature)
        acousticSignature->draw();

    if (poseHistory)
    {
        glPushMatrix();
        glLoadIdentity();
        poseHistory->line->getPosable()->applyTransforms(win->getViewInfo());
        //poseHistory->line->DrawLowRes(degradation, win);
        poseHistory->Draw(lastIdleTime, degradation, win);
        glPopMatrix();
    }
    
    if(hudFlag)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0);//left,right,bottom,top

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glColor3f(1.0,1.0,1.0);
        glDisable(GL_LIGHTING);

        mainWindow->printAt((float)0.01,(float)0.95,0,GetHUDString().c_str(),1,GLUT_BITMAP_HELVETICA_18);

        glEnable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

}

std::string Object::GetHUDString()
{
    std::stringstream hud;
    gz4d::Point<double> p = GetPosition();
    hud << "E" << std::fixed << std::setprecision(1) << p[0] << ", N" <<   p[1] << "   Depth: " << p[2];
    double d = mainWindow->getScene3D()->GetHeightAt(p[0],p[1]);
    if (d != invalidHeight)
        hud << "   Altitude: " << p[2]-d;

    return hud.str();
}

/*--------------------------------------------------------------------------*/
/** Preferred viewing parameters--&&& this is a quick hack until there's
    enough demand to implement preferred components in the posables themselves.
*/
double Object::getPreferredScale()
{
    double scaleVal;
    
    if(!bbox.hasVolume())
        return 0.0;
    
    scaleVal = bbox.e - bbox.w;
    if (scaleVal < bbox.n - bbox.s)
        scaleVal = bbox.n - bbox.s;

    return 0.25 / scaleVal;
}

/*--------------------------------------------------------------------------*/
gz4d::Point<double> Object::getPreferredOffset()
{
    return pose->getPosition();
}

/*--------------------------------------------------------------------------*/
unsigned int Object::objectClickedEvent()
{
    return objectClickedInputType->value;
}

/*--------------------------------------------------------------------------*/
unsigned int Object::objectSelectedEvent()
{
    return objectSelectedInputType->value;
}

/*--------------------------------------------------------------------------*/
unsigned int Object::objectPoseChangedEvent()
{
    return objectPoseChangedInputType->value;
}

/*--------------------------------------------------------------------------*/
unsigned int Object::objectStabilizedEvent()
{
    return objectStabilizedInputType->value;
}
