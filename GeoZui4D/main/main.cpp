/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

// main doxygen documentation page moved to sourceDocs.h

//#include "gzVersion.h"
//#include <SDL/SDL.h>

char const *GeoZuiVersion = GZ4D_VERSION;

#include<gz4d/glo.h>
#include <GL/glut.h>

#include "MainWindow.h" 

#include "engine/Scene3D.h"
#include "engine/Object.h"
#include "engine/PosableStateBase.h"
#include "utilities/DottedAddress.h"
#include "engine/ConversionMethod.h"
#include "engine/gzCommandInterface.h"
#include "interactors/Animator.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
//#include "HackInteractor.h"

#include "engine/Interactor.h"
#include "engine/InputMediator.h"
#include "interactors/PlanarPointHandle.h"
//#include "engine/Joystick.h"
#include "interactors/InputTentacle.h"
#include "engine/PosableStateWrapper.h"
#include "interactors/RubberbandInteractor.h"
#include "engine/Wiimote.h"
#include "engine/Spaceball.h"

#ifndef WIN32
    #include <sys/types.h>
    #include <unistd.h>
    pid_t pid;
    #define GETPID getpid
#else
    #include<process.h>
    int pid;
    #define GETPID _getpid
#endif

//extern Joystick *joystickEventMediator;

#include <gz4d/ogr/link.h>

MainWindow *mainWindow;
PointHandle *pointHandle;
gzCommandInterface *gz;

#ifndef WIN32
//Wiimote wiimote;
Spaceball spaceball;
#endif

int main( int argc, char *argv[] )
{
    #ifndef WIN32
        //ifdef'd out by mdp because it doesn't crash on exit if this is taken out...
        //The problem is definitely something else, but this works for now. (3/28/2008)
        pid = GETPID();
    #endif

    glutInit(&argc,argv);

    //create the scene
    Scene3D scene3d(argc, argv);

    //set up the dotted naming hierarchy
    rootAddress->addChild(PosableNeighborhood);
    //rootAddress->addChild(ProjectionNeighborhood);
    //rootAddress->addChild(ConversionMethodNeighborhood);
    rootAddress->addChild(AnimatorNeighborhood);
    rootAddress->addChild(CouplingNeighborhood);
    rootAddress->addChild(InteractorNeighborhood);
    InteractorNeighborhood->addChild(InteractorWindowNeighborhood);
    rootAddress->addChild(EventMediatorNeighborhood);
    rootAddress->addChild(EventNeighborhood);
//  PosableNeighborhood->addChild(ObjectNeighborhood);
    PosableNeighborhood->addChild(WindowNeighborhood);

    //set up the main window and run the application
    mainWindow = new MainWindow();

//    globalInputMediator->PushInteractor(joystickEventMediator);
    gz = new gzCommandInterface;
    gz->setScene3D(&scene3d);
//  highLevelEventMediator->PushInteractor(new HackInteractor);
//  joystickEventMediator->PushInteractor(new HackInteractor);
    mainWindow->setScene3D(&scene3d);

    pointHandle = new PlanarPointHandle;
    pointHandle->publishName("pointHandle");
    globalInputMediator->PushInteractor(pointHandle->getKeyGrabber());
    InteractorNeighborhood->addChild(pointHandle->getAddress());

    gutmRubberband = new RubberbandInteractor; //done here becasue GeoZui crashes under linux if it is done statically in the RubberbandInteractor.cpp file

//    try {
    mainWindow->Init(argc,argv);
/*    } catch (gz4d::Exception e)
    {
        cerr << "Uncaught exception: " << e.what() << endl;
    }
*/    
    return 0; 
}
