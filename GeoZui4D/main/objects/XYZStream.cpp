/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "GZincludes.h"
//#include <time.h>
#include "XYZStream.h"
#include "engine/ObjectPoseState.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "objects/nodeTypes/XYZPoint.h"
#include "objects/nodeTypes/XYZPointAvg.h"
#include "objects/nodeTypes/XYZPointWA.h"
#include "objects/nodeTypes/XYZPointMin.h"
#include "engine/InputMediator.h"
#include <filesystem>
#include "engine/ViewManager.h"
#include "FishSchool.h"
#include "engine/TimeControl.h"
#include <gz4d/geo/Projection.h>

extern MainWindow *mainWindow;


REGISTER_OBJECT_WITH_MENU(XYZStream, "sss") = "\
{{Hide} hide \
{Show Quadtree} showquad \
{Hide Quadtree} hidequad \
{Pause} pause \
{Unpause} unpause\
}";




XYZStream::XYZStream()
{
    showquad = 0;
    paused = 0;
    pointcount = 0;
    radius = 0.0;
    maxdeg=0;
    degres=0;
    ssfile = NULL;
    degup=degdown=0;
    gtree=NULL;
    drawLine = 0;
    altData = false;
}

XYZStream::~XYZStream()
{
}

/*---------------------------------------------------------------------------*/
int XYZStream::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    std::string message = argv[0];

    if (argc >= 1)
    {
        if(message == "showquad")
        {
                showquad = true;
                return TCL_OK;
        }
        else if (message == "hidequad")
        {
                showquad = false;
                return TCL_OK;
        }
        else if (message == "pause")
        {
                paused = true;
                return TCL_OK;
        }
        else if (message == "unpause")
        {
                paused = false;
                return TCL_OK;
        }
        else if (message == "degradation")
        {
                degres = atoi(argv[1]);
                return TCL_OK;
        }
        else if (message == "addPoint")
        {
            if (argc == 5)
            {
                Point3d pd;
                pd[0] = atof(argv[1]);
                pd[1] = atof(argv[2]);
                pd[2] = atof(argv[3]);
                double t = atof(argv[4]);
                gtree->AddPoint(pd,radius,t);
                return TCL_OK;
            }
            if (argc == 8)
            {
                gz4d::Point<float> pf;
                double lat,lon;
                lat = atof(argv[1]);
                lon = atof(argv[2]);
                pf[0] = atof(argv[3]);
                pf[1] = atof(argv[4]);
                pf[2] = atof(argv[5]);
                gz4d::Time t(atof(argv[6]));
                float val = atof(argv[7]);
                AddPoint(lat,lon,pf,t,val);
                return TCL_OK;
            }
            if (argc == 9)
            {
                gz4d::Point<float> pf;
                double lat,lon;
                lat = atof(argv[1]);
                lon = atof(argv[2]);
                pf[0] = atof(argv[3]);
                pf[1] = atof(argv[4]);
                pf[2] = atof(argv[5]);
                gz4d::Time t(atof(argv[6]));
                float val = atof(argv[7]);
                float rad = atof(argv[8]);
                AddPoint(lat,lon,pf,t,val,rad);
                return TCL_OK;
            }

            Tcl_AppendResult(interp, "Usage: addPoint x y z time\n       addPoint lat lon x y z time value [radius]", NULL);
            return TCL_ERROR;
        }
        else if (message == "initialize")
        {
            double res;
            std::string type = "swa";
            double rad = 0.0;
            if (argc >= 2)
            {
                res = atof(argv[1]);
                if(argc >= 3)
                {
                    type = argv[2];
                    if (argc == 4)
                        rad = atof(argv[3]);
                }
                Initialize(res,type,rad);
                return TCL_OK;
            }
            Tcl_AppendResult(interp, "Usage: initialize resolution [type [radius]]", NULL);
            return TCL_ERROR;
        }
        else if (message == "useAlternateData")
        {
            altData = true;
            return TCL_OK;
        }
    }
    return Object::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int XYZStream::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "showquad hidequad pause unpause ", NULL);
	return Object::list(interp);
}

void XYZStream::DrawLowRes(float degradation, Window3D *win)
{
	ObjectCollection::DrawLowRes(degradation,win);
	if(drawLine)
	{
		glColor3f(0.0,0.0,1.0);
		glBegin(GL_LINES);
		glVertex3dv(lastPos);
		glVertex3dv(lastPing);
		glEnd();
	}
	return;
}

int XYZStream::DrawHighRes(int eye, Window3D *win)
{
	return ObjectCollection::DrawHighRes(eye,win);

}

// documented in Object
int XYZStream::readFrom(FILE * infile)
{
	char instring[500];
	char fname[500] = "";
	double res = 1.0;
	double rad = 0.0;
	char type[100];

        std::cerr << "XYZStream::readFrom\n";

        while(fgets(instring, 500, infile) && strstr(instring,"BEGIN SOUNDING_STREAM\n")== NULL)
	{
	}
	while(!feof(infile))
	{
		sscanf(instring,"FNAME %s ",fname);
		char tmp[100];
		sscanf(instring,"FTYPE %s ",tmp);
		ftype = tmp;
		sscanf(instring,"RESOLUTION %lf ",&res);
		sscanf(instring,"RADIUS %lf ",&rad);
		sscanf(instring,"HANDLER %s ",type);
		int i;
		if(sscanf(instring,"POSITIVE_DEPTH %i ",&i) && i)
			posd = true;
		sscanf(instring,"MAX_DEGRADE %i",&maxdeg);
		if(!fgets(instring, 500, infile))
                    break;
	}
	if(strlen(fname))
	{
		InitFile(fs::path(GetFilenameAsPath().parent_path()/fname).string(),type,res,rad);
	}
	return 0;
}

void XYZStream::InitFile(std::string const &fn,std::string type,double res, double rad)
{
    Point3d gr;

    if((ssfile = fopen(fn.c_str(),"r"))!=NULL)
    {
        Initialize(res,type,rad);
        if(ftype == "utmxyz" || ftype == "llxyz")
        {
            if(fscanf(ssfile,"%lf %lf %lf ", &gr[0], &gr[1], &gr[2]) == 3)
            {
                if(posd == 1)
                {
                    gr[2] = -gr[2];
                }
                if(ftype == "llxyz")
                {
                    projectionRequired = GL_TRUE;
                    if (autoProjection(gr[X],gr[Y]))
                        return;
                    gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                    gz4d::Point<double> xy = proj(gr[Y], gr[X]);
                    gr[X] = xy[X];
                    gr[Y] = xy[Y];
                }
                SetPosition(gr);
            }
            fseek(ssfile,0,SEEK_SET);
        }
        if(ftype == "Remus")
        {
            projectionRequired = GL_TRUE;
        }
    }
    else
    {
        std::cerr << "Error opening file: " <<  fn << std::endl;
    }
}

void XYZStream::Initialize(double res, std::string type, double rad)
{
    resolution = res;
    radius = rad;

    if(!gtree)
    {
        gtree = new GutmTree(altData);
        gtree->SetLabel("Tree");
        gtree->Initialize(type, resolution);
        AddObject(gtree);
    }

    degcount = 0;

    if(radius == 0.0)
    {
        radius = res;
    }
}

void XYZStream::HandlePacket(std::shared_ptr<gz4d::Socket::Packet> &data)
{
    gz4d::binary::Wrapper<gz4d::binary::Sonar::Channel::BottomDetection> s(data);
    if(s->IsValid())
    {
        gz4d::Point<float> p;
        p[0] = s->x;
        p[1] = s->y;
        p[2] = s->z;
        //std::cerr << "XYZStream::HandlePacket: " << p << "\tbackscatter:" << *std::max_element(s->samples.values,s->samples.values+s->samples.size) << "\tchannel:" << s->channelID << std::endl;
        AddPoint(s->latitude,s->longitude,p,s->timestamp,*std::max_element(s->samples.values,s->samples.values+s->samples.size),1.5*std::max(s->variance.x,s->variance.y));
    }
    else
    {
        ObjectCollection::HandlePacket(data);
    }
}

void XYZStream::Idle(double time)
{
    ObjectCollection::Idle(time);
    if(ssfile != NULL)
    {
        LoadNext();
    }
}

void XYZStream::LoadNext()
{
    int i;
    Point3d pd;
    double altitude;
    char line[200];
    int added = 0;

    if(ftype == "Remus")
    {
        for(i=0;i<1 && (paused != 1); i++)
        {
            if(fgets(line,200,ssfile))
            {
                if(sscanf(line,"%lf, %lf, %lf, %lf,", &pd[1], &pd[0], &altitude, &pd[2])==4)
                {
                    autoProjection(pd[0],pd[1]);
                    gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                    gz4d::Point<double> xy = proj(pd[1],pd[0]);
                    pd[0] = xy[0];
                    pd[1] = xy[1];
                                        //projectXY(1,&pf[0],&pf[1],GL_FALSE);
                    pd[2] =-pd[2]-altitude;
                    gtree->AddPoint(pd,radius);
                    added = 1;
                }
            }
        }
    }
    else
    {
        for(i=0;i<100 && (paused != 1); i++)
        {
            if(fgets(line,200,ssfile))
            {
                if(sscanf(line,"%lf %lf %lf", &pd[0], &pd[1], &pd[2])==3)
                {
                    if(ftype == "llxyz")
                    {
                        gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                        gz4d::Point<double> xy = proj(pd[1],pd[0]);
                        pd[0] = xy[0];
                        pd[1] = xy[1];
                    }
                    if(posd == 1)
                    {
                        pd[2] = -pd[2];
                    }
                    gtree->AddPoint(pd,radius);
                    added = 1;
                }
            }
        }
    }

    if(added && !mainWindow->getScene3D()->timeControl->isPaused())
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

void XYZStream::AddPoint(double lat, double lon, gz4d::Point<float> const &p, gz4d::Time t, float value, float rad)
{
    if (rad < 0.0)
        rad = radius;

    Point3d position;
    autoProjection(lon,lat);
    gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
    gz4d::Point<double> xy = proj(lat,lon);
    position[0] = xy[0] + p[0];
    position[1] = xy[1] + p[1];
    position[2] = p[2];

    gtree->AddPoint(position,rad,t.secondsSinceEpoch().asSeconds(),value);

    if(!mainWindow->getScene3D()->timeControl->isPaused())
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

void XYZStream::ShowInfo(GLuint *names, int numNames)
{
	Scene3D *scene;

    char countstr[100];

	scene = mainWindow->getScene3D();

    sprintf(countstr,"Sounding count: %li",pointcount);

	scene->appendInfo(countstr, GL_TRUE);
	scene->appendInfo("--- Generic Info:---\n");

	Object::ShowInfo(names, numNames);
}

int XYZStream::writeTo(FILE * outfile)
{
//	fprintf(outfile,"BEGIN SSS 1.0\n");
//	fprintf(outfile,"CELLSIZE %f\n",resolution);
//	fprintf(outfile,"LOCAL_ORIGIN %f, %f, %f\n", geoRef[0], geoRef[1], geoRef[2]);
//	quadtree->writeTo(outfile);
	return TCL_OK;
}

Object * XYZStream::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    std::string type;
    double res = 1.0;
    double rad = 0.0;
    Point3d p;
    fs::path fn;

    Object::LoadXML(xml,path,ncfile);

    ftype = xml.get("<xmlattr>.streamtype",ftype);
    res = xml.get("<xmlattr>.resolution",res);
    rad = xml.get("<xmlattr>.radius",rad);
    type = xml.get("<xmlattr>.handler",type);

    boost::optional<std::string> attribute;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.positive_depth"))
        posd = gz4d::IsTrue(*attribute);
    if(attribute = xml.get_optional<std::string>("<xmlattr>.alternateData"))
        altData = gz4d::IsTrue(*attribute);

    fn = xml.get<std::string>("<xmlattr>.filestream","");

    if (!fn.empty())
    {
        std::cerr << fn.filename() << std::endl;
        InitFile((path/fn).string(),type,res,rad);
    }
    else
    {
        if(!type.empty())
        {
            Initialize(res,type,rad);
        }
    }

    for(bpt::ptree::const_iterator node = xml.begin(); node != xml.end(); ++node)
    {
        if(node->first == "Depth")
        {
            if(attribute = xml.get_optional<std::string>("<xmlattr>.position"))
            {
                std::stringstream ss(*attribute);
                ss >> p[0] >> p[1] >> p[2];
            }
            boost::optional<double> double_attrib;
            if(double_attrib = xml.get_optional<double>("<xmlattr>.time"))
                gtree->AddPoint(p,rad,*double_attrib);
            else
                gtree->AddPoint(p,rad);
        }
        if(node->first == "Input")
        {
            if(attribute = xml.get_optional<std::string>("<xmlattr>.type"))
            {
                if (*attribute == "netcdf")
                {
                    //same format as FishSchool's netcdf point data
                    fs::path ncfilename;
                    if(attribute = xml.get_optional<std::string>("<xmlattr>.filename"))
                    {
                        ncfilename = *attribute;
                        std::stringstream progCaption;
                        progCaption << "Loading " << ncfilename.filename();
                        createProgress(progCaption.str());
                        FishSchool school((path/ncfilename).string());
                        double e = school.GetBaseEasting();
                        double n = school.GetBaseNorthing();
                        double baseTime = school.GetBaseTime();
                        long fishCount = school.GetFishCount();
                        for (long i = 0; i < fishCount; i++)
                        {
                            FishSchool::Fish f = school[i];
                            Point3d position;
                            position[0] = f.position[X]+e;
                            position[1] = f.position[Y]+n;
                            position[2] = f.position[Z];
                            if (f.position[Z] != 0.0)
                                gtree->AddPoint(position,rad,f.time+baseTime,f.size);
                            if (i%250 == 0)
                                updateProgress(i*100.0/fishCount);
                        }
                        killProgress();
                    }
                }
            }
        }
    }


    return this;
}
