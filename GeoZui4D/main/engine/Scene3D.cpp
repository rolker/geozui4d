/* Copyright (c) 2000 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
//////////////////////////////////////////////////////////////////////
// Scene3D.cpp: implementation of the Scene3D class.
//
//////////////////////////////////////////////////////////////////////


#include "objects/Kinect.h"
#include <gz4d/TimeSequence.h>
#include "objects/Sound.h"
#include "objects/MissionPlan/MissionPlan.h"

#include "MainWindow.h"
#include "engine/Window3D.h"
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include "objects/SwathStream.h"
#include "engine/PosableStateBase.h"
#include "objects/XYZStream.h"
#include "objects/gutm.h"
#include "engine/Scene3D.h"
#include "SubWindow.h"
#include "engine/Object.h"
#include "engine/SessionRecorder.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "widgets/Sv_Standards.h"
#include "utilities/DottedAddress.h"
#include "objects/Vector.h"
#include "interactors/Animator.h"
//#include "MovieScreen.h"
//#include "objects/arm.h"
#include "engine/ObjectCollection.h"
#include "objects/NetCDF.h"
#include "objects/Flow.h"
#include "interactors/PointHandle.h"
#include "objects/TimeVaryingObject.h"
#include <boost/filesystem.hpp>
#include "objects/Chart.h"
//#include "TVGutm.h"
#include "objects/Current.h"
#include "objects/CurrentLayer.h"
#include "objects/Floater.h"
#include "objects/S57/S57.h"
#include <gz4d/socket/TCPSocket.h>
#include "objects/dxf/DXF.h"
#include "objects/Text.h"
#include "objects/DynamicTides.h"
#include "objects/BasicVessel.h"
#include "objects/Srtm.h"
#include "engine/TimeControl.h"
//#include "TimeCircle.h"


#include "objects/SoundOgg.h"
#include "objects/Sonar/EK500.h"
#include "engine/Notes.h"

#include "engine/TextureMaps/TextureMap.h"
#include "engine/TextureMaps/Clut.h"
#include "engine/TextureMaps/RTVideo.h"
#include "engine/ViewManager.h"
#include <gz4d/X3D.h>
#include "Wiimote.h"
#include "Spaceball.h"
#include <gz4d/geo/Projection.h>
#include "utilities/Cache.h"

using namespace std;

//#ifdef WIN32
//LINK_OBJECT(arm);
LINK_OBJECT(AveragingOverview);
LINK_OBJECT(BasicVessel);
LINK_OBJECT(Boundary);
LINK_OBJECT(Caption);
LINK_OBJECT(Chart);
LINK_OBJECT(CoordinateGrid);
LINK_OBJECT(Current);
LINK_OBJECT(CurrentLayer);
LINK_OBJECT(Curtain);
LINK_OBJECT(DataLines);
LINK_OBJECT(DataPoints);
LINK_OBJECT(DataTriangles);
//LINK_OBJECT(Dtm);
LINK_OBJECT(DXF);
LINK_OBJECT(DynamicTides);
LINK_OBJECT(EK500);
LINK_OBJECT(Floater);
LINK_OBJECT(Flow);
LINK_OBJECT(Gutm);
LINK_OBJECT(Hugin);
LINK_OBJECT(Label);
//LINK_OBJECT(MissionPlan);
LINK_OBJECT(NetCDF);
LINK_OBJECT(Points);
LINK_OBJECT(ProximityHighlighter);
LINK_OBJECT(Reference);
LINK_OBJECT(Sound);
//LINK_OBJECT(SoundOgg);
LINK_OBJECT(Text);
LINK_OBJECT(TextureScreen);
LINK_OBJECT(Triangles);
LINK_OBJECT(Tube);
LINK_OBJECT(Reson);
LINK_OBJECT(S57);
LINK_OBJECT(Srtm);
LINK_OBJECT(SwathStream);
//LINK_OBJECT(Vector);
LINK_OBJECT(Vessel);
LINK_OBJECT(Vip);
LINK_OBJECT(WhalePath);
LINK_OBJECT(XYZStream);
LINK_OBJECT(FishSchool);
LINK_OBJECT(X3D);
LINK_OBJECT(Kinect);
//#endif

//#include "BilinearInterpolation.h"
//BilinearInterpolation *bilinearInterpolation = new BilinearInterpolation;

extern MainWindow *mainWindow;
extern PointHandle *pointHandle;


#ifndef WIN32
//extern Wiimote wiimote;
extern Spaceball spaceball;
#endif

const EventType *Scene3D::addObjectInputType = Interactor::newEventType("addObject");
const EventType *Scene3D::sceneChangedInputType = Interactor::newEventType("sceneChanged");
const EventType *Scene3D::sceneResetInputType = Interactor::newEventType("sceneReset");
const EventType *Scene3D::objectChangedInputType = Interactor::newEventType("objectChanged");

#define MAX_TOOL_COMMAND 1024

TimeControl * Scene3D::timeControl;
Cache *Scene3D::imageCache;
//TimeCircle *Scene3D::timeCircle;
int Scene3D::frametime = 0;
double Scene3D::interactivityHint = 0.5;
int Scene3D::continuouslyChanging = 0;
int Scene3D::numEditedObjects = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------*/
Scene3D::Scene3D(int &argc, char *argv[])
:Interactor("gz"), objects(new ObjectCollection(GL_TRUE, 0)) //instantiate Objects without it being pickable
{

  numObjectsLastTime = 0;

  frametime = 0;
  degradation = 1.0;
  _timeBufferFactor = _timeBufferAmount = 0.0;
  _timeBufferFront = _timeBufferBack = GL_TRUE;
    
  WidgetsVisibleFlag = true;
  PointerToMutexHolder = GL_FALSE;

  subwinstack = NULL;

  objects->SetTreePath("");
  objects->ignore(Pickable::PICK_CLICK1);
  objects->ignore(Pickable::PICK_PRESS2);
  objects->ignore(Pickable::PICK_PRESS3);
  objects->SetLabel("Object");
  ObjectNeighborhood = objects->getAddress();
  PosableNeighborhood->addChild(ObjectNeighborhood);

  tide = NULL;
  Object::SetTidePtr(&tide);

  navPath = NULL;
  Object::SetNavPathPtr(&navPath);

  //oldx = oldy = oldz = oldh = oldp = oldr = olds = oldt = oldts = 0.0;
  memset(&lastState,0,sizeof(lastState));
  maxTransparencyLevel = 1;  
  
  globalWindowContext = NULL;

  proj_name = "";
}

/*---------------------------------------------------------------------------*/
Scene3D::~Scene3D()
{
    ioSocket->Close();
    for(SocketList::iterator c = ioConnections.begin(); c != ioConnections.end(); ++c)
        c->Close();
}

/*---------------------------------------------------------------------------*/
int Scene3D::AddObject(Object * object, Tcl_Interp *interp)
{
    double scale;
    int wasEmpty = objects->IsEmpty();

    if (!object)
        return -1;

    if (!currentSpatialReference && object->getSpatialReference())
        setSpatialReference(object->getSpatialReference());

    int ret = objects->AddObject(object);
    PostAddObject(object);
    if (wasEmpty)
    {
        PosableStateBase *pose = mainWindow->getPoseInfo();
        gz4d::Point<double> pos = pose->getPosition();
        if (pose->getScale() != 1.0 || pos[X] != 0.0 || pos[Y] != 0.0 || pos[Z] != 0)
            return ret;
        scale = object->getPreferredScale();
        if (scale)
        {
            pose->setScale(object->getPreferredScale());
            gz4d::Point<double> offset = object->getPreferredOffset();
            if (offset.IsValid())
                pose->setPosition(offset);
            pose->setOrientation(20, 0, ChangeRotatePitch);
        }
    }

    return ret;
}

/*---------------------------------------------------------------------------*/
/** This function ensures that there is a path of ObjectCollections to the
 *  passed path, including the last name in the dotted address.  If any
 *  item exists and is not an ObjectCollection, returns NULL.  Otherwise,
 *  a path is created or confirmed such that it.
 */
ObjectCollection *Scene3D::ensureObjectCollectionPath(string path)
{
    ObjectCollection *parentOC, *childOC;
    Addressable *adr;
    
    string childPath;
    
    string::size_type cursor = path.rfind("."); //look through path for .'s
    
    if (cursor != string::npos) //recursive step
    {
        childPath = path.substr(cursor+1);
        parentOC = ensureObjectCollectionPath(path.substr(0,cursor));
    }
    else //base of recursion
    {
        parentOC = objects.get();
        childPath = path;
    }

    if (!parentOC)
        return NULL;

    adr = parentOC->getAddress()->findItem(childPath);
    if (!adr)
    {
        childOC = new ObjectCollection;
        childOC->SetLabel(childPath);
        parentOC->AddObject(childOC);
        return childOC;
    }
    DYNAMIC_CAST(childOC, ObjectCollection *, adr);
    if (!childOC) //exists, but is not an object collection!
        return NULL;

    return childOC;
}

/*--------------------------------------------------------------------------*/
/** This function reloads an object by deleting it and replacing it, such
 *  that it keeps the same position on the object list.  If NULL is passed
 *  for object, all objects are reloaded.  The reloaded object is actually
 *  loaded before the old object is deleted, in case there are any problems
 *  loading the new object.
 */
int Scene3D::ReloadObject(Object* object)
{
    return objects->ReloadObject(object);
}

/*---------------------------------------------------------------------------*/
void Scene3D::setGZTime(double time)
{
    timeControl->setGZTime(time);
}

/*---------------------------------------------------------------------------*/
double Scene3D::timeBufferAmount(double amount)
{
    if (amount >= 0)
    {
        _timeBufferAmount = amount;
        _timeBufferFactor = 0.0;
    }
    return _timeBufferAmount;
}

/*---------------------------------------------------------------------------*/
double Scene3D::timeBufferFactor(double percent)
{
    if (percent >= 0)
    {
        _timeBufferFactor = percent;
        _timeBufferAmount = 0.0;
    }
    return _timeBufferFactor;
}

/*---------------------------------------------------------------------------*/
GLboolean Scene3D::timeBufferFront(int val)
{
    if (val >= 0)
        _timeBufferFront = val;
    return _timeBufferFront;
}

/*---------------------------------------------------------------------------*/
GLboolean Scene3D::timeBufferBack(int val)
{
    if (val >= 0)
        _timeBufferBack = val;
    return _timeBufferBack;
}

/*---------------------------------------------------------------------------*/
int Scene3D::AddTexture(std::shared_ptr<TextureMap> &texture)
{
  if (!texture)
      return 1;
  string label = texture->GetLabel();

  // make sure texture with same file name not already in list
  // if it is, reject this add request
  if(texturesByLabel.count(label))
      return 1;

  texturesByLabel[label] = texture;
  texturesByID[texture->GetId()] = texture;

  stringstream tclstr;
  tclstr << "addtexture {" << texture->GetId() << " - " << texture->GetLabel() << "}";
  mainWindow->Tcl_Eval(tclstr.str().c_str());

  return 0;
}

/*---------------------------------------------------------------------------*/
int Scene3D::DeleteTexture(int id)
{
    if(!texturesByID.count(id))
        return 1;

    texturesByLabel.erase(texturesByID[id]->GetLabel());
    texturesByID.erase(id);

    return 0;
}

/*---------------------------------------------------------------------------*/
int Scene3D::DrawAllHighRes(int eye, Window3D *win)
{
    int doneFlag = 1; //gets unset if any objects are not done.

    
#ifdef TIME_HIGHRES
    if (lastdrawhighres == 0)
    {
        starttime = clock();
        endtime = 0;
    }
    lastdrawhighres = 1;
#endif

    glPushMatrix();
    doneFlag =  objects->DrawHighRes(eye,win);
    glPopMatrix();

#ifdef TIME_HIGHRES
    if (doneFlag == 1 && endtime == 0)
    {
        endtime = clock();
        gzlog << "Hires: Frame time: " << endtime - starttime << endl;
    }
#endif
    
    drewHighRes = GL_TRUE;

    return doneFlag;
}


void Scene3D::DrawAllOrtho()
{
    objects->DrawOrtho();
}

/*---------------------------------------------------------------------------*/
/// Draws the scene in low resolution starting from the root ObjectCollection.
/// \callgraph
void Scene3D::DrawAllLowRes(Window3D *win)
{
    // take care of the degradation
    if (degradation > 1.0)
    {
        degradation = 1.0;
    }

    if (degradation < 0.001)
    {
        degradation = float(0.001);
    }
    if (globalWindowContext)
        win = globalWindowContext;

    glPushMatrix();
    if(win->isRecording())
        objects->DrawLowRes(0.0,win);
    else
        objects->DrawLowRes(degradation,win);
    glPopMatrix();
    lastdrawhighres=0;
    drewLowRes = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
int Scene3D::setMaxTransparencyLevel(short level)
{
    if (level >= 0)
    {
        maxTransparencyLevel = level;
        globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
        return TCL_OK;
    }
    return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
void Scene3D::CallObjectIdle()
{

    CheckSockets();
    
    for(std::map<int,std::shared_ptr<TextureMap> >::iterator t = texturesByID.begin(); t != texturesByID.end(); ++t)
    {
        if(t->second && t->second->TextureIdle(timeControl->getGZTime())!= 0)
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }
    
    objects->GetBoundingBox();//force a recalc if needed
    objects->Idle(timeControl->getGZTime());
}

/*--------------------------------------------------------------------------*/
/** This function resets all objects to start over drawing high res the next
 *  time DrawHighRes is called.
 */
void Scene3D::ResetAllHighRes()
{
    objects->ResetHighRes();
}

/*---------------------------------------------------------------------------*/
/** Resets the iterator for GetObjectPtr and Next calls.
 */
int Scene3D::ResetIterator()
{
    return objects->Reset();
}

/*---------------------------------------------------------------------------*/
/** Resets the first_object_flag if the objects collection is indeed empty,
 *  and generates an appropriate event.  If the objects collection is not
 *  empty, returns false.
 */
int Scene3D::Reset()
{
    if (objects->Reset())
    {
        PosableStateBase *pose;
        //reset the main view.
        mainWindow->SetScale(1.0);
        pose = mainWindow->getPoseInfo();
        pose->setPosition();
        pose->setOrientation(0.0, 0.0);
        pose->setPoseInfoFromTo(ChangePose, ChangeHomePose);
        globalInputMediator->Trigger(Scene3D::sceneResetEvent());
    }
    return objects->IsEmpty();
}

/*---------------------------------------------------------------------------*/
Object *Scene3D::Next()
{
    return objects->Next();
}

/*---------------------------------------------------------------------------*/
Object * Scene3D::GetObjectPtr()
{
    return objects->GetObjectPtr();
}

/*---------------------------------------------------------------------------*/
Object * Scene3D::GetObjectPtr(int id)
{
    return objects->GetObjectPtr(id);
}

/*---------------------------------------------------------------------------*/
Object * Scene3D::GetObjectPtr(const string& l)
{
    return objects->GetObjectPtr(l);
}


/*---------------------------------------------------------------------------*/
std::shared_ptr<TextureMap> &Scene3D::GetTexturePtr()
{
    return currenttexture;
}


/*---------------------------------------------------------------------------*/
void Scene3D:: Fuse(Object *o) // this merges gutm objects
  // the high resolution ones drill holes in the low resolution ones
{
    objects->Fuse(o);
}

/*---------------------------------------------------------------------------*/
void Scene3D::setHeightExaggeration(GLfloat heightEx)
{
    objects->setHeightExaggeration(heightEx);
}

/*---------------------------------------------------------------------------*/
std::string const &Scene3D::GetObjectTypes()
{
  updateObjectStrings();
  return objectTypeString;
}

/*---------------------------------------------------------------------------*/
std::string const &Scene3D::GetObjectExtensions()
{
  updateObjectStrings();
  return objectExtensionString;
}

/*---------------------------------------------------------------------------*/
void Scene3D::updateObjectStrings()
{
    ObjectRegistrationEntry **registry;
    
    if (numObjectsLastTime == Object::getObjectRegistrySize())
        return; //no changes since last time!

    //clean up old idea of strings
    objectTypeString.clear();
    objectExtensionString.clear();

    registry = Object::getObjectRegistry();

    numObjectsLastTime = Object::getObjectRegistrySize();

    for (int i = 0; i < numObjectsLastTime; i++)
    {
        if (registry[i]->aliasFor.empty()) //don't add aliases
        {
            if (!objectTypeString.empty())
            {
                objectTypeString += " ";
                objectExtensionString += " ";
            }
            objectTypeString += registry[i]->className;
            if(registry[i]->extension.empty())
                objectExtensionString += "{}";
            else
                objectExtensionString += registry[i]->extension;
        }
    }
}

/*---------------------------------------------------------------------------*/
void Scene3D::resetInfo()
{
    char tclstr[20];

    sprintf(tclstr, "clearObjectInfo");
    mainWindow->Tcl_Eval(tclstr);
}

/*---------------------------------------------------------------------------*/
void Scene3D::appendInfo(string info, bool addNewLine)
{
    stringstream tclstr;

    tclstr << "appendObjectInfo \"" << info;
    if (addNewLine)
        tclstr << "\n";
    tclstr <<"\"";
    mainWindow->Tcl_Eval(tclstr.str());
}

/*---------------------------------------------------------------------------*/
Object * Scene3D::LoadObject(std::string const &filename, Tcl_Interp *interp, char * l, bool broadcast, ObjectCollection *parentCollection)
{
    Object * object;
    std::shared_ptr<TextureMap> texture;
    string ext;
    int ret;

    std::cerr << filename << endl;

    if (broadcast && mainWindow->getMasterPort())
    {
        gz4d::IPSocket::Address a(mainWindow->getMasterAddress(),mainWindow->getMasterPort());
        masterSocket.SendTo(filename,a);
    }

    object = NULL;

    ext = extension(bfs::path(filename));

    if (!ext.empty()) //Load the appropriate object based on extension
    {
        if (ext == ".gzn" ||  ext == ".gzx")
            if (LoadXML(filename,interp, parentCollection))
                return NULL;

        if(ext == ".gznotes")
        {
            mainWindow->GetNotes()->Load(filename);
            return NULL;
        }

        if ((texture = LoadTexture(filename,interp)))  // Special case: texture
        {
            AddTexture(texture);
            return NULL;
        }
        else
            object = Object::createFromExtension(ext);
    }

    if (object)
    {
        ret = object->SetFilename(filename);
        if (ret==0)
        {
            if (object->Load() == 0)
            {
                if (l)
                    object->SetLabel(l);
            }
            else
            {
                delete object;
                object = NULL;
            }
        }
        else
        {
            delete object;
            object = NULL;
        }
  }
    else
  {
        std::cerr << "No object loaded!" << endl;
  }
    return object;
}

/*---------------------------------------------------------------------------*/
bool Scene3D::LoadXML(std::string const & filename, Tcl_Interp *interp, ObjectCollection *parentCollection)
{
    GLboolean hadScripts = GL_FALSE;

    bpt::ptree doc;

    //try netcdf
    
    gz4d::netcdf::File ncfile(filename);
    if(ncfile)
    {
        gz4d::netcdf::Att a = ncfile.GetAtt("gzx");
        if(a)
        {
            std::istringstream ss(a.As<std::string>(0));
            read_xml(ss,doc);
        }

        if(doc.empty())
        {
            std::cerr << "gzx attribute not found in netCDF file: " << filename << endl;
            return false;
        }
    }
    else
    {

        // not a netcdf file, see if a gzx
        try {
            read_xml(filename,doc);
        }
        catch (std::runtime_error e) {
        }
        if(doc.empty())
        {
            std::cerr << "Not a netCDF or xml file: " << filename << endl;
            return false;
        }
    }

    //Do we have a GZ document?

    bpt::ptree n = doc.get_child("GZ");
    if(!n.empty())
    {
        if (parentCollection == NULL)
            parentCollection = objects.get();
        bool wasEmpty = ((objects.get() == parentCollection) && objects->IsEmpty());
        ProcessGZ(n);
        //n.FirstNode();
        Object * o = parentCollection->LoadXML(n,bfs::path(filename).parent_path().string(),ncfile);
        if (!o)
            return false;
        PostAddObject(o);
        if (wasEmpty && o)
        {
            PosableStateBase *pose = mainWindow->getPoseInfo();
            gz4d::Point<double> pos = pose->getPosition();
            if (pose->getScale() == 1.0 && pos[X] == 0.0 && pos[Y] == 0.0 && pos[Z] == 0.0)
            {
                if (o->getPreferredScale())
                {
                    pose->setScale(o->getPreferredScale());
                    if (o->getPreferredOffset().IsValid())
                        pose->setPosition(o->getPreferredOffset());
                    pose->setOrientation(20, 0, ChangeRotatePitch);
                }
            }
        }
    }
    else
    {
        std::cerr << "GZ root not found in xml file: " << filename << endl;
        return false;
    }
    if (hadScripts && sessionRecorder->getRecordingState() == SessionRecorder::Stopped)
        mainWindow->Tcl_Eval("after idle idleProc");
    return true;
}

/*---------------------------------------------------------------------------*/
void Scene3D::LoadXMLTexture(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    std::shared_ptr<TextureMap> texture;

    boost::optional<std::string> attribute;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.filename"))
    {
        texture = LoadTexture((bfs::path(path)/(*attribute)).string(),ToglWindow::interp);
        if (texture)
        {
            if(attribute = xml.get_optional<std::string>("<xmlattr>.label"))
              texture->SetLabel(*attribute);
            AddTexture(texture);
            if(attribute = xml.get_optional<std::string>("<xmlattr>.default"))
                if(gz4d::IsTrue(*attribute))
                    *global_texture = texture;
        }
    }
    else
        std::cerr << "XML texture support still under construction...\n";
}

/*---------------------------------------------------------------------------*/
void Scene3D::LoadXMLAnimator(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Animator *a;

    boost::optional<std::string> attribute;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.type"))
    {
        a = Animator::createFromClassname(*attribute);
        if (!a)
        {
            cerr << "Invalid Animator type " << *attribute << " in XML file.\n";
            return;
        }
        a->LoadXML(xml, path, ncfile);
    }
    else
        cerr << "No animator type given in XML Animator tag!\n";
}

bool Scene3D::LoadXMLScript(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    bool hadScripts = false;
    stringstream tclstr;

    boost::optional<std::string> attribute;
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.filename"))
    {
        std::string src = (bfs::path(path)/(*attribute)).string();
        tclstr << "set gzSourceFile {" << src << "}";
        mainWindow->Tcl_Eval(tclstr.str().c_str());
        tclstr.str("");
        tclstr << "source {" << src << "}";
        mainWindow->Tcl_Eval(tclstr.str().c_str());
        hadScripts = true;
    }

    // run embeded script
    std::string script = xml.data();
    if(!script.empty())
    {
        mainWindow->Tcl_Eval(script.c_str());
        hadScripts = true;
    }
    return hadScripts;
}

/*---------------------------------------------------------------------------*/
/** Create an object, and add it to the list
 */
Object * Scene3D::NewObject(string otype, Tcl_Interp *interp, string l, double x, double y, double z, int posSelfInit)
{
    Object * object = NULL;
    
    object = NewObject(otype);
    
    if (object != NULL)  // if an object did manage to get created, add it to the list
    {
        object->SetLabel(l);
//          object->SetDefaultTexture(&global_texture);
//          object->SetVisible(); //moved before Load by mdp 6/25/2001
        Point3d g;
        g[0] = x; g[1] = y; g[2] = z;
        if(!posSelfInit)
            object->SetPosition(g);
        AddObject(object, interp);
        globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
    }
    else
    {
        cerr << "No object created!\n";
    }
    return object;
}

/*---------------------------------------------------------------------------*/
// Create an object based on type or extension
Object * Scene3D::NewObject(string otype)
{
    Object * object;
    //BBox bbox;

    //cerr << "Scene3D::NewObject\n";
    

    object = NULL;
    
    // Is otype an extention or an object type?
    if(otype[0] == '.')
        object = Object::createFromExtension(otype);
    else
        object = Object::createFromClassname(otype);

//  char tclstr[200];
//  sprintf(tclstr,"addobj end root %i tmp",object->GetId());
//  mainWindow->Tcl_Eval(tclstr);
    
  return object;
}


/*---------------------------------------------------------------------------*/
//
// This is used to create the default texture by MainWindow and ClutEditor
// Right now this is the default Clut. The "new Clut()" command below
// should be changed if a different default is required
//
void Scene3D::InitTexture(Tcl_Interp *interp)
{
    global_texture = std::shared_ptr<std::shared_ptr<TextureMap> >(new std::shared_ptr<TextureMap>);
    *global_texture = std::shared_ptr<TextureMap>(new Clut());   // new Clut() -- will be "default.clut"
    (*global_texture)->Init();
    AddTexture(*global_texture);
    Object::SetDefaultTexture(global_texture);

    //#ifndef WIN32
#ifdef LINUX
    // Initialize Real Time Video if available
    if(!mainWindow->isRTVideoDisabled())
    {
        rtvideo = std::shared_ptr<TextureMap>(new RTVideo);
        rtvideo->Build();
        rtvideo->SetLabel("RTVideo");
        if(rtvideo->IsBuilt())
        {
            AddTexture(rtvideo);
        }
        else
        {
            rtvideo.reset();
        }
    }
    else
    {
        cerr << "RTVideo disabled\n";
    }
#endif
}

/*---------------------------------------------------------------------------*/

std::shared_ptr<TextureMap> Scene3D::LoadTexture(std::string const &filename, Tcl_Interp *interp)
{
  std::shared_ptr<TextureMap> texture;

  texture = CreateTexture(filename);

  if(!texture || texture->SetFilename(filename) || texture->Load())
      texture.reset();

  return texture;
}

/*---------------------------------------------------------------------------*/
void Scene3D::VisChange()
{
  degradation = 1.0;
  //cerr << "VisChange - deg: " << degradation << endl;
}

/*---------------------------------------------------------------------------*/
int Scene3D::GetTextureCount()
{
    return texturesByID.size();
}

/*---------------------------------------------------------------------------*/
void Scene3D::SetGlobalTexture(int i)
{
  *global_texture = texturesByID[i];

  //let the objects update their knowledge of the current default texture.
  objects->UpdateObjectsUsing(*global_texture);
}

/*---------------------------------------------------------------------------*/
void Scene3D::SetGlobalTexture(char * id)
{
    int i = atoi(id);
    if(texturesByID.count(i))
        *global_texture = texturesByID[i];
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<TextureMap> &Scene3D::GetTexturePtr(int i)
{
    std::map<int,std::shared_ptr<TextureMap> >::iterator t = texturesByID.begin();
    for (int j = 0; j<i && t != texturesByID.end(); ++j, ++t)
    {
    }
    if(t == texturesByID.end())
        return texturesByID[-1];
    return t->second;
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<TextureMap> &Scene3D::GetTexturePtr(string label)
{
    return texturesByLabel[label];
}

/*---------------------------------------------------------------------------*/
void Scene3D::SaveTexture(std::string const &fn, char *id)
{
    int i = atoi(id);
    if(texturesByID.count(i))
        texturesByID[i]->Save(fn);
}

/*---------------------------------------------------------------------------*/
void Scene3D::SaveTexture(std::string const &fn, int id)
{
    std::map<int,std::shared_ptr<TextureMap> >::iterator t = texturesByID.begin();
    for (int j=0; j<id && t != texturesByID.end(); ++j, ++t)
    {
    }
    if(t != texturesByID.end())
        t->second->Save(fn);
}

/*---------------------------------------------------------------------------*/
void Scene3D::ReloadTexture(int id)
{
    std::map<int,std::shared_ptr<TextureMap> >::iterator t = texturesByID.begin();
    for (int j=0; j<id && t != texturesByID.end(); ++j, ++t)
    {
    }
    if(t != texturesByID.end())
        t->second->Reload();
}

/*---------------------------------------------------------------------------*/
void Scene3D::SaveObject(int id, char * fn)
{
    objects->SaveObject(id,fn);
}

/*---------------------------------------------------------------------------*/
void Scene3D::SaveEditedObjects()
{

    objects->SaveEditedObjects();
}

/*---------------------------------------------------------------------------*/
void Scene3D::objectEdited(GLboolean val)
{
    if (val)
        numEditedObjects++;
    else
    {
        numEditedObjects--;
        if (numEditedObjects <0)
            numEditedObjects = 0;
    }
}


/*---------------------------------------------------------------------------*/
void Scene3D::SetVisible(int id)
{
    objects->SetVisible(id);
}

/*---------------------------------------------------------------------------*/
void Scene3D::SetWidgetsVisible(bool v)
{
    WidgetsVisibleFlag = v;
    globalInputMediator->Trigger(sceneChangedEvent());
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<TextureMap> &Scene3D::GetTexture(int id)
{
    if(texturesByID.count(id))
        return texturesByID[id];
    return texturesByID[-1];
}

/*---------------------------------------------------------------------------*/
void Scene3D::SetInvisible(int id)
{
    objects->SetInvisible(id);
}

/*---------------------------------------------------------------------------*/
string Scene3D::GetExtension(int id)
{
    return objects->GetExtension(id);
}

/*---------------------------------------------------------------------------*/
void Scene3D::UseDefaultTexture(int id)
{
    objects->SetDefaultTexture(id);
}

/*---------------------------------------------------------------------------*/
void Scene3D::UseCustomTexture(int oid, int cid)
{
    if(texturesByID.count(cid))
        objects->SetCustomTexture(oid,texturesByID[cid]);
}

/*---------------------------------------------------------------------------*/
void Scene3D::DisplayTexture(Tcl_Interp *interp, int id, float min , float max, char * target)
{
    if(texturesByID.count(id))
    {
        if(texturesByID[id]->Type() != CLUTTEXTURE)
            cerr << "Only CLUT textures can be displayed" << endl;
        else
            texturesByID[id]->Display(interp, min, max, target);
    }
}

/*---------------------------------------------------------------------------*/
void Scene3D::BuildScene(Tcl_Interp *interp, string scenepath)
{
    stringstream tclstr;

    tclstr << "puts $outfile \"<?xml version=\\\"1.0\\\"?>\n<GZ>\"";
    mainWindow->Tcl_Eval(tclstr.str().c_str());
    for (std::map<int, std::shared_ptr<TextureMap> >::iterator t = texturesByID.begin(); t != texturesByID.end(); ++t)
    {
        tclstr.str("");
        string d;
        if (t->second == *global_texture)
            d = " default=\\\"true\\\"";
        tclstr << "puts $outfile \"\t<Texture filename=\\\"" << (scenepath/ t->second->GetFilenameAsPath()).string() << "\\\" label=\\\"" << t->second->GetLabel() << "\\\"" << d << "/>\"";
        mainWindow->Tcl_Eval(tclstr.str().c_str());
    }
    
    objects->WriteToScene(scenepath);
    
    //remember where the main window was and save it.
    mainWindow->Tcl_Eval("puts $outfile \"\t<Script>gz posable set Window.main xyzhpS [gz posable get Window.main xyzhpS]\"");
    mainWindow->Tcl_Eval("puts $outfile \"\t\tgz posable set Window.main HxyzhpS [gz posable get Window.main HxyzhpS]</Script>\"");
    mainWindow->Tcl_Eval("puts $outfile \"</GZ>\"");
}

void Scene3D::SaveScene(Tcl_Interp *interp, string scenefn, std::string const &path)
{
    FILE * sf;
    
    sf = fopen(scenefn.c_str(),"w");
    if(!sf)
        return;
    fprintf(sf,"<?xml version=\"1.0\"?>\n<GZ>\n");
    
    for (std::map<int, std::shared_ptr<TextureMap> >::iterator t = texturesByID.begin(); t != texturesByID.end(); ++t)
    {
        fprintf(sf,"<Texture filename=\"%s\" label=\"%s\" ",(path/t->second->GetFilenameAsPath()).string().c_str(), t->second->GetLabel().c_str());
        if (t->second == *global_texture)
            fprintf(sf,"default=\"true\" ");
        fprintf(sf,"/>\n");
    }
    
    objects->SaveXML(sf,path);
    
    fprintf(sf,"</GZ>\n");
    fclose(sf);
}


/*--------------------------Mutex on High-res Drawing------------------------*/
/*---------------------------------------------------------------------------*/
/** Grab the high-res lock from another object.  Returns GL_TRUE if another
 *  object has the lock prior to the grab.
 */
GLboolean Scene3D::ForceLockHighRes(void *callingObject)
{
  GLboolean retval = GL_FALSE;

  if (PointerToMutexHolder != NULL &&
      PointerToMutexHolder != callingObject)
    retval = GL_TRUE;

  PointerToMutexHolder = callingObject;
  return retval;
}

/*---------------------------------------------------------------------------*/
/** Try to get the high-res lock.  Returns GL_TRUE if no one else has the
 *  lock (and grabs it), and GL_FALSE if someone does have the lock.

 */
GLboolean Scene3D::AssertLockHighRes(void *callingObject)
{
  if (PointerToMutexHolder == callingObject)
    return GL_TRUE;

  if (PointerToMutexHolder != NULL)
    return GL_FALSE;

  PointerToMutexHolder = callingObject;
  return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/** Try to release the high-res lock.  Returns GL_TRUE if the calling object
 *  had the lock (and releases it), and GL_FALSE if someone else had the lock,

 *  or if no one had the lock.
 */
GLboolean Scene3D::UnlockHighRes(void *callingObject)
{
  if (PointerToMutexHolder == callingObject)
    {
      PointerToMutexHolder = NULL;
//      if (firstNotCompletedInAll)
//              objects->ResetAllHighRes(firstNotCompletedInAll);
//      return GL_TRUE;
    }
  return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/** Return the pointer to who currently has the high-res lock.  Returns NULL
 *  if no one had the lock.
 */
void *Scene3D::whoHasLockHighRes()
{
  return PointerToMutexHolder;
}

/*---------------------------------------------------------------------------*/
/** Register any global variables with Tcl, using passed Window3D.
 */
int Scene3D::RegisterWithTcl(Window3D *throughWin)
{
    #define ITEMVALUEPAIRS "{ Best Interactivity} 1.0\
                            { High Interactivity} 0.75\
                            { Balanced}           0.5\
                            { High Resolution}  0.25\
                            { Best Resolution}    0.0"

    char tclCommand[MAX_TOOL_COMMAND];


    throughWin->Tcl_LinkVar("highResMutexHolder", &PointerToMutexHolder);
    throughWin->Tcl_LinkVar("drewLowRes", &drewLowRes);
    throughWin->Tcl_LinkVar("interactivityHint", &interactivityHint, TCL_LINK_DOUBLE);
    throughWin->Tcl_LinkVar("numEditedObjects", &numEditedObjects, TCL_LINK_INT | TCL_LINK_READ_ONLY);
    sprintf(tclCommand, "addOption {Interactivity Hint} cascade {" ITEMVALUEPAIRS "} interactivityHint");
    throughWin->Tcl_Eval(tclCommand);

   timeControl = new TimeControl();
    timeControl->setBBox(objects->GetBoundingBoxPtr());

    imageCache = new Cache();
    imageCache->setTargetSize(350000000); //350M
    imageCache->setMaxSize(500000000); //500M
    
    return TCL_OK;
}

/*-----------------------------Input Handling--------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  The Scene3D class needs to know how long an idle cycle takes to degrade
 *  properly.  Catching the idle begin/end events allows this to happen.
 *  Such events are sunk here, so anything else that wants them should


 *  try to get to them first.
 *
 *  @param      inputType     The kind of event coming in.
 *  @param      info     ignored.
 *
 *  @return     PASS_EVENT_THROUGH / SINK_EVENT
 */
int Scene3D::HandleInput(unsigned int inputType, InputInfo *info)
{

    if (!(inputType & EnumeratedAction))
        return PASS_EVENT_THROUGH;

    if (inputType == BeginIdleAction)
    {
        // update time
        timeControl->updateCurrentTime();
        
        drewLowRes = GL_FALSE;
        drewHighRes = GL_FALSE;


        createStackedSubwindows();

        CallObjectIdle();
        return SINK_EVENT;
    }
    else if (inputType == EndIdleAction)
    {
        glFinish(); //added by mdp 7/5/2001


        objects->GetBoundingBox();
        timeControl->updateCurrentTime(GL_TRUE); 

        frametime = (int)(timeControl->getFrameTime() * 1000);
        if (frametime < 0) //at startup, frametime can be so big that the int rolls into negative territory, and causes a wicked long wait below (mdp 2/19/2004)
            frametime = 0;


        if (!drewHighRes)
            objects->RemoveObject(NULL);

        if (frametime < 40)
        {
            if (imageCache->processRequest())
            {
                timeControl->updateCurrentTime(GL_TRUE); //this is done to reset our notion of where the frame starts!
                frametime += (int)(timeControl->getFrameTime() * 1000);
            }
        }

        if (!drewLowRes)
            return SINK_EVENT;
            
        if (lastdrawhighres != 1 && !objects->IsEmpty())
        {
            if (frametime <= 10)
                degradation *= float(0.9);

            if (frametime <= 20)
                degradation *= float(0.95);
                //degradation -= .001f;

            if (frametime <= 30)
            {
                Tcl_Sleep(30 - frametime);
                timeControl->updateCurrentTime(GL_TRUE); //this is done to reset our notion of where the frame starts!
                frametime = 30;
            }

            if (frametime > 40)
            {
                if (degradation <= 0.1)
                {
                    degradation = float(0.1);
                }
                if (frametime < 50 || degradation > 0.93)
                {
                    //degradation += .001f;
                    degradation *= 1.01f;
                }
                else
                {
                    degradation += (1.0-degradation)*0.5;
                    //degradation *= 1.1;//frametime/40.0f;
                }
            }
            //cerr << "Deg adjusted: " << degradation << endl;
        }
        return SINK_EVENT;
    }
    else if (inputType == Clut::clutChangedEvent())
        objects->UpdateObjectsUsing(GetTexture(info->val.i));

    return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/** This function returns the time required to complete rendering of the most
 *  recent frame, modified so as not to report anything wildly different from
 *  the time before.
 */
long Scene3D::timeForLastFrame()
{
    static int prevTime = 30;
    int returnAmount = frametime;

    if (sessionRecorder->getRecordingState())
        return 40;
    else if (returnAmount < 30)
        returnAmount = 30;
    else if (returnAmount > 250)
        returnAmount = 250;

    if (prevTime * 1.3 < returnAmount)
        returnAmount = (int)(prevTime * 1.3);

    if (prevTime * .75 > returnAmount)
        returnAmount = (int)(prevTime * .75);

    prevTime = returnAmount;
    return returnAmount;
}

/*---------------------------------------------------------------------------*/
/** This function adds a subwindow creation request to a stack. Some other 
 *  function should create it at the appropriate time... I hope....
 */
void Scene3D::requestNewSubwindow(Object * creator, Window3D * parentWindow)
{
    SubWinReqEntry * tmp;

    tmp = new SubWinReqEntry;
    tmp->parentWindow = parentWindow;
    tmp->creator = creator;
    tmp->next = subwinstack;
    subwinstack = tmp;
}

/*---------------------------------------------------------------------------*/
void Scene3D::createStackedSubwindows()
{
    SubWindow * subwin;
    SubWinReqEntry * tmp;

    while(subwinstack != NULL)
    {
        subwin = new SubWindow(subwinstack->parentWindow);
        subwinstack->parentWindow->AddSubwindow(subwin);
        new PosableStateLock(subwinstack->creator,subwinstack->creator->getPosable(),subwin->getPoseInfo(),PosableStateLock::LockTranslate | PosableStateLock::LockFromOwner);      

        tmp = subwinstack;
        subwinstack = subwinstack->next;
        delete tmp;
    }
}

// /*---------------------------------------------------------------------------*/
// int Scene3D::setProjection(gz4d::geo::Projection const &newProjection, GLboolean override)
// {
//     if (currentProjection == newProjection)
//         return TCL_OK;
// 
//     if (currentProjection && !override)
//         return TCL_ERROR;
// 
//     currentProjection = newProjection;
//     return TCL_OK;
// }

/*---------------------------------------------------------------------------*/
bool Scene3D::setSpatialReference(gz4d::ogr::SpatialReference newSR, bool override)
{
    if (currentSpatialReference && currentSpatialReference.IsSame(newSR))
        return true;

    if (currentSpatialReference && !override)
        return false;

    currentSpatialReference = newSR;

    std::string proj_label = "global currentProjection; set currentProjection \"";

    proj_name = currentSpatialReference.exportToProj4();

    proj_label += proj_name;
    proj_label +="\"";
    mainWindow->Tcl_Eval(proj_label.c_str());

    return true;
}

/*---------------------------------------------------------------------------*/
std::shared_ptr<std::shared_ptr<TextureMap> > &Scene3D::GetDefaultTexturePtrPtr()
{
    return global_texture;
}


/*---------------------------------------------------------------------------*/
void Scene3D::setContinuousChange(unsigned char changing)
{
    if (changing)
        continuouslyChanging++;
    else
    {
        continuouslyChanging--;

        if (continuouslyChanging < 0)
            continuouslyChanging = 0;
    }
}

/*---------------------------------------------------------------------------*/
void Scene3D::SetTide(TimeVaryingObject * t)
{
    if(tide)
    {
        cerr << "Uh-oh! Tides are already present, but the new ones will replace them...\n";
    }
    tide = t;
}

/*---------------------------------------------------------------------------*/
void Scene3D::SetNavPath(NavigationPath * n)
{
    if(navPath)
    {
        cerr << "Uh-oh! A navigation path is already present, but the new ones will replace it...\n";
        //want to delete old navPath here
        //navPath->remove;

    }
    navPath = n;
}


/*---------------------------------------------------------------------------*/
void Scene3D::PostAddObject(Object *ob)
{
    globalInputMediator->Trigger(addObjectInputType->value);
}

/*---------------------------------------------------------------------------*/
void Scene3D::PostObjectMenu(int id)
{
    objects->PostMenu(id);
}

/*---------------------------------------------------------------------------*/
double Scene3D::GetHeightAt(double x, double y, Point3d coord, GLboolean secondary)
{
    return objects->GetHeightAt(x,y,coord,secondary);
}

/*---------------------------------------------------------------------------*/
void Scene3D::UpdateBBox()
{
    objects->GetBoundingBox();
}

/*---------------------------------------------------------------------------*/
void Scene3D::ProcessGZ(bpt::ptree const &xml)
{
    float num, div;

    boost::optional<std::string> attribute;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.bgcolor"))
    {
        std::stringstream color(*attribute);
        double r,g,b;
        color >> r >> g >> b;
        mainWindow->BackgroundColor(r,g,b);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.scale"))
    {
        div = 1.0;
        sscanf(attribute->c_str(), "%f:%f", &num, &div);
        mainWindow->SetScale(num/div);
        mainWindow->getPoseInfo()->setScale(num/div, ChangeHomeScale);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.spatial_reference"))
    {
        gz4d::ogr::SpatialReference sr;
        sr.SetFromUserInput(*attribute);
        setSpatialReference(sr);
    }

}

/*---------------------------------------------------------------------------*/
void Scene3D::InitSockets()
{
    std::cerr << "IO port: " << mainWindow->getIOPort() << std::endl;

    if(mainWindow->getIOPort())
    {
        gz4d::IPSocket::Address a("ANY",mainWindow->getIOPort());
        ioSocket = std::shared_ptr<gz4d::TCPSocket>(new gz4d::TCPSocket);
        try
        {
            ioSocket->Bind(a);
            ioSocket->Listen();
            ioSocket->SetNonBlocking();
        }
        catch (gz4d::Exception e)
        {
            std::cerr << "Error opening IO listening socket: " << e.what() << "\n";
            ioSocket->Close();
            ioSocket.reset();
        }
    }
    
    if(mainWindow->getSlavePort())
    {
        gz4d::IPSocket::Address a("ANY",mainWindow->getSlavePort());
        try
        {
            slaveSocket.Bind(a);
            slaveSocket.SetNonBlocking();
            std::cout << "slave listening to " << mainWindow->getSlavePort() << endl;
        }
        catch(gz4d::Exception e)
        {
            std::cerr << "Error opening slave socket: " << e.what() << "\n";
            slaveSocket.Close();
        }

    }

    if(mainWindow->getMasterPort())
    {
        gz4d::IPSocket::Address a("ANY",0);
        try
        {
            masterSocket.Bind(a);
            masterSocket.SetOption(gz4d::Socket::Broadcast,true);
            std::cout << "master broadcasting to " << mainWindow->getMasterPort() << endl;
        }
        catch(gz4d::Socket::Exception e)
        {
            std::cerr << "Error opening master socket: " << e.what() << endl;
            masterSocket.Close();
        }
    }

}

/*---------------------------------------------------------------------------*/
void Scene3D::CheckSockets()
{
    PosableStateBase * mp;
    long retval;


    gz4d::IPSocket::Address ca;
    gz4d::Socket c;
    if (ioSocket)
    {
        try
        {
            c = ioSocket->Accept(ca);
            std::cout << "Connection from " << ca.GetAddress() << endl;
            try
            {
                c.Send(std::string("---   Welcome to GeoZui4D   ---\n"));
                c.SetNonBlocking();
                ioConnections.push_back(c);
            }
            catch(gz4d::Socket::Exception e)
            {
                std::cerr << "Error sending welcome message: " << e.what() << endl;
            }
        }
        catch(gz4d::Socket::WouldBlockException)
        {}
    }

    for (SocketList::iterator c = ioConnections.begin(); c != ioConnections.end(); c++)
    {
        std::string tclstr;
        tclstr.resize(2048);
        try
        {
            c->Recv(tclstr);
            std::string retstr;
            retval = mainWindow->Tcl_Eval(tclstr, retstr);
            stringstream retstrn;
            retstrn << retval << endl << retstr << endl;
            c->Send(retstrn.str());
        }
        catch(gz4d::Socket::WouldBlockException){}
        catch(gz4d::Socket::Exception)
        {
            c = ioConnections.erase(c);
        }
    }


    if(mainWindow->getSlavePort())
    {
        std::string msg(255,0);
        gz4d::IPSocket::Address ma;
        std::string rmsg;
        // run through queued msgs and just apply the last one
        while(true)
            try
            {
                slaveSocket.RecvFrom(msg,ma);
                rmsg = msg;
            }
            catch(gz4d::Socket::WouldBlockException)
            {
                break;
            }

        if(rmsg.size() > 0)
        {
            if(!(rmsg[0] == '\n'))
            {
                Object *obj = LoadObject(rmsg,ToglWindow::interp);
                if(obj)
                {
                    AddObject(obj, ToglWindow::interp);
                }
            }
            else
            {
                if (rmsg.size() == sizeof(State))
                {
                    mainWindow->getViewInfo()->unlockFromObject();
                    State const *s = (State const *)rmsg.data();
                    mp = mainWindow->getPoseInfo();
                    mp->setPosition(s->x,s->y,s->z);
                    OrientationInfo orient;
                    orient.axis[Yaw] = s->heading;
                    orient.axis[Pitch] = s->pitch;
                    orient.axis[Roll] = s->roll;
                    mp->setOrientation(orient);

                    mp->setScale(s->scale);

                    timeControl->setOffsetAndScale(s->timeOffset,s->timeScale);
                    mainWindow->getViewInfo()->setHeightExaggeration(s->heightExaggeration);
                    globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
                }
            }
        }
    }

    if (mainWindow->getMasterPort())
    {
        PosableStateBase * mp = mainWindow->getPoseInfo();
        State s;
        s.s = '\n';
        s.x = mp->getPosition()[0];
        s.y = mp->getPosition()[1];
        s.z = mp->getPosition()[2];
        OrientationInfo const *o = mp->getOrientation();
        s.heading = o->axis[Yaw];
        s.pitch = o->axis[Pitch];
        s.roll = o->axis[Roll];
        s.scale = mp->getScale();
        s.heightExaggeration = mainWindow->getViewInfo()->getHeightExaggeration();
        s.timeOffset = timeControl->getOffset();
        s.timeScale = timeControl->getScale();
        if (!(s == lastState))
        {
            std::string msg;
            msg.append((char const *)&s,sizeof(s));
            gz4d::IPSocket::Address a(mainWindow->getMasterAddress(),mainWindow->getMasterPort());
            try
            {
                masterSocket.SendTo(msg,a);
            }
            catch (gz4d::Exception e)
            {
                std::cerr << "Master send error: " << e.what() << endl;
            }
            lastState = s;
        }
    }
#ifndef WIN32
    //wiimote.Idle();
    spaceball.Idle();
#endif

}
