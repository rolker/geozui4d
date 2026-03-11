// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// MainWindow.h
///////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------*/
class MainWindow; //this line connects the above comments with this class.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "utilities/LinkedList.h"
#include "engine/Window3D.h"

class ClutEditor;

enum {GeoZui4D, GeoNav3D, GeoCoastPilot};
/**
 *  This class implements a main viewport Window3D.
 *  This class implements the following responsibilities:
 *  \li Provide an interface between ClutEditors, Window3D's and Scene3D's.
 *  \note Derived from ToglWindow 
 *  \author commented by Matt Plumlee mdp 
 *  \date 7/14/2000
 */
class MainWindow : public Window3D
{
public:
    /*---Initialization---*/
    MainWindow();
    ~MainWindow();
    void Init(int argc, char *argv[]);
    void createObjectMenus();

    /*---Object Bookkeeping Functions---*/
    virtual void setScene3D(Scene3D *s3d);
    void setClutEditor(ClutEditor *cluted);

    /*---Input Handling Functions---*/
    int HandleInput(unsigned int inputType, InputInfo *info);

    static int getInterfaceType() {return interfaceType;}

private:
    void parseCommandLine( int argc, char* argv[] );
    virtual void initTkCallBack(Tcl_Interp *interp);
    static int interfaceType;

//    SDV_Cursor * sdvcursor;

    ClutEditor * clutEditor, *colorKey;

    //char *sdvFile;
        std::vector<std::string> startupFiles; ///< files supplied on the command line to be loaded at startup.
};

#endif
