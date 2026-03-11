// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// Scene3D.h

///////////////////////////////////////////////////////////////////////////////

// Scene3D.h: interface for the Scene3D class.
//  
//////////////////////////////////////////////////////////////////////

#ifndef SCENE3D_H
#define SCENE3D_H

#include "SDV_types.h"

#include "engine/Interactor.h"
#include <gz4d/socket/TCPSocket.h>
#include <gz4d/socket/UDPSocket.h>
#include <filesystem>
#include <gz4d/ogr.h>
#include <memory>
//#include <gz4d/util/Xml.h>
#include <gz4d/netcdf.h>
#include <list>

#include <boost/property_tree/ptree.hpp>

namespace bpt = boost::property_tree;


class Object;
class ObjectCollection;
class Gutm;
class Window3D;
class TimeVaryingObject;
class RTVideo;
class TextureMap;
class TimeControl;
class NavigationPath;
class Cache;

//class TimeCircle;

/**
 *	This class manages a 3D scene of objects.
 *	It has the following responsibilities:
 *	\li Manage a collection of objects (add, delete, iterate)
 *	\li Provide operations to display and reset all objects
 *	\li Calculate and utilize degradation information
 *	Derived from Interactor
 *  \author Matt Plumlee (mdp)
 *  \date 8/4/2000 (for degradation calc)
 */
class Scene3D : public Interactor
{
protected:
//	struct Node;
	struct SubWinReqEntry
	{
		Window3D * parentWindow;
		Object * creator;
		SubWinReqEntry * next;
	};

public:
    Scene3D(int &argc, char *argv[]);
    virtual ~Scene3D();

    // Texture stuff
    void UseDefaultTexture(int id);
    void UseCustomTexture(int oid, int cid);
    void SaveTexture(std::string const &fn, int id);
    void SaveTexture(std::string const &fn, char *id);
    void SetGlobalTexture(int i);
    void SetGlobalTexture(char * id);
    void InitTexture(Tcl_Interp *interp);
    void DisplayTexture(Tcl_Interp *interp, int id, float min , float max, char * target);
    std::shared_ptr<TextureMap> &GetTexture(int id);
    int GetTextureCount();

    std::shared_ptr<TextureMap> &GetRTVideo(){return rtvideo;};
    
    std::shared_ptr<TextureMap> &GetTexturePtr();
    std::shared_ptr<TextureMap> &GetTexturePtr(std::string label);
    std::shared_ptr<TextureMap> &GetTexturePtr(int i);
    std::shared_ptr<std::shared_ptr<TextureMap> > &GetDefaultTexturePtrPtr();
    int TextureNext();
    int AddTexture(std::shared_ptr<TextureMap> &texture);
    int DeleteTexture(int id);
    std::shared_ptr<TextureMap> LoadTexture(std::string const &filename, Tcl_Interp *interp);
    void ReloadTexture(int id);

    // visibility stuff
    void SetVisible(int id);
    void SetInvisible(int id);

    bool AreWidgetsVisible(){return WidgetsVisibleFlag;};
    bool * GetWidgetsVisibleFlag(){return &WidgetsVisibleFlag;};
    void SetWidgetsVisible(bool v=true);

    void VisChange();

    // object stuff
    Object * GetObjectPtr();
    Object * GetObjectPtr(int id);
    Object * GetObjectPtr(std::string const &l);
    Object * Next();
    int Reset();
    int ResetIterator();
    int DrawAllHighRes(int eye, Window3D *win); //mdp 3/7/2000 -- changed return value so we know if anything actually rendered
    void DrawAllLowRes(Window3D *win);
    void DrawAllOrtho();
    void CallObjectIdle();
    void ResetAllHighRes();
    int AddObject(Object* object, Tcl_Interp *interp);
    Object * LoadObject(std::string const & fn, Tcl_Interp *interp, char * l = NULL, bool broadcast = false, ObjectCollection *parentCollection = NULL);
    Object * NewObject(std::string type, Tcl_Interp *interp, std::string l, double x = 0, double y = 0, double z = 0, int posSelfInit = 0); // posSelfInit flag indicates if object self initializes its position, so we don't override it. rja Oct 26, 2005
    Object * NewObject(std::string type);
    void PostObjectMenu(int id);
    ObjectCollection *ensureObjectCollectionPath(std::string path);
    
    bool LoadXML(std::string const & filename, Tcl_Interp *interp, ObjectCollection *parentCollection = NULL);
    void LoadXMLTexture(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    void LoadXMLAnimator(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    bool LoadXMLScript(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    void ProcessGZ(bpt::ptree const &xml);
    
    int ReloadObject(Object* object);
    void Fuse(Object *o = NULL);
    void setHeightExaggeration(GLfloat heightEx);

    std::string GetExtension(int id);
    void SaveObject(int id, char * fn);
    void SaveEditedObjects();

    std::string const &GetObjectTypes();
    std::string const &GetObjectExtensions();

    //Object info stuff
    void resetInfo();
    void appendInfo(std::string info, bool addNewLine = false);
    short getMaxTransparencyLevel() {return maxTransparencyLevel;}
    int setMaxTransparencyLevel(short level);

    // Scene building stuff
    void BuildScene(Tcl_Interp *interp, std::string scenepath);
    void SaveScene(Tcl_Interp *interp, std::string scenefn, std::string const &path);
    static void objectEdited(GLboolean val = GL_TRUE);

    bool setSpatialReference(gz4d::ogr::SpatialReference newSR, bool override=false);
    gz4d::ogr::SpatialReference getSpatialReference() {return currentSpatialReference;}
    std::string const &getCurrentProjectionName() {return proj_name;}

    // Mutex on High-res drawing, control over continuous low-res
    GLboolean ForceLockHighRes(void *);
    GLboolean AssertLockHighRes(void *);
    GLboolean UnlockHighRes(void *);
    void *whoHasLockHighRes();
    static void setContinuousChange(unsigned char changing);
    static GLboolean isInContinuousChange() {return (continuouslyChanging > 0);}
    int RegisterWithTcl(Window3D *throughWin);

    // Input management
    virtual int HandleInput(unsigned int inputType, InputInfo *info);
    static unsigned int addObjectEvent() { return addObjectInputType->value;}
    static unsigned int sceneChangedEvent() { return sceneChangedInputType->value;}
    static unsigned int sceneResetEvent() { return sceneResetInputType->value;}
    static unsigned int objectChangedEvent() { return objectChangedInputType->value;}
    static long timeForLastFrame();

    //sub window stuff
    void requestNewSubwindow(Object * creator, Window3D * parentWindow);
    void createStackedSubwindows();

    void setGlobalWindowContext(Window3D *win) {globalWindowContext = win;}

    // time stuff
    static TimeControl * timeControl;
    // static TimeCircle *timeCircle;
    void setGZTime(double time);
    double timeBufferAmount(double amount = -1.0);
    double timeBufferFactor(double percent = -1.0);
    GLboolean timeBufferFront(int val = -1);
    GLboolean timeBufferBack(int val = -1);
    
    //caching stuff
    static Cache *imageCache;

    void UpdateBBox();

    static double interactivityHint;
    
    //Tide
    void SetTide(TimeVaryingObject * t);
    void SetNavPath(NavigationPath * n);

    //Master slave stuff
    void InitSockets();
    void CheckSockets();

    double GetHeightAt(double x, double y, Point3d coord = NULL, GLboolean secondary = GL_FALSE);

    void PostAddObject(Object *ob);

protected:

    std::string proj_name;

    std::shared_ptr<ObjectCollection> objects;
    short maxTransparencyLevel;

    std::string objectTypeString, objectExtensionString;
    int numObjectsLastTime;
    void updateObjectStrings();

    std::shared_ptr<TextureMap> currenttexture;

    static int numEditedObjects;

    // color lookup table list

    std::map<int, std::shared_ptr<TextureMap> > texturesByID;
    std::map<std::string, std::shared_ptr<TextureMap> > texturesByLabel;
    std::shared_ptr< std::shared_ptr<TextureMap> > global_texture;

    std::shared_ptr<TextureMap> rtvideo;

    gz4d::ogr::SpatialReference currentSpatialReference;

    // used for degradation calcs
    static int frametime;
    float degradation;
    int lastdrawhighres;
    GLint drewLowRes, drewHighRes;

    // time bounding box buffer preferences
    double _timeBufferFactor, _timeBufferAmount;
    GLboolean _timeBufferFront, _timeBufferBack;

#ifdef TIME_HIGHRES
    // timing 
    clock_t starttime;
    clock_t endtime;
#endif

    // widgets
    bool WidgetsVisibleFlag;

    // mutex, continuous change
    void *PointerToMutexHolder;
    static int continuouslyChanging;

    // interaction
    static const EventType *addObjectInputType;
    static const EventType *sceneChangedInputType;
    static const EventType *sceneResetInputType;
    static const EventType *objectChangedInputType;

    // subwindows stuff
    SubWinReqEntry * subwinstack;
    
    /// Pointer to tides if they are loaded, NULL if no tides present.
    TimeVaryingObject * tide;

    NavigationPath * navPath;

    std::shared_ptr<gz4d::TCPSocket> ioSocket;
    typedef std::list<gz4d::Socket> SocketList;
    SocketList ioConnections;

    gz4d::UDPSocket slaveSocket;
    gz4d::UDPSocket masterSocket;

    struct State
    {
        char s; ///< to distinguish from load object message.
        double x;
        double y;
        double z;
        double heading;
        double pitch;
        double roll;
        double scale;
        double heightExaggeration;
        double timeScale;
        double timeOffset;
        bool operator==(State const &o){return (x==o.x&&y==o.y&&z==o.z&&heading==o.heading&&pitch==o.pitch&&roll==o.roll&&scale==o.scale&&heightExaggeration==o.heightExaggeration&&timeScale==o.timeScale&&timeOffset==o.timeOffset);}
    };
    State lastState;
    //double oldx,oldy,oldz,oldh,oldp,oldr,olds,oldt,oldts;

    //Support for illustrating subwindow rendering
    Window3D *globalWindowContext;
};

#endif
