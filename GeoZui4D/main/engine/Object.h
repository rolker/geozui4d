/* Copyright (c) 2000 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/// Object.h


#ifndef OBJECT_H
#define OBJECT_H

#include<gz4d/glo.h>
#include <sstream>

#include "engine/Persistence.h"
#include "PosableStateUser.h"
#include "interactors/Pickable.h"
#include "engine/ObjectRegistrationEntry.h"
#include "SDV_types.h"
#include "PosableState.h"
#include "tclLinkage.h"
#include "engine/RTSource.h"
#include <gz4d/socket/UDPSocket.h>
#include <gz4d/socket/SocketReceiver.h>
//#include <gz4d/util/Xml.h>
#include <boost/property_tree/ptree.hpp>
#include <gz4d/netcdf.h>
#include <gz4d/Point.h>
#include <gz4d/ogr/SpatialReference.h>
#include <gz4d/Pixel.h>

namespace bpt = boost::property_tree;

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;


class Window3D;
class DottedAddress;
class ObjectHistory;
class TimeVaryingObject;
class NavigationPath;
class ObjectCollection;
//class LinkedList;
class PosableStateLock;
class TextureMap;
class disc;
struct PosableState;
class PosableStateBase;
class Interactor;
class EventType;


/*---------------------------------------------------------------------------*/
/** 
 *  interface for the Object class.
 *
 *  \author Roland Arsenault (rja)
 *  \date 2000
 *  \image html object.jpg 
 *  \image html objectChild.jpg
 */
/*---------------------------------------------------------------------------*/

class Object : public PosableStateUser, public Pickable,virtual public Persistence, public RTSource
{
public:

    Object(GLboolean allocatePosable = GL_TRUE, int pickingTypes = PICK_CLICK1 | PICK_PRESS3);
    virtual ~Object();

    enum {
          SUCCESS = 0,
          FAILURE = -1
    };

    enum ColorName {BLACK,
                    DARK_GREEN,
                    GREEN,
                    CYAN,
                    BLUE,
                    ELEPHANT_GREY,
                    MAGENTA,
                    AUBURN,
                    PERIWINKLE,
                    YELLOW,
                    RED,
                    WHITE,
                    N_COLOR_CODES
    };

    //Bounding box  
    virtual BBox GetBoundingBox();
    BBox * GetBoundingBoxPtr();
    void MergeBBox(BBox b);
    virtual void expandBBoxTime(double t);

    ///\name Position & orientation 
    ///@{
    void SetPosition(gz4d::Point<double> const & p);
    void SetPosition(const Point3d p);
    void SetLatLon(double lat, double lon);
    void SetPosition(double x, double y, double z);
    void SetPosition(double x, double y, double z, double t, float altitude = -1.0);
    void SetLatLon(double lat, double lon, double heading, double t, double pitch = 0.0, double roll = 0.0);
    void SetPositionOrientation(double x, double y, double z,OrientationInfo o,double t, float alt = -1.0);
    double GetPosition(int i);
    gz4d::Point<double> GetPosition();
    void GetPosition(Point3d &p);

    gz4d::ogr::SpatialReference getSpatialReference(){return spatialReference;}
    ///@}

    ///\name Visibility
    ///@{
    virtual void SetInvisible(int id = -1);
    virtual void SetVisible(int id = -1);
    GLboolean isVisible(Window3D *win = NULL);
    void setHeadingRange(float minDegrees, float maxDegrees);
    void setDistanceRange(float min, float max);
    void setScaleRange(float min, float max);
    void startCommonRange(Point3d &pos);
    void endCommonRange(int afterNmore);
    ///@}
    
    ///\name Time-Related
    ///@{
    GLboolean isOutsideTimeBounds() {return outsideTimeBounds;}
    ///@}

    ///Keywords shared by all objects.
    enum ObjectParams {PROJECT, SPATIAL_REFERENCE, LOCAL_ORIGIN, REVERSED_XY, DEGRADATION, ON, OFF, FirstObjectKeyword}; //up here essentially so we know about ON and OFF
    ///\name File-related
    ///@{
    virtual int Load();
    virtual int readFrom(FILE *file);
    virtual int SaveAs(std::string const &fn);
    virtual int save(std::string const &path = "");
    virtual int writeTo(FILE *file);
    virtual void SaveObject(int id, std::string const &fn);
    void requireFullReload(bool req = true) {requiresFullReload = req;}
    bool fullReloadRequired() {return requiresFullReload;}
    ///@}

    int wasEdited() {return editedFlag;}
    void setEdited(GLboolean val = GL_TRUE);
    bool isEditable() {return isEditableFlag;}

    std::string GetSuffix(); // returns filename suffix
    bool SetLabel(const std::string& l);

    virtual Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    virtual void SaveXML(FILE * out, std::string const &path);
    virtual void SaveXMLattributes(FILE * out);

    int isPublished(){return published;};
    virtual void publish(int parent);
    void dontPublish(){published = -1;};

    //Indicators of use by other objects
    virtual void setTimeIndex(int i) {timeIndex = i;} //used by objects that are used for time varying objects
    bool isReferable(){return referable;};

    //Preferred viewing parameters--&&& this is a quick hack until there's enough demand
    //to implement preferred components in the posables themselves.
    virtual double getPreferredScale();
    virtual gz4d::Point<double> getPreferredOffset();

    //ID
    void replace(Object *);
    virtual Object * GetObjectPtr(int id);
    virtual Object * GetObjectPtr(std::string const &l);

    //Object Hierarchy
    virtual void setParent(ObjectCollection * p);
    ObjectCollection *getParent() {return parent;}
    void SetTreePath(char const * tpath); //Where in the Objects window (Tcl side) should the object appear

    //Class determination
    static const ObjectRegistrationEntry *registerObject(const char *classname, const char *extension, Object *(*instantiating_fn)(), const char *ctextMenu);
    static const ObjectRegistrationEntry *registerAlias(const char *aliasname, const char *classname, const char *extension = NULL);
    virtual const ObjectRegistrationEntry *getRegistrationEntry() = 0;
    static ObjectRegistrationEntry **getObjectRegistry() {return objectRegistry;}
    static int getObjectRegistrySize() {return numEntries;}
    static ObjectRegistrationEntry *getRegistryEntry(const std::string& classname);
    static ObjectRegistrationEntry *getRegistryEntryFromExt(std::string const &ext);
    static Object *createFromClassname(const std::string& classname);
    static Object *createFromExtension(std::string const &ext);

    //Rendering
    /// Renders the object in low resolution suitable for interactivity.
    /// The degradation parameter tells us how fast we must render. As is gets
    /// closer to 1.0, we must draw more quickly, usualy by eliminating details.
    /// @param degradation From 0.0 for full detail to 1.0 for minumum details.
    /// @param win Can be used to query view parameters, such as pixel-size-in-meters, etc.
    virtual void DrawLowRes(float degradation, Window3D *win)=0;

    virtual void DrawOrtho(){}

    /// Renders the object in full resolution.
    /// When the scene is not changing, the object gets rendered in chunks.
    /// Rendering a chunk, and returning allows the hi resolution drawing to be quickly
    /// interrupted in order to start interacting.
    /// @param eye Eye for which are we rendering.
    /// @param win Can be used to query view parameters, such as pixel-size-in-meters, etc.
    /// @return 1 we are done, 0 we still need to render.
    virtual int DrawHighRes(int eye, Window3D *win);
    
    /// Allows us to optionally drill a whole in ourselves.
    /// Normally used for grids or images where data covering 
    /// the same area can exist at multiple resolutions. In such
    /// a case, if the passed in object is of finner resolution
    /// we should create a whole in ourself where we are covered
    /// by other.
    /// @param other Object that wants to drill a hole in us.
    virtual void Fuse(Object *other){};
    
    virtual void useNewHeightExaggeration(GLfloat heightEx){lastHeightExaggeration = heightEx;};
    virtual void ResetHighRes();
    virtual void ShowInfo(GLuint *names = NULL, int numNames = 0);
    virtual void getHtmlInfo(std::stringstream &html, short level);
    float getPriority() {return scalePriority;}
    virtual void setPriority(float newPriority) {scalePriority = newPriority;}
    
    /// Allows us to take advantage of idle cycle and/or update our time.
    /// Provides the current time, so we can update ourself if necessary.
    /// @param time Current GeoZui time in seconds since 1970 
    virtual void Idle(double time);
    
    bool asWireframe() { return showAsWireframe;}
    void setWireframe(bool val = true) {showAsWireframe = val;}
    void drawObjectExtras(float degradation, Window3D *win);
    int getDegradationFlag() {return degradationFlag;}
    int isRenderingForPicking(){return renderingForPicking;};

    void setRenderWindow(Window3D *win);

    bool showSpheroid(){return spheroidFlag;};
    bool showHud(){return hudFlag;};

    short getTransparencyLevel() {return transparencyLevel;}

    /*---Interaction facilitation---*/
    virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);
    virtual void PoseContinuouslyChanging(PosableStateBase *whichPose, GLboolean isChanging = GL_TRUE);

        virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);
    virtual void renderPickableObject(Window3D *inWindow);
    void setRespondToPress2(unsigned char value) {respondToPress2 = value; if (value == GL_FALSE) ignore(PICK_PRESS2);} //send in non-GL_TRUE/GL_FALSE value to force it to stay GL_FALSE

    static unsigned int objectClickedEvent();
    static unsigned int objectSelectedEvent();
    static unsigned int objectPoseChangedEvent();
    static unsigned int objectStabilizedEvent();

    void postContextMenu(InputInfo *inputInfo);
    virtual void PostMenu(int id);

    GLboolean isSelected() {return selectedFlag;}
    static int tellSelectedObject(int argc, const char **argv, Tcl_Interp *interp);
    static Object *getSelectedObject() {return selectedObject;}
    static void setSelectedObject(Object *newSelection, GLboolean selectForMenu = GL_FALSE);
    virtual PosableStateBase *getSelectedPose() {return NULL;}

    static Object *getFrameOfReference() {return frameOfReference;}
    static void setFrameOfReference(Object *newSelection);

    GLboolean isUsingInternalHandle() {return usingInternalHandleFlag;}
    void useInternalHandle(GLboolean val = GL_TRUE) {usingInternalHandleFlag = val;}
    bool getAllowBaseExternalEdit() {return allowBaseExternalEdit;}

    //Scripting
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);

    //TextureMap stuff
    static void SetDefaultTexture(std::shared_ptr<std::shared_ptr<TextureMap> > &ptr);
    virtual void SetCustomTexture(std::shared_ptr<TextureMap> &ptr);
    virtual void SetCustomTexture(int oid, std::shared_ptr<TextureMap> &t);
    void UpdateCustomTexture();
    /** Disables the use of textures, even if EnableTexture is called in the rendering loop.  */
    virtual void DontUseTextures();
    /** If rendering uses textures, this will enable them. */
    virtual void UseTextures();

    void UpdateTexture();
    std::shared_ptr<TextureMap> &GetCustomTexture();
    std::shared_ptr<TextureMap> &GetCurrentTexture();

    //Tide & NavigationPath
    static void SetTidePtr(TimeVaryingObject ** t);
    static void SetNavPathPtr(NavigationPath ** n);

    //Querying
    static const float invalidHeight;
    virtual double GetHeightAt(double x, double y, Point3d coord = NULL, GLboolean secondary = GL_FALSE);
    enum {height_max, height_min, height_average};
    virtual double GetHeightNear(double x, double y, float rangeMeters = 1.0, int heightType = height_max);

    //attribute stuff
    virtual int getAttribute(SmartValue &val, int attributeIndex);
    virtual int setAttribute(SmartValue &val, int attributeIndex);

    //Garbage collection
    virtual void Unload() {garbageFlag = GL_TRUE;} //added by mdp 9/19/2000
    int IsGarbage() {return garbageFlag;} //added by mdp 9/14/2000

    /// Return string of data to display on screen Head's up display.
    virtual std::string GetHUDString();

    virtual void HandlePacket(std::shared_ptr<gz4d::Socket::Packet> &data);
protected:
    //Bounding Box
    BBox bbox;

    //Position & orientation
    int calculateCOG;

    //Visibility
    int visible;
    struct VisibilityRanges {
        VisibilityRanges() {useHeading = useScale = useDistance = false; commonRange = NULL; destroyCommonRange = false;}
        float minHeading, maxHeading;
        float minScale, maxScale;
        float minDistance, maxDistance;
        bool useHeading;
        bool useScale;
        bool useDistance;
        double *commonRange;
        bool destroyCommonRange;
    } *visibilityRanges;
    static double *commonRange;
    static int rangesLeft;

    //TimeRelated
    double lastIdleTime;// default idle function will set this
    double qstart, qend, qlength; // the start and end of the current time 'quantum'
    // it is meant to be used to know when the time has changed enough for
    // the current object to worry about it. Ex: Current flow data, sampled
    // once an hour doesn't need to lookup the which values to display every
    // second. If qstart and qend are calculated to 13:00 and 14:00, then as
    // long as the time is still withing these bounds, don't worry about
    // displaying updated data. Doesn't make sense? Ask rja. Also see 
    // Vector.cpp's Idle function.
    bool hideOutsideTimeBounds;
    int outsideTimeBounds;
    float preShowTime, postShowTime;
    int interpolatePositions;

    //File-related
    virtual void registerLabel();
    virtual void initializeAs(std::string alias) {};
    virtual int processField(int option, char **Field);
    int published; // -1 don't publish, 0 need publishing, 1 published
    
    ///\name Object Connection
    ///@{
    gz4d::SocketReceiver::Ptr socketThread;
    gz4d::ThreadedQueue<std::shared_ptr<gz4d::Socket::Packet> >::Ptr socketQueue;
    ///@}
    
    GLboolean centerGiven;

    GLboolean editedFlag;   //has this object been edited (and require saving)?

    //gz4d::geo::Projection proj; //what projection are we using?
    gz4d::ogr::SpatialReference spatialReference;
    GLboolean projectionRequired;
    GLboolean reversedCoordinates; //true if REVERSED_XY is specified.
    int projectXY(int nVals, double *xVals, double *yVals, GLboolean completesProjection = GL_TRUE);
    int projectXY(std::vector<gz4d::Point<double> > &points, GLboolean completesProjection = GL_TRUE);
    int autoProjection(double longitude, double latitude);
    bool setUTMProjection(int zone, bool north);

    //Indicators of use by other objects
    bool referable; // A referable object does not draw itself, it gets drawn from objects referencing it
                   // example usage is displaying many copies of a dxf object without loading multiple
                   // copies, just use references instead
    int timeIndex; //used only for loadXML with NetCDF argument right now (mdp 2/12/2004)

    //Object Hierarchy
    char * treePath; // path used by the object tree widget
    virtual void initializingLabel() {}; //allows for initialization of names for components "under" this object in the Tcl namespace
    ObjectCollection * parent;
    PosableStateLock * lockToParent;
    bool requiresFullReload;

    //Rendering
    bool showAsWireframe;
    int degradationFlag; ///< do we degrade, if so, how?
    int doneFlag;    ///< done with high res rendering
    short transparencyLevel;
    GLboolean renderingForPicking; //are we rendering for picking (or to the screen)?
    static GLfloat lastHeightExaggeration;
    static Point3f palette[12];  // the color of each point;
    float scalePriority; //for use in blowing objects up larger-than-life (logical fisheye)
        
    //Interaction facilitation
    GLboolean selectedFlag; //is this object currently selected?
    bool isEditableFlag;   ///< is this object editable? (rja, 2006-10-26, I think this really means Do I need to keep track if I was edited so I can pop up a confirmation on exit if not saved.
    GLboolean usingInternalHandleFlag;  //is this object already using a handle?
    GLboolean canUseInternalHandleFlag; //does this object want to draw internal handles?
    bool allowBaseExternalEdit; ///< can this object's main posable be moved around by the point handles?
    GLboolean respondToPress; //does this object respond to button presses the same way as clicks?
    unsigned char respondToPress2; //does this object allow attaching through the second mouse button?
    GLboolean respondToClick; //does this object respond to clicks at all?

    static const EventType *objectClickedInputType;
    static const EventType *objectSelectedInputType;
    static const EventType *objectPoseChangedInputType;
    static const EventType *objectStabilizedInputType;

    //TextureMap stuff
    void EnableTexture(int mode = TEXTURE_COORD_USER);
    void DisableTexture();
    virtual void UpdateTexture(std::shared_ptr<TextureMap> &ptr) {};
    //const Point4ub *GetColorAtValue(float val);
    gz4d::Pixel<gz4d::RGBA8> GetColorAtValue(float val) const;

    static std::shared_ptr<std::shared_ptr<TextureMap> > defaulttextureptr;  // Global lookup table pointer
    std::shared_ptr<TextureMap>  customtextureptr;
    int useTextures; /** Allows an object to optionally use textures */
    int clutMode;
    int customtextureid;

    //Tide & NavigationPath
    static TimeVaryingObject ** tide;
    static NavigationPath ** navPath;

    //attribute stuff
    enum ObjectAttributes {Attr_wireframe, Attr_visible, Attr_clutMode, Attr_useTextures,
                         Attr_isEditable, Attr_respondToPress, Attr_respondToPress2,
                         Attr_degradation, Attr_hud,
                         NumObjectAttributes};

    std::shared_ptr<ObjectHistory> poseHistory; // allows for any object to hook into the time mechanism

    //Rendering
    Window3D *onlyWin;  //allow an object to only appear in one window

private:
    //Position & orientation
    float cog,sog;  // calculated for varying pose

    //Class determination
    static ObjectRegistrationEntry **objectRegistry;
    static int numEntries, maxEntries;
    static Object *selectedObject;
    static Object *frameOfReference;
    static int settingOffset;

    bool spheroidFlag;// spheroid - conceived from the idea of displaying a "sphere of influence" of sound from vessels traveling around whales, but I'm sure we can think of other uses!
    float spheroid_x,spheroid_y,spheroid_z; //meters
    Point3f spheroid_color;
    disc * acousticSignature;
    
    bool hudFlag;

    //TextureMap stuff
    std::string pendingCustomTexture;

    //Garbage collection
    int garbageFlag;  //is it time to clean us up?
    
    std::queue<bfs::path> fileQueue;
    std::shared_ptr<std::ifstream> currentFile;
};

extern DottedAddress *ObjectNeighborhood;

#define OBJECT_NAMES \
    "PROJECT", "SPATIAL_REFERENCE", "LOCAL_ORIGIN", "REVERSED_XY", \
    "DEGRADATION", "ON", "OFF"

#define OBJECT_ATTRIBUTES \
    "wireframe", "visible", "clutMode", "useTextures", "isEditable", "respondToPress", \
    "respondToPress2", "degradation", "hud"
#endif

#define NUM_COLOR_CODES Object::N_COLOR_CODES
