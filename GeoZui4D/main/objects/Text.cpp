/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include"Text.h"
#include <filesystem>
#include"objects/dxf/DXF.h"
#include "engine/ObjectPoseState.h"
#include "GZincludes.h"
#include <gz4d/vector/ios.h>

DXF * Text::ascii[128];
int Text::asciiInitialized = 0;


extern MainWindow *mainWindow;

REGISTER_OBJECT(Text, "");

Text::Text()
{
    int i;
    char fname[35];

    text = "";
    color[0] = color[1] = color[2] = 1.0;
    lineLengths = NULL;
    lineCount = 0;
    
    justification = Left;
    
    pose = new ObjectPoseState(this);
    
    if(!asciiInitialized)
    {
        for(i=0;i<128;i++)
            ascii[i] = NULL;
        for(i='0';i<='9';i++)
        {
            sprintf(fname,"dxf/%c.dxf",i);
            loadLetter(fname,i);
        }
        for(i='a';i<='z';i++)
        {
            sprintf(fname,"dxf/%c.dxf",i);
            loadLetter(fname,i);
        }
        for(i='A';i<='Z';i++)
        {
            sprintf(fname,"dxf/%cM.dxf",i);
            loadLetter(fname,i);
        }

        sprintf(fname,"dxf/exclamation.dxf");
        loadLetter(fname,'!');
        
        sprintf(fname,"dxf/dblquote.dxf");
        loadLetter(fname,'"');
        
        sprintf(fname,"dxf/hash.dxf");
        loadLetter(fname,'#');
        
        sprintf(fname,"dxf/dollar.dxf");
        loadLetter(fname,'$');
        
        sprintf(fname,"dxf/percent.dxf");
        loadLetter(fname,'%');
        
        sprintf(fname,"dxf/amp.dxf");
        loadLetter(fname,'&');
        
        sprintf(fname,"dxf/quote39.dxf");
        loadLetter(fname,39);
        
        sprintf(fname,"dxf/quote96.dxf");
        loadLetter(fname,'`');
        
        sprintf(fname,"dxf/openp.dxf");
        loadLetter(fname,'(');

        sprintf(fname,"dxf/closep.dxf");
        loadLetter(fname,')');
        
        sprintf(fname,"dxf/asterix.dxf");
        loadLetter(fname,'*');
        
        sprintf(fname,"dxf/plus.dxf");
        loadLetter(fname,'+');
        
        sprintf(fname,"dxf/comma.dxf");
        loadLetter(fname,',');
        
        sprintf(fname,"dxf/minus.dxf");
        loadLetter(fname,'-');
        
        sprintf(fname,"dxf/period.dxf");
        loadLetter(fname,'.');
        
        sprintf(fname,"dxf/slash.dxf");
        loadLetter(fname,'/');
        
        sprintf(fname,"dxf/colon.dxf");
        loadLetter(fname,':');
        
        sprintf(fname,"dxf/semicolon.dxf");
        loadLetter(fname,';');
        
        sprintf(fname,"dxf/lessthan.dxf");
        loadLetter(fname,'<');
        
        sprintf(fname,"dxf/equal.dxf");
        loadLetter(fname,'=');
        
        sprintf(fname,"dxf/greaterthan.dxf");
        loadLetter(fname,'>');
        
        sprintf(fname,"dxf/question.dxf");
        loadLetter(fname,'?');
        
        sprintf(fname,"dxf/at.dxf");
        loadLetter(fname,'@');
        
        sprintf(fname,"dxf/openb.dxf");
        loadLetter(fname,'[');
        
        sprintf(fname,"dxf/closeb.dxf");
        loadLetter(fname,']');
        
        sprintf(fname,"dxf/backslash.dxf");
        loadLetter(fname,'\\');
        
        sprintf(fname,"dxf/caret.dxf");
        loadLetter(fname,'^');
        
        sprintf(fname,"dxf/underscore.dxf");
        loadLetter(fname,'_');
        
        sprintf(fname,"dxf/opens.dxf");
        loadLetter(fname,'{');
        
        sprintf(fname,"dxf/pipe.dxf");
        loadLetter(fname,'|');
        
        sprintf(fname,"dxf/closes.dxf");
        loadLetter(fname,'}');
        
        sprintf(fname,"dxf/tilde.dxf");
        loadLetter(fname,'~');

        asciiInitialized = 1;
    }
}

Text::~Text()
{
    delete[] lineLengths;
}

void Text::loadLetter(char * fname,char i)
{
    FILE * file;
    
    fs::path fpath(mainWindow->GetAppdir(),fname);
    file = fopen(fpath.string().c_str(),"r");
    if(file)
    {
        ascii[(int)i]=new DXF();
        ascii[(int)i]->readFrom(file);
        ascii[(int)i]->dontUseColor();
        fclose(file);
    }
    else
    {
        std::cerr << "Text::loadLetter - error opening " << fpath.string() << std::endl;
    }
}

    
Object * Text::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    std::string just;
    OrientationInfo orient;

    Object::LoadXML(xml,path,ncfile);

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.color"))
    {
        std::istringstream ss(*attribute);
        ss >> color[0];
        ss >> color[1];
        ss >> color[2];
    }
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.justification"))
    {
        if(*attribute == "center")
            justification = Center;
        else if(*attribute == "right")
            justification = Right;
    }

    orient.axis[Yaw] = xml.get("<xmlattr>.yaw",orient.axis[Yaw]);
    orient.axis[Pitch] = xml.get("<xmlattr>.pitch",orient.axis[Pitch]);
    orient.axis[Roll] = xml.get("<xmlattr>.roll",orient.axis[Roll]);
    
    boost::optional<double> double_attrib;
    if(double_attrib = xml.get_optional<double>("<xmlattr>.scale"))
        pose->scaleBy(*double_attrib);

    pose->setOrientation(orient);

    setText(xml.data());

    return this;
}

void Text::DrawLowRes(float degradation, Window3D *win)
{
    unsigned int i;
    int l = 0;
    
    if(!text.empty())
    {
        glColor3dv(color);
        glPushMatrix();
        glPushMatrix();
        if(justification == Center)
            glTranslatef(-lineLengths[l]/2.0,0.0,0.0);
        else if (justification == Right)
            glTranslatef(-lineLengths[l],0.0,0.0);
        for(i=0;i<text.size();i++)
        {
            if(ascii[(int)(text[i])])
            {
                ascii[(int)(text[i])]->DrawLowRes(degradation,win);
                glTranslatef((ascii[(int)(text[i])]->GetBoundingBox().e-(ascii[(int)(text[i])]->GetBoundingBox().w))+.1,0.0,0.0);
            }
            else if(text[i]=='\n')
            {
                glPopMatrix();
                glTranslatef(0.0,-TEXT_LINEFEED_HEIGHT,0.0);
                glPushMatrix();
                l++;
                if(justification == Center)
                    glTranslatef(-lineLengths[l]/2.0,0.0,0.0);
                else if (justification == Right)
                    glTranslatef(-lineLengths[l],0.0,0.0);
            }
            else
            {
                glTranslatef(TEXT_SPACE_WIDTH,0.0,0.0);
            }
        }
        glPopMatrix();
        glPopMatrix();
    }
}

void Text::setText(std::string t)
{
    unsigned int i,j;
    float w;
    
    text = t;
    
    lineCount = 1;
    for(i=0;i<text.size();i++)
    {
        if(text[i] == '\n')
            lineCount++;
    }
    lineLengths = new float[lineCount];
    j=0;
    w=0;
    for(i=0;i<text.size();i++)
    {
        if(text[i] != '\n')
        {
            if(ascii[(int)(text[i])])
                w +=  (ascii[(int)(text[i])]->GetBoundingBox().e-(ascii[(int)(text[i])]->GetBoundingBox().w))+TEXT_INTER_CHAR_WIDTH;
            else
                w += TEXT_SPACE_WIDTH;
            lineCount++;
        }
        else
        {
            lineLengths[j] = w;
            j++;
            w=0;
        }
    }
}
