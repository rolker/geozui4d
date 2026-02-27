#include "engine/InputMediator.h"
#include "engine/Scene3D.h"

#include "utilities/Utilities.h"
#include "engine/Window3D.h"
#include <gz4d/acoustic/BeamPattern.h>
#include <gz4d/acoustic/RectangularElement.h>
#include <gz4d/acoustic/CircularElement.h>
#include "BeamPattern.h"

REGISTER_OBJECT(BeamPattern, "");

BeamPattern::BeamPattern():amplitude(1.0),footprint(60),fp_steps(60),thetaInterval(0.0,M_PI/2.0),phiInterval(0.0,M_PI),buildClut(true)
{
    gz4d::Raster<float>::Ptr r(new gz4d::Raster<float>);

    raster.SetRaster(r);
    minDB = -10.0;
    raster.SetValueScaler(-1.0/minDB,1.0);
    thetaRes = 180;
    phiRes = 180;

    elementType = "Rectangular";

    N = 1;
    M = 1;

    Dx = 1.0;
    Dy = 1.0;

    Lx = 0.5;
    Ly = 0.5;

    xSteering = 0.0;
    ySteering = 0.0;

    plot.SetRaster(raster);


    grid.SetStartPoint(gz4d::SphericalPoint<float>(0.0,gz4d::Radians(-90.0),0.0));
    grid.AddDimension(gz4d::SphericalPoint<float>(0.0,gz4d::Radians(5.0),0.0),36);
    grid.AddDimension(gz4d::SphericalPoint<float>(0.2,0.0,0.0),5);

    Build();
}

void BeamPattern::Build()
{
    gz4d::acoustic::Element::Ptr e;
    if (elementType == "Rectangular")
        e = gz4d::acoustic::Element::Ptr(new gz4d::acoustic::RectangularElement(gz4d::acoustic::Size(Lx,Ly)));
    else if (elementType == "Line")
        e =gz4d::acoustic::Element::Ptr(new gz4d::acoustic::LineElement(Lx));
    else if (elementType == "Circular")
        e =gz4d::acoustic::Element::Ptr(new gz4d::acoustic::CircularElement(Lx));
    else
        e =gz4d::acoustic::Element::Ptr(new gz4d::acoustic::Element());
    array.SetElement(e);

    array.SetNM(N,M);
    array.SetSpacing(gz4d::acoustic::Size(Dx,Dy));

    array.SetSteering(xSteering, ySteering);

    gz4d::acoustic::Environment env;
    double w = 2.0*M_PI;
    
    fp_center = gz4d::Vector<double,3>(sin(xSteering)/(cos(xSteering)*cos(ySteering)),sin(ySteering)/(cos(xSteering)*cos(ySteering)),1.0);
    double epsilon = 0.00001;

    for(int i = 0; i < fp_steps; ++i)
    {
        double lx = std::max(N*Dx,Lx);
        double ly = std::max(M*Dy,Ly);
        double inc = std::min(.1/lx,.1/ly);
        double a = 2.0*M_PI*i/double(fp_steps);
        double dx = sin(a);
        double dy = cos(a);
        double x = xSteering;
        double y = ySteering;
        gz4d::acoustic::Wave wave(w);
        double response = 1.0;

        footprint[i] = fp_center;
        while((fabs(response-0.5) > epsilon)&&(cos(x)*cos(y)>.1))
        {
            gz4d::Vector<double,3> v(sin(x),sin(y),cos(x)*cos(y));
            wave.SetDirection(gz4d::acoustic::Wave::Direction(normalize(v)));
            double resp = array.Response(wave,env);
            resp *= resp;
            if(fabs(response-resp) > fabs(response-0.5)) // did we overshoot?
            {
                inc *= -.5; // change direction, and go at half 'speed'
            }
            x += dx*inc;
            y += dy*inc;
            response = resp;
            footprint[i][0]=v[0]/v[2];
            footprint[i][1]=v[1]/v[2];
            footprint[i][2]=1.0;
        }

    }

    raster.GetRaster()->Resize(thetaRes,phiRes);
    raster.SetPhiInterval(phiInterval);
    raster.SetThetaInterval(thetaInterval);
    raster.SetValueScaler(-1.0/minDB,1.0);

    gz4d::acoustic::BeamPattern(array, raster,w, env, amplitude);
    plot.SetRaster(raster);

    pclut.Clear();
    clutnode * c = new clutnode;
    c->height = 0.0;
    c->color[0] = 64;
    c->color[1] = 64;
    c->color[2] = 64;
    c->color[3] = 255;
    pclut.AddNode(c);
    c = new clutnode;
    c->height = 1.0-(-6.0/minDB);
    c->color[0] = 64;
    c->color[1] = 128;
    c->color[2] = 128;
    c->color[3] = 255;
    pclut.AddNode(c);
    c = new clutnode;
    c->height = 1.0-(-3.0/minDB);
    c->color[0] = 255;
    c->color[1] = 128;
    c->color[2] = 128;
    c->color[3] = 255;
    pclut.AddNode(c);
    c = new clutnode;
    c->height = 1.0;
    c->color[0] = 255;
    c->color[1] = 255;
    c->color[2] = 255;
    c->color[3] = 255;
    pclut.AddNode(c);
    buildClut = true;

}


int BeamPattern::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    if (argc >= 1)
    {
        std::string command(argv[0]);
        if (command ==  "setNM")
        {
            if(argc == 3)
            {
                N = atoi(argv[1]);
                M = atoi(argv[2]);
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (command ==  "setSpacing")
        {
            if(argc == 3)
            {
                Dx = atof(argv[1]);
                Dy = atof(argv[2]);
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (command ==  "setL")
        {
            if(argc == 3)
            {
                Lx = atof(argv[1]);
                Ly = atof(argv[2]);
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (command ==  "setResolution")
        {
            if(argc == 3)
            {
                thetaRes = atoi(argv[1]);
                phiRes = atoi(argv[2]);
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (command ==  "setSteering")
        {
            if(argc == 3)
            {
                xSteering = atof(argv[1]);
                ySteering = atof(argv[2]);
                return TCL_OK;
            }
            return TCL_ERROR;
        }

        if (command ==  "setType")
        {
            if(argc == 2)
            {
                elementType = argv[1];
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (command ==  "setAmplitude")
        {
            if(argc == 2)
            {
                amplitude = atof(argv[1]);
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (command ==  "setMinDB")
        {
            if(argc == 2)
            {
                minDB = atof(argv[1]);
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (command ==  "setRange")
        {
            if(argc == 5)
            {
                thetaInterval = gz4d::Interval<float>(atof(argv[1]),atof(argv[2]));
                phiInterval = gz4d::Interval<float>(atof(argv[3]),atof(argv[4]));
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (command == "update")
        {
            Build();
            globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
            return TCL_OK;
        }
    }
    return Object::perform(argc, argv, interp);
}

void BeamPattern::DrawLowRes(float degradation, Window3D *win)
{
    if(buildClut)
    {
        pclut.Build();
        buildClut = false;
    }
    if(customtextureptr)
        EnableTexture(TEXTURE_COORD_NONE);
    else
        pclut.Enable();
    glPushMatrix();
    glColor3f(1.0,1.0,1.0);
    plot.Draw();
    glPopMatrix();
    if(customtextureptr)
        DisableTexture();
    else
        pclut.Disable();
    glColor3f(1.0,1.0,1.0);

    grid.Draw();

    glPushMatrix();
    glRotatef(90.0,0.0,0.0,1.0);
    grid.Draw();
    glPopMatrix();


    glPushAttrib(GL_LINE_BIT|GL_LIGHTING_BIT|GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0,0.0,0.0);
    gz4d::glo::Vertex(fp_center);
    glColor3f(0.5,0.5,0.5);
    for(int i = 0; i < fp_steps; ++i)
    {
        gz4d::glo::Vertex(footprint[i]);
    }
    gz4d::glo::Vertex(footprint[0]);
    glEnd();
    glColor3f(1.0,1.0,1.0);

/*    glBegin(GL_LINES);
    glColor3f(1.0,0.0,0.0);
    for(int i = 0; i < fp_steps; ++i)
    {
        gz4d::glo::Vertex(footprint[i]);
        glVertex3d(0.0,0.0,0.0);
    }
    gz4d::glo::Vertex(footprint[0]);
    glVertex3d(0.0,0.0,0.0);
    glEnd();*/
    glColor3f(1.0,1.0,1.0);

    for(int i=0;i<=5;i++)
    {
        std::stringstream label;
        label << (5-i)*minDB/5.0 << "dB";
        win->printAt(i/5.0,0.0,0.0,label.str(),false,GLUT_BITMAP_HELVETICA_18);
        win->printAt(0.0, i/5.0,0.0,label.str(),false,GLUT_BITMAP_HELVETICA_18);

    }


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.5);

    if (minDB < -3.0)
    {
        glColor4f(1.0,0.8,0.8,0.9);
        glBegin(GL_LINE_STRIP);
        float r = 1.0+3.0/minDB;
        for(int i = 0; i <= 30; ++i)
        {
            float a = M_PI*i/30.0;
            glVertex3f(cos(a)*r,0.0,sin(a)*r);
        }
        glEnd();
        glBegin(GL_LINE_STRIP);
        for(int i = 0; i <= 30; ++i)
        {
            float a = M_PI*i/30.0;
            glVertex3f(0.0,cos(a)*r,sin(a)*r);
        }
        glEnd();
    }

    glBegin(GL_LINES);

    for(int i = 1; i < 180; ++i)
    {
        float alpha = 0.35;
        float len = 1.0+3.0/minDB;
        if(i%10==0)
        {
            alpha = 0.5;
            len = .5;
        }
        if(i%30==0)
        {
            alpha = 0.9;
            len = .05;
        }

        glColor4f(1.0,1.0,1.0,alpha);
        float a = M_PI*i/180.0;
        glVertex3f(cos(a)*1.0, 0.0,sin(a)*1.0);
        glVertex3f(cos(a)*len, 0.0,sin(a)*len);
        glVertex3f(0.0, cos(a)*1.0,sin(a)*1.0);
        glVertex3f(0.0, cos(a)*len,sin(a)*len);
    }

    glEnd();
    glPopAttrib();
}

Object * BeamPattern::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object::LoadXML(xml,path, ncfile);

    N = xml.get("<xmlattr>.N",N);
    M = xml.get("<xmlattr>.M",M);
    Dx = xml.get("<xmlattr>.NSpacing",Dx);
    Dy = xml.get("<xmlattr>.MSpacing",Dy);
    Lx = xml.get("<xmlattr>.Lx",Lx);
    Ly = xml.get("<xmlattr>.Ly",Ly);
    amplitude = xml.get("<xmlattr>.Amplitude",amplitude);
    elementType = xml.get("<xmlattr>.ElementType",elementType);
    minDB = xml.get("<xmlattr>.MinDB",minDB);

    Build();
    return this;
}

std::string BeamPattern::GetHUDString()
{
    std::stringstream ret;
    ret << "N:" << N << "  M:" << M;

    if (N>1)
        ret << "  Dx:" << Dx;

    if (M>1)
        ret << "  Dy:" << Dy;

    ret << "  " << elementType;

    if (elementType == "Line")
        ret << "  L:" << Lx;

    if (elementType == "Circular")
        ret << "  R:" << Lx/2.0;

    if (elementType == "Rectangular")
        ret << "  Lx:" << Lx << "  Ly:" << Ly;

    if (xSteering != 0.0)
        ret << " Xs:" << 180*xSteering/M_PI;

    if (ySteering != 0.0)
        ret << " Ys:" << 180*ySteering/M_PI;

    return ret.str();
}
