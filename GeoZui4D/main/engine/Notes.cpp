/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "engine/Notes.h"
#include "engine/Window3D.h"
#include <iostream>
#include "engine/ToglWindow.h"
#include "engine/Scene3D.h"
#include <boost/filesystem.hpp>
#include "MainWindow.h"
#include "utilities/Utilities.h"
#include "engine/TimeControl.h"
#include "engine/ViewManager.h"
#include "engine/PosableStateBase.h"
#include <string.h>
#include <boost/property_tree/xml_parser.hpp>

extern MainWindow *mainWindow;

//using namespace std;

static int gznotes_Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv)
{
    Notes * n = (Notes*)clientData;
    
    //cerr << "Processing: " << argv[1] << endl;
    
    if(strcmp(argv[1],"add")==0)
    {
        return n->Add();
    }

    if(strcmp(argv[1],"delete")==0)
    {
        n->Delete();
        return TCL_OK;
    }
        
    if(strcmp(argv[1],"selectChanged")==0)
    {
        n->ChangeCurrent(argv[2]);
        return TCL_OK;
    }

    if(strcmp(argv[1],"selectActivated")==0)
    {
        n->Activate(argv[2]);
        return TCL_OK;
    }

    if(strcmp(argv[1],"changeLabel")==0)
    {
        n->ChangeLabel(argv[2]);
        return TCL_OK;
    }

    if(strcmp(argv[1],"changeDescription")==0)
    {
        n->ChangeDescription(argv[2]);
        return TCL_OK;
    }

    if(strcmp(argv[1],"changeCode")==0)
    {
        n->ChangeCode(atoi(argv[2]),argv[3]);
        return TCL_OK;
    }

    
    if(strcmp(argv[1],"save")==0)
    {
        n->Save(argv[2]);
        return TCL_OK;
    }
     
//  if(strcmp(argv[1],"preview")==0)
//  {
//      n->Preview(argc,argv);
//      return TCL_OK;
//  }
    
//  if(strcmp(argv[1],"record")==0)
//  {
//      n->Record(argc,argv);
//      return TCL_OK;
//  }

    if(strcmp(argv[1],"adjust")==0)
    {
        n->Adjust();
        return TCL_OK;
    }

    if(strcmp(argv[1],"open")==0)
    {
        n->Load(argv[2]);
        return TCL_OK;
    }
    
    Tcl_AppendResult(interp, "Unknown gznotes command.\n",NULL);
    return TCL_ERROR;
}

Notes::Notes(Window3D * w)
{
    w3d = w;
    Tcl_CreateCommand(ToglWindow::interp, "gznotes", gznotes_Cmd,(ClientData) this, NULL);
    
    nextid = 0;
    
    notes = NULL;
    currentNote = NULL;
    picking = 0;

    fromNote = NULL;
    moving = 0;
    movingStart = 0.0;
    preview = 0;
    
    noteCursor = NULL;

}


Notes::~Notes()
{
}

void Notes::Load(std::string const &fn)
{
    
    NoteType * n; // new note
    NoteType * nc; // cursor
    
    SetFilename(fn);

    bpt::ptree doc;
    read_xml(fn,doc);
    
    bpt::ptree node = doc.get_child("Notes");
    for(bpt::ptree::const_iterator ni = node.begin(); ni != node.end(); ++ni)
        if(ni->first == "Note")
        {
            n = new NoteType;
            n->next=NULL;

            n->index = nextid++;

            n->description = ni->second.get<std::string>("<xmltext>");

            boost::optional<std::string> attribute;
            if(attribute = ni->second.get_optional<std::string>("<xmlattr>.label"))
                n->label = *attribute;
            if(attribute = ni->second.get_optional<std::string>("<xmlattr>.code1"))
                n->code[0] = *attribute;
            if(attribute = ni->second.get_optional<std::string>("<xmlattr>.code2"))
                n->code[1] = *attribute;
            if(attribute = ni->second.get_optional<std::string>("<xmlattr>.code3"))
                n->code[2] = *attribute;
            
            n->time = ni->second.get("<xmlattr>.time",n->time);
            n->timeScale = ni->second.get("<xmlattr>.timeScale",n->timeScale);
            n->cowPosition[0] = ni->second.get("<xmlattr>.x",n->cowPosition[0]);
            n->cowPosition[1] = ni->second.get("<xmlattr>.y",n->cowPosition[1]);
            n->cowPosition[2] = ni->second.get("<xmlattr>.z",n->cowPosition[2]);
            n->scale = ni->second.get("<xmlattr>.scale",n->scale);
            n->cowOrientation.type =  OrientationInfo::typeYawPitchRoll;
            n->cowOrientation.axis[0] = ni->second.get("<xmlattr>.heading",n->cowOrientation.axis[0]);
            n->cowOrientation.axis[1] = ni->second.get("<xmlattr>.pitch",n->cowOrientation.axis[1]);
            n->cowOrientation.axis[2] = ni->second.get("<xmlattr>.roll",n->cowOrientation.axis[2]);


            std::stringstream tclstr;
            tclstr << "$Notes::left.listScroll.list insert " << n->index << " " << n->index << " -text \"" << n->label << "\" -image bitmap_notes_red_ball";
            w3d->Tcl_Eval(tclstr.str().c_str());
            tclstr.str("");
            tclstr << "$Notes::left.listScroll.list selection set " << n->index;
            w3d->Tcl_Eval(tclstr.str().c_str());

            if(notes)
            {
                nc = notes;
                while(nc->next)
                    nc = nc->next;
                nc->next = n;
            }
            else
            {
                notes = n;
            }

            currentNote = n;
            PublishCurrent();
        }

}

void Notes::Save(std::string const &fname)
{
    NoteType * n;
    FILE * o;
    
    if(notes)
    {
        if(!fname.empty())
        {
            bfs::path fn(fname);
            if((extension(fn) == ".txt") || (extension(fn) == ".csv"))
            {
                o = fopen(fname.c_str(),"w");
                if(o)
                {
                    fprintf(o,"label, code1, code2, code3, x, y, z, time, description\n");
                    n = notes;
                    while(n)
                    {
                        std::stringstream line;
                        line << n->label << ", " << n->code[0] << ", " << n->code[1] << ", " << n->code[2] << ", " << n->cowPosition[0] << ", " << n->cowPosition[1] << ", " << n->cowPosition[2] << ", " << n->time << ", " << n->description << "\n";
                        fprintf(o,"%s",line.str().c_str());
                        n=n->next;
                    }
                    fclose(o);
                }
                return;
            }
        }

        if(fname.empty() && GetFilename().empty())
        {
            w3d->Tcl_Eval("savenotesas");
            return;
        }
        if(!fname.empty())
            SetFilename(fname);
        OpenWrite();
        if(out)
        {
            fprintf(out,"<?xml version=\"1.0\"?>\n<Notes>\n");
            n = notes;
            while(n)
            {
                std::stringstream line;
                line << "\t<Note label=\"" << n->label << "\" code1=\"" << n->code[0] << "\" code2=\"" << n->code[1] << "\" code3=\"" << n->code[2] << "\" x=\"" << n->cowPosition[0] << "\" y=\"" << n->cowPosition[1] << "\" z=\"" << n->cowPosition[2] << "\" heading=\"" << n->cowOrientation.axis[0] << "\" pitch=\"" << n->cowOrientation.axis[1] << "\" roll=\"" << n->cowOrientation.axis[2] << "\" scale=\"" << n->scale << "\" time=\"" << n->time << "\" timeScale=\"" << n->timeScale << "\">" << n->description << "</Note>\n";
                
                fprintf(out,"%s",line.str().c_str());
                n=n->next;
            }
            fprintf(out,"</Notes>\n");
            CloseWrite();
        }
    }
}

void Notes::CreateNote(NoteType*&n)
{

    n = new NoteType;

    n->description="";
    
    n->code[0] = "";
    n->code[1] = "";
    n->code[2] = "";
    
    n->next=NULL;
    Snapshot(n);
}

void Notes::Snapshot(NoteType * n)
{
    n->time = Scene3D::timeControl->getGZTime();
    n->timeScale = Scene3D::timeControl->getScale();
    COPY_POINT(n->cowPosition,w3d->getViewInfo()->getPosable()->getPosition())
    n->scale = w3d->getViewInfo()->getPosable()->getScale();
    n->cowOrientation.axis[0] = w3d->getViewInfo()->getPosable()->getOrientation()->axis[0];
    n->cowOrientation.axis[1] = w3d->getViewInfo()->getPosable()->getOrientation()->axis[1];
    n->cowOrientation.axis[2] = w3d->getViewInfo()->getPosable()->getOrientation()->axis[2];
    n->cowOrientation.type = w3d->getViewInfo()->getPosable()->getOrientation()->type;
    n->cowOrientation.angle = w3d->getViewInfo()->getPosable()->getOrientation()->angle;
}

int Notes::Add()
{
    NoteType * n; // new note
    NoteType * nc; // cursor
    
    CreateNote(n);

    n->index = nextid++;
    
    std::stringstream tl;
    tl << n->index;
    n->label = tl.str();
    
    std::stringstream tclstr;
    tclstr << "$Notes::left.listScroll.list insert " << n->index << " " << n->index << " -text \"" << n->label << "\" -image bitmap_notes_red_ball";
    w3d->Tcl_Eval(tclstr.str().c_str());
    tclstr.str("");
    tclstr << "$Notes::left.listScroll.list selection set " << n->index;
    w3d->Tcl_Eval(tclstr.str().c_str());
    
    
    if(notes)
    {
        nc = notes;
        while(nc->next)
            nc = nc->next;
        nc->next = n;
    }
    else
    {
        notes = n;
    }
    
    currentNote = n;
    PublishCurrent();

    return TCL_OK;
}

void Notes::Adjust()
{
    if(currentNote)
    {
        Snapshot(currentNote);
        PublishCurrent();
    }
}

void Notes::Delete()
{
    NoteType * cursor;
    char tclstr[1000];

    if(currentNote)
    {
        if(currentNote == notes)
        {
            notes = currentNote->next;
            sprintf(tclstr,"$Notes::left.listScroll.list delete %i",currentNote->index);
            w3d->Tcl_Eval(tclstr);
            delete currentNote;
            currentNote = notes;
        }
        else
        {
            cursor = notes;
            while(cursor->next != currentNote)
                cursor = cursor->next;
            cursor->next = cursor->next->next;
            sprintf(tclstr,"$Notes::left.listScroll.list delete %i",currentNote->index);
            w3d->Tcl_Eval(tclstr);
            delete currentNote;
            currentNote = cursor->next;
            if(!currentNote)
                currentNote =  notes;
        }
        PublishCurrent();
    }
}

void Notes::PublishCurrent()
{
    char tclstr[1000];

    //sprintf(tclstr,"$Notes::details.name.e delete 0 end; $Notes::details.time.e configure -state normal; $Notes::details.time.e delete 0 end; $Notes::details.time.e configure -state readonly; $Notes::details.position.e configure -state normal; $Notes::details.position.e delete 0 end; $Notes::details.position.e configure -state readonly; $Notes::details.textScroll.text delete 1.0 end");
    sprintf(tclstr,"$Notes::details.name.e delete 0 end; $Notes::details.time.e delete 0 end; $Notes::details.position.e delete 0 end;  $Notes::details.textScroll.text delete 1.0 end; $Notes::codes.one delete 0 end; $Notes::codes.two delete 0 end; $Notes::codes.three delete 0 end; ");
    w3d->Tcl_Eval(tclstr);
    
    if(currentNote)
    {
        sprintf(tclstr,"$Notes::details.name.e insert 0 \"%s\"",currentNote->label.c_str());
        w3d->Tcl_Eval(tclstr);
        sprintf(tclstr,"$Notes::details.time.e insert 0 \"%f\"",currentNote->time);
        w3d->Tcl_Eval(tclstr);
        sprintf(tclstr,"$Notes::details.position.e insert 0 \"%f %f %f\"", currentNote->cowPosition[0],currentNote->cowPosition[1],currentNote->cowPosition[2]);
        w3d->Tcl_Eval(tclstr);
        sprintf(tclstr,"$Notes::details.textScroll.text insert end \"%s\"",currentNote->description.c_str());
        w3d->Tcl_Eval(tclstr);
        sprintf(tclstr,"$Notes::codes.one insert 0 \"%s\"",currentNote->code[0].c_str());
        w3d->Tcl_Eval(tclstr);
        sprintf(tclstr,"$Notes::codes.two insert 0 \"%s\"",currentNote->code[1].c_str());
        w3d->Tcl_Eval(tclstr);
        sprintf(tclstr,"$Notes::codes.three insert 0 \"%s\"",currentNote->code[2].c_str());
        w3d->Tcl_Eval(tclstr);
        Backup();
    }
}

void Notes::ChangeCurrent(std::string const &s)
{
    ChangeCurrent(atoi(s.c_str()));
}

void Notes::ChangeCurrent(int i)
{
    NoteType * c;

    if(preview)
    {
        preview = 0;
        fromNote = NULL;
    }
    
    if(notes)
    {
        c = notes;
        while(c)
        {
            if(c->index == i)
            {
                if(c!=currentNote)
                {
                    currentNote = c;
                    PublishCurrent();
                }
                c = NULL;
            }
            else
                c = c->next;
        }
    }
}

void Notes::Activate(std::string const &s)
{
    ChangeCurrent(s);
    if(fromNote)
        delete fromNote;
    CreateNote(fromNote);
    moving = 1;
    movingStart = Scene3D::timeControl->getSysTime();
    mainWindow->getViewInfo()->unlockFromObject();
}


void Notes::move()
{
    double p;

    OrientationInfo o;

    if(preview)
    {
        p = (Scene3D::timeControl->getGZTime()-fromNote->time)/(currentNote->time-fromNote->time);
    }
    else
    {
        p = Scene3D::timeControl->getSysTime()-movingStart;
        p /= .75;
    }

    if(p>=1.0)
    {
        Scene3D::timeControl->setTimeAndScale(currentNote->time,currentNote->timeScale);
        w3d->getViewInfo()->getPosable()->setPosition(currentNote->cowPosition);
        w3d->getViewInfo()->getPosable()->setOrientation(currentNote->cowOrientation);
        w3d->getViewInfo()->getPosable()->setScale(currentNote->scale);
        if(preview)
        {
            if(currentNote->next)
            {
                fromNote = currentNote;
                currentNote = currentNote->next;
            }
            else
            {
                fromNote = NULL;
                preview = 0;
                moving = 0;
            }
        }
        else
            moving = 0;
    }
    else
    {
        Scene3D::timeControl->setTimeAndScale(fromNote->time+(currentNote->time-fromNote->time)*p,currentNote->timeScale);
        w3d->getViewInfo()->getPosable()->setPosition((fromNote->cowPosition[0]+(currentNote->cowPosition[0]-fromNote->cowPosition[0])*p),
                                                      (fromNote->cowPosition[1]+(currentNote->cowPosition[1]-fromNote->cowPosition[1])*p),
                                                      (fromNote->cowPosition[2]+(currentNote->cowPosition[2]-fromNote->cowPosition[2])*p));
        o.axis[0] = interpolateDegrees(fromNote->cowOrientation.axis[0],currentNote->cowOrientation.axis[0],p);
        o.axis[1] = interpolateDegrees(fromNote->cowOrientation.axis[1],currentNote->cowOrientation.axis[1],p);
        o.axis[2] = interpolateDegrees(fromNote->cowOrientation.axis[2],currentNote->cowOrientation.axis[2],p);
        o.angle=currentNote->cowOrientation.angle;
        o.type=currentNote->cowOrientation.type;
        w3d->getViewInfo()->getPosable()->setOrientation(o);
        w3d->getViewInfo()->getPosable()->setScale(fromNote->scale+(currentNote->scale-fromNote->scale)*p);
    }
}

/// Initiates notes for preview mode, to preview a movie based on notes.

int Notes::Preview()
{
    if(notes)
    {
        
        if(!preview && fromNote)
            delete fromNote;
        fromNote = notes;
        currentNote = fromNote->next;
        Scene3D::timeControl->setTimeAndScale(fromNote->time,fromNote->timeScale);
        w3d->getViewInfo()->getPosable()->setPosition(fromNote->cowPosition);
        w3d->getViewInfo()->getPosable()->setOrientation(fromNote->cowOrientation);
        w3d->getViewInfo()->getPosable()->setScale(fromNote->scale);
        preview = 1;
        moving = 1;
        return 0;
    }
    return 1;
}


void Notes::ChangeLabel(std::string const &s)
{
    if(currentNote)
    {
        currentNote->label = s;
        std::stringstream tclstr;
        tclstr << "$Notes::left.listScroll.list delete " << currentNote->index;
        w3d->Tcl_Eval(tclstr.str().c_str());
        tclstr.str("");
        tclstr << "$Notes::left.listScroll.list insert " << currentNote->index << " " << currentNote->index << " -text \"" << currentNote->label << "\" -image bitmap_notes_red_ball";
        w3d->Tcl_Eval(tclstr.str().c_str());
    }
}

void Notes::ChangeDescription(const std::string &s)
{
    if(currentNote)
        currentNote->description = s;
}

void Notes::ChangeCode(int n, std::string const &s)
{
    if(currentNote && n >= 1 && n <= 3)
        currentNote->code[n-1] = s;
}
    
void Notes::Draw(Window3D * w)
{
    NoteType * c;
    gz4d::Point<double> curPos;
    double x, y, pixSize;
    
    gz4d::Point<double> r = w->getViewInfo()->getReferencePoint();
    Point3f p;
    
    
    
    c = notes;
    glColor3f(1.0,0.3f,0.3f);
    glPushAttrib(GL_POINT_BIT);
    glPointSize(10);
    while(c)
    {
        if(picking)
            glPushName(c->index);
        glBegin(GL_POINTS);
        DIFF_POINT(c->cowPosition,r,p);
        glVertex3fv(p);
        glEnd();
        glPushMatrix();
        glTranslated(p[X],p[Y],p[Z]);
        glutSolidSphere(.5,20,20);
        //glutSolidSphere(10,20,20);
        
        //Draw labels near cursor
        curPos = w->getCursorPosOnSurface();
        if (curPos.IsValid())// (x, y, c->cowPosition[Z])) //see if the cursor is nearby.  If so, show label.
        {
            pixSize = w->getViewInfo()->getPixelSizeAt(curPos);
            x = curPos[X] - c->cowPosition[X];
            y = curPos[Y] - c->cowPosition[Y];
            if ((x * x + y * y) < (100.0 * pixSize * pixSize) || (x * x + y * y) < 4.0 ) // are we within 10pix of the note?
            {
                glColor4f(1.0, 1.0, 1.0, 1.0);
                glPushMatrix();
                glLoadIdentity();
                glDisable(GL_LIGHTING);
                w->printAt(c->cowPosition[X], c->cowPosition[Y], c->cowPosition[Z], c->label,GL_FALSE,GLUT_BITMAP_HELVETICA_18);
                glEnable(GL_LIGHTING);
                glPopMatrix();
                glColor3f(1.0,0.3f,0.3f);
            }
        }
        
        glPopMatrix();
        if(picking)
            glPopName();
        c=c->next;
    }
    glPopAttrib();

}

int Notes::pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
    ChangeCurrent(names[1]);
    return GL_TRUE;
}

void Notes::renderPickableObject(Window3D *inWindow)
{
    picking = 1;
    Draw(inWindow);
    picking = 0;
}

void Notes::Backup()
{
    std::string f = GetFilename();
    Save("notes_backup.gznotes");
    SetFilename(f);
}

void Notes::ResetCursor()
{
    noteCursor = notes;
}

double Notes::GetNextTime()
{
    double ret = 0.0;
    
    if(noteCursor)
    {
        ret = noteCursor->time;
        noteCursor = noteCursor->next;
    }
    
    return ret;
}

double Notes::GetFirstNoteTime()
{
    if(notes)
        return notes->time;
    return 0.0;
}

double Notes::GetLastNoteTime()
{
    NoteType * c = notes;
    
    if(c)
    {
        while(c->next)
        {
            c = c->next;
        }
        return c->time;
    }
    else
        return 0.0;
}

