// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// gzCommandInterface.h
///////////////////////////////////////////////////////////////////////////////
class gzCommandInterface; //this line connects the above comments with this class.

#ifndef GZCOMMANDINTERFACE_H
#define GZCOMMANDINTERFACE_H

class Scene3D;
class Window3D;
class MainWindow;
class Object;
class Addressable;
class DottedAddress;
class Stopwatch;
class PosableStateBase;
class DynamicTides;
struct Tcl_Interp;
extern DottedAddress *PosableNeighborhood;

/**
 *	This class defines the Tcl commands specific to GeoZiu3D.
 *
 *  \author Matt Plumlee (mdp)
 *  \date 12/7/2001
 */
class gzCommandInterface
{
  public:
	/*---Initialization---*/
	gzCommandInterface();
	~gzCommandInterface();
	int Gz_Init(Tcl_Interp *interp);

	/*---Commands---*/
	int PosableCommand(int argc, const char *argv[]);
#ifdef EYE_TRACKER
	int EyeTrackCommand(int argc, const char *argv[]);
#endif
	int ControlStopwatch(int argc, const char *argv[]);
	int LoadObject(int argc, const char *argv[]);
	int NewObject(int argc, const char *argv[]);
	int SaveObject(int argc, const char *argv[]);
	int SaveEditedObjects(int argc, const char *argv[]);
	int HideObject(int argc, const char *argv[]);
	int DeleteObject(int argc, const char *argv[]);
	int ReloadObject(int argc, const char *argv[]);
	int ExplainObject(int argc, const char *argv[]);
	int ShowObject(int argc, const char *argv[]);
	int Widgets(int argc, const char *argv[]);
	int BuildScene(int argc, const char *argv[]);
	int SaveScene(int argc, const char *argv[]);
	int htmlInfo(int argc, const char *argv[]);
	int ClearScene();
        std::string GetObjectTypes();
        std::string GetObjectExtensions();
        std::string GetExtension(int object);
	int ProjectionCommand(int argc, const char *argv[]);
	int AnimatorCommand(int argc, const char *argv[]);
	int AddressCommand(int argc, const char *argv[]);
	int EventCommand(int argc, const char *argv[]);
	int TimeCommand(int argc, const char *argv[]);
	int MaxTransparencyLevelCommand(int argc, const char *argv[]);
	int PostObjectMenu(int id);
	
// 	// video recording
// 	int StartVideoRecorder(int argc, const char *argv[]);
// 	int AddFrame(int argc, const char *argv[]);
// 	int RecordFrame(int argc, const char *argv[]);
// 	int StopVideoRecorder(int argc, const char *argv[]);

	/*---Object Bookkeeping---*/
	void setScene3D(Scene3D *s3d);
	void addObject(Object *object, int flags);
        static int findPosable(std::string posableName, Addressable *&address,
					PosableStateBase *&pose,
					GLboolean reportError = GL_TRUE, DottedAddress *loc = PosableNeighborhood);
        static int findAddressIn(std::string name, DottedAddress *neighborhood,
							 Addressable *&address, DottedAddress *neighborhood2 = NULL);
        static int findWindow(std::string windowName, Addressable *&address,
				Window3D *&win, GLboolean reportError = GL_TRUE);
        static int findObject(std::string objectName, Addressable *&address,
				Object *&ob, GLboolean reportError = GL_TRUE);
        static unsigned int convertEvent(std::string name, Interactor *context = NULL);
	static int listChildrenOf(DottedAddress *parentAddress, GLboolean listFullName = GL_TRUE);
        static long parseCouplingType(std::string couplingString,
								  GLboolean reportError = GL_TRUE);
	static long getUniqueCount();

	Scene3D *getScene3D() {return scene3d;}

	static void setDefaultTides(DynamicTides *dt) {dTides = dt;}
	static DynamicTides *getDefaultTides() {return dTides;}

  private:
	static Tcl_Interp *interp;
	static DynamicTides *dTides;
	static long uniqueCount;
	Scene3D *scene3d;
	MainWindow *mainwin;
	Stopwatch *stopwatch;
};

#endif
