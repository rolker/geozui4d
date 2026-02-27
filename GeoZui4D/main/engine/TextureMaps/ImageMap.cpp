/* Copyright (c) 2000 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

// netcdf now handled via gz4d::netcdf wrappers
#include "engine/TextureMaps/ImageMap.h"
#include <math.h>
#include <iostream>
#include <string.h>
#include <boost/filesystem.hpp>
#include "objects/gutm.h"
#include "objects/NetCDF.h"
#include "engine/Scene3D.h"
#include "MainWindow.h"
#include <gz4d/Image.h>
#include <gz4d/image/io.h>
#include "utilities/Utilities.h"
#include "utilities/Cache.h"
#include "engine/TimeControl.h"

#ifdef WIN32
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#define DEFAULTSIZE 1000

extern MainWindow *mainWindow;

ImageMap::ImageMap()
{
    type = IMAGETEXTURE;
    mipbase = NULL;
    repeat = GL_FALSE;
    noBorder = true;
    applyMode = GL_MODULATE;

    // set up the reader to not look for BEGIN line for an IMAGETEXTURE
    SetTypeAndVersion("");

    twidth = theight = 0;
    iwidth = iheight = 0;

    firstmake = true;
    usemipmaps = 1;
    needsMake = false;
    forceLoad = false;
    enabled = false;
    haveMetaInfo = false;

    currentTimeStep = 0;
    flipYcoord = true;
    canBeCached = false;
    cacheDistanceBoost = 0.0;
}


/********************************************************************/
ImageMap::~ImageMap()
{
    Destroy();
    glDeleteTextures(1, &textureID); //added by mdp 7/17/2009
}

void ImageMap::Init()
{
  Destroy();

  if (GetFilename().empty())
    {
      SetFilename("default.tfw");
    }
    
  if (Load() != 0)
    {
        std::cerr << "Creating default image" << std::endl;
      CreateDefault();
    }
}

/********************************************************************/

void ImageMap::InitDims()
{
    if (extension(bfs::path(GetFilename())) == ".nc")
    {
        ncfile = gz4d::netcdf::File(GetFilename());
        if(ncfile)
        {
            gz4d::netcdf::Dim rowd = ncfile.GetDim("row");
            gz4d::netcdf::Dim cold = ncfile.GetDim("column");
            if(rowd && cold)
            {
                iwidth = cold.Size();
                iheight = rowd.Size();
            }
            else
            {
                std::cerr << "Error finding row or column dimension in netcdf file.\n";
            }
        }
        else
        {
            std::cerr << "Error opening netcdf file\n";
        }
        return;
    }

    canBeCached = true;

    std::string imfilename = GetFilename();   // get file name, including path
    std::string ext = extension(bfs::path(imfilename));

    if(ext == ".jgw")
        imfilename = imfilename.substr(0,imfilename.size()-3)+"jpg";
    else if (ext == ".tfw")
        imfilename = imfilename.substr(0,imfilename.size()-3)+"tif";
    else if(ext == ".pgw") //added by mdp 1/23/2009
    {
        imfilename = imfilename.substr(0,imfilename.size()-3)+"png";
        ext = ".png";
    }
    if(ext == ".png")
    {
        gz4d::image::Info info = gz4d::image::info(imfilename);
        iwidth = info.width;
        iheight = info.height;
        return;
    }
    gz4d::image::Info imginfo =  gz4d::image::info(imfilename);
    iwidth = imginfo.width;
    iheight = imginfo.height;
}

/********************************************************************/
//
// read georef data from World Header File and image data from image file
// file name suffix convention: .tfw pairs with .tif, .jgw pairs with .jpg
// WHF file format is: 
//   horiz pixel size, 2 rotation params, vert pixel size, 
//   east georef, north georef (georefs are for upper lefthand pixel)
//
int ImageMap::Load()
{
  int i;
  float rot1, rot2;
  char line[256];

  // first, determine if we have a file or a meta-file
  
  // find world header filename suffix, and construct image filename
  // from it
    if (!forceLoad && canBeCached)
    {
        Scene3D::imageCache->requestLoad(this);
        //cacheDistances[cacheLastDrawn] = -Scene3D::timeControl->getSysTime(); //negative so that closest to "now" is smallest
        //std::cerr << "Request at " << cacheDistances[cacheLastDrawn] + 1248000000.0 << "; File: " << GetFilename().string() << std::endl;
        return 0;
    }

    if(GetExtension() == ".nc")
    {
        if(!ncfile)
            InitDims();
        if(!ncfile)
            return 1;
        image.Resize(iwidth,iheight);
        ncimage = ncfile.GetVar("images");
        ncimage.SetCur(0,0,0,0);
        try
        {
            ncimage.Get<int8_t>(reinterpret_cast<int8_t*>(image.raw()),1,iheight,iwidth,4);
        }
        catch (gz4d::netcdf::Exception)
        {
            deleteImage();
        }
        gz4d::netcdf::Dim td = ncfile.GetDim("time");
        if (td)
        {
            long  timeSteps = td.Size();
            timeIndex.resize(timeSteps);
            gz4d::netcdf::Var timevar = ncfile.GetVar("time");
            timevar.SetCur(0);
            try
            {
                timevar.Get(&timeIndex.front(),timeSteps);
            }
            catch (gz4d::netcdf::Exception)
            {
                timeIndex.clear();
            }
            mainWindow->getScene3D()->GetObjectPtr(0)->expandBBoxTime(timeIndex[0]);
            mainWindow->getScene3D()->GetObjectPtr(0)->expandBBoxTime(timeIndex[timeSteps-1]);
            usemipmaps = 0;
        }
    }


    std::string imfilename = GetFilename();   // get file name, including path
    std::string ext = GetExtension();

    if(ext == ".jgw" || ext == ".tfw" || ext == ".pgw")
    {
        if(ext == ".jgw")
            ext = ".jpg";
        else if (ext == ".tfw")
            ext = ".tif";
        else if(ext == ".pgw")
            ext = ".png";
        imfilename = imfilename.substr(0,imfilename.size() - 4) + ext;

        if(Open())
            return 1;

        i = fscanf(in, "%f %f %f %f %f %f", &eastscale, &rot1, &rot2,
            &northscale, &eastref, &northref);

        if(i != 6 || rot1 != 0 || rot2 != 0)    // must have all 6 entries, and 0 rots
            return 1;

        if (!feof(in))
        {
            if (fscanf(in, "%s", line) == 1 && !strncmp(line, "REPEAT", 6))
            {
                repeat = GL_TRUE;
                noBorder = GL_TRUE;
            }
        }

        Close();
        haveMetaInfo = true;
    }
    else if (!haveMetaInfo)
    {
        // no metafile
        rot1 = rot2 = 0;
        eastscale = 1.0;
        northscale = 1.0;
        eastref = 0.0;
        northref = 0.0;
    }

    if(!ncfile)  // ncfile is always false when netcdf C++ API unavailable
    {
        if (MainWindow::getInterfaceType() != GeoCoastPilot)
            std::cerr << "Loading " << imfilename << "\n";
        if (!gzImage && !gzSmallImage)
        {
            gzImage = std::shared_ptr<gz4d::ImageRGBA8>(new gz4d::ImageRGBA8);
            gzImage->Load(imfilename);
            iwidth = gzImage->GetRaster().GetColumns();
            iheight = gzImage->GetRaster().GetRows();
            image = *gzImage; // (unsigned char (*)[4])gzImage->raw();
        }
        if (MainWindow::getInterfaceType() != GeoCoastPilot)
            std::cerr << iwidth << "x" << iheight << std::endl;
        flipYcoord = true;
        #ifdef ALPHAMAX
        // make sure that the alpha value for each pixel is <= ALPHAMAX
        for(i = 0; i < iwidth * iheight; i++)
            image[i][3] = int(ALPHAMAX * image[i][3]);
        #endif

    }

    // record the horizontal and vertical image scales
    hsize = (float)fabs(eastscale) * iwidth;
    vsize = (float)fabs(northscale) * iheight;
    if(iwidth>twidth)
        hOffsetScale = hsize;
    else
        hOffsetScale = (float(twidth) / iwidth) * hsize;
    if(iheight>theight)
        vOffsetScale = vsize;
    else
        vOffsetScale = (float(theight) / iheight) * vsize;

    // adjust the north and east georefs as needed to lower lefthand corner
    if(northscale < 0)
    {
        northref -= vsize;
        northscale *= -1;
    }
    if(eastscale < 0)
    {
        eastref -= hsize;
        eastscale *= -1;
    }

    if(!image.empty())
    {
        loaded = 1;
        return 0;
    }
    else
    {
        std::cerr << "Error loading image\n";
        image.reset();
        return 1;
    }
}

/********************************************************************/
int ImageMap::Save(std::string const &fn)
{
  return 1;
}

/********************************************************************/
bool ImageMap::imageIsReady(bool tryToBuild)
{
    if (!image.empty() || built)
        return true;

    if (canBeCached)
    {
        Scene3D::imageCache->requestLoad(this);
        cacheDistances[cacheLastDrawn] = -Scene3D::timeControl->getSysTime() - cacheDistanceBoost; //negative so that closest to "now" is smallest
        //std::cerr << "Request at " << cacheDistances[cacheLastDrawn] + 1248000000.0 << "; File: " << GetFilename().string() << std::endl;
    }
    return false;
}

/********************************************************************/
bool ImageMap::Enable(const Point3d offset)
{
  if(!imageIsReady())     // forget it if image failed to be created
    return false;

  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, transparentBlendThreshold);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  checkMake();

  if(!built)        // create OpenGL texture just in time
    Build();

  // generate texture coords automatically in both s and t directions
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  GLfloat splane[] = {(float)(1 / hOffsetScale), 0.0, 0.0, 0.0};
  glTexGenfv(GL_S, GL_OBJECT_PLANE, splane);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  GLfloat tplane[] = {0.0, (float)(1 / vOffsetScale), 0.0, 0.0};
  glTexGenfv(GL_T, GL_OBJECT_PLANE, tplane);
       
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);
  
  glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    float xoffset = (float)((offset[0] - eastref) / hOffsetScale);
    float yoffset = (float)((offset[1] - northref) / vOffsetScale);
    glTranslatef(xoffset, yoffset, 0);

  glMatrixMode(GL_MODELVIEW);
  enabled = true;
  return true;
}

/********************************************************************/

bool ImageMap::Enable()
{
    if(!imageIsReady())       // forget it if image failed to be created
        return false;

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, transparentBlendThreshold);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glBindTexture(GL_TEXTURE_2D, textureID);

    if(!built)      // create OpenGL texture just in time
        Build();
    checkMake();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, applyMode);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glScalef(widthRatio,heightRatio,1.0);
    glMatrixMode(GL_MODELVIEW);
    enabled = true;
    return true;
}

void ImageMap::Bind()
{
    if (!textureID) //texture creation moved here because there was flakiness when glGenTextures was called from a non-rendering thread (mdp 2/3/2005)
        glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

/********************************************************************/

void ImageMap::Disable()
{
  if(!enabled)     // forget it if image failed to be created
    return;

  glDisable(GL_TEXTURE_2D);

  glDisable(GL_ALPHA_TEST);
  glAlphaFunc(GL_ALWAYS, 0);
  glBlendFunc(GL_ONE, GL_ZERO);

  glMatrixMode(GL_TEXTURE);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  if (canBeCached)
  {
    cacheDistances[cacheLastDrawn] = -Scene3D::timeControl->getSysTime() - cacheDistanceBoost; //negative so that closest to "now" is smallest
    //std::cerr << "LastDrawn update at " << cacheDistances[cacheLastDrawn] + 1248000000.0 << "; File: " << GetFilename().string() << std::endl;
  }
  enabled = false;
}

/********************************************************************/

// Build a mipmap for the current image with a black border around each
// image at each level. Routine assumes that the texture is currently
// bound in OpenGL.
// twidth, theight are width and height of mipmap base
// iwidth, iheight are dimensions of image.
// twidth, theight must be integer powers of 2
// twidth >= iwidth + 2, theight >= iheight + 2
void ImageMap::makemipmap()
{
  if(image.empty())     // nothing to do if no image
    return;

  Point4ub *miplayer;
  Point4ub *oldmiplayer = NULL;
  int i;
  int w, h, ow, oh;

  ow = oh = 0;
  
    if(iwidth < twidth)
        widthRatio = (float)iwidth/(float)twidth;
    if(iheight < theight)
        heightRatio = (float)iheight/(float)theight;

  Point4ub *mipbase = new Point4ub[twidth * theight];
  if(mipbase == NULL)
    {
        std::cerr << "Out of memory allocating space for image texture!" << std::endl;
      deleteImage();
      return;
    }

  // black, transparent case
    memset(mipbase,0,twidth * theight *4);
    gluBuild2DMipmaps(GL_TEXTURE_2D, colorFormat, twidth, theight, GL_RGBA, (tColorFormat == RGB5)? GL_UNSIGNED_SHORT_5_5_5_1 : GL_UNSIGNED_BYTE, mipbase);

  // now, create the images for each of the mipmap levels and assign
  // them to the texture. Makes sure that the border pixels are always
  // transparent black
  for(i = 0, w = twidth, h = theight; w>3 || h>3; i++, w /= 2, h /= 2)
    {
    w = (w > 3? w : 3);
        h = (h > 3? h : 3);
        //std::cerr << "w,h: " << w << "," << h << std::endl;
        if(oldmiplayer || noBorder)
            miplayer = new Point4ub[w * h];
        else
            miplayer = new Point4ub[(w-2) * (h-2)];
        if(miplayer == NULL)
        {
            std::cerr << "Out of memory allocating space for image texture!" << std::endl;
            delete []mipbase;
            deleteImage();
        }
        if(oldmiplayer)
        {
            if (noBorder)
                gluScaleImage(GL_RGBA, ow, oh, GL_UNSIGNED_BYTE, oldmiplayer,(int)(w*widthRatio), (int)(h*heightRatio), GL_UNSIGNED_BYTE, miplayer);
            else
                gluScaleImage(GL_RGBA, ow, oh, GL_UNSIGNED_BYTE, oldmiplayer,(int)((w-2)*widthRatio), (int)((h-2)*heightRatio), GL_UNSIGNED_BYTE, miplayer);
        }
        else
        {
            if (noBorder)
                //gluScaleImage(GL_RGBA, iwidth, iheight, (tColorFormat == RGB5)? GL_UNSIGNED_SHORT_5_5_5_1 : GL_UNSIGNED_BYTE, image,(int)(w*widthRatio), (int)(h*heightRatio), GL_UNSIGNED_BYTE, miplayer);
                gluScaleImage(GL_RGBA, iwidth, iheight, GL_UNSIGNED_BYTE, image.raw(),(int)(w*widthRatio), (int)(h*heightRatio), GL_UNSIGNED_BYTE, miplayer);
            else
                //gluScaleImage(GL_RGBA, iwidth, iheight, (tColorFormat == RGB5)? GL_UNSIGNED_SHORT_5_5_5_1 : GL_UNSIGNED_BYTE, image,(int)((w-2)*widthRatio), (int)((h-2)*heightRatio), GL_UNSIGNED_BYTE, miplayer);
                gluScaleImage(GL_RGBA, iwidth, iheight, GL_UNSIGNED_BYTE, image.raw(),(int)((w-2)*widthRatio), (int)((h-2)*heightRatio), GL_UNSIGNED_BYTE, miplayer);
        }
        
        if (noBorder) //don't put the black border on if we're repeating (mdp 3/10/2003)
            glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, (int)(w*widthRatio), (int)(h*heightRatio), GL_RGBA, GL_UNSIGNED_BYTE, miplayer);
        else
            glTexSubImage2D(GL_TEXTURE_2D, i, 1, 1, (int)((w-2)*widthRatio), (int)((h-2)*heightRatio), GL_RGBA, GL_UNSIGNED_BYTE, miplayer);
        delete []oldmiplayer;
        oldmiplayer = miplayer;
        if (noBorder)
        {
            ow = (int)(w*widthRatio);
            oh = (int)(h*heightRatio);
        }
        else
        {
            ow = (int)((w-2)*widthRatio);
            oh = (int)((h-2)*heightRatio);
        }
  }
  delete []mipbase;

}

void ImageMap::maketexture()
{

  if(!image)        // nothing to do if no image
    return;

    if (firstmake)
    {
        if(iwidth < twidth)
            widthRatio = (float)iwidth/(float)twidth;
        if(iheight < theight)
            heightRatio = (float)iheight/(float)theight;

        unsigned char * blank = new unsigned char[(twidth+2) * (theight+2) *4];
        memset(blank,0,twidth * theight *4);
        //glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, twidth, theight, 0, GL_RGBA, (tColorFormat == RGB5)? GL_UNSIGNED_SHORT_5_5_5_1 : GL_UNSIGNED_BYTE, blank);
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, twidth, theight, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank);
        delete blank;
        firstmake = false;
  }

    if(!image.empty())
    {
        if (noBorder)
            //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iwidth, iheight,GL_RGBA,(tColorFormat == RGB5)? GL_UNSIGNED_SHORT_5_5_5_1 : GL_UNSIGNED_BYTE, image);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iwidth, iheight,GL_RGBA,GL_UNSIGNED_BYTE, image.raw());
        else
            //glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, iwidth, iheight,GL_RGBA,(tColorFormat == RGB5)? GL_UNSIGNED_SHORT_5_5_5_1 : GL_UNSIGNED_BYTE, image);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, iwidth, iheight,GL_RGBA,GL_UNSIGNED_BYTE, image.raw());
    }
}


/********************************************************************/

void ImageMap::Build()
{
  int texturesize;
  GLint w, h;
  int m, n;
  int buffer = 2;

  if (MainWindow::getInterfaceType() != GeoCoastPilot)
      std::cerr << "Build\n";
  
  if (noBorder)
      buffer = 0;

  // find minimum power of 2 > each dimension (plus border)
  for(theight = 1, m = 0; theight < iheight + buffer; theight *= 2, m++);
  for(twidth = 1, n = 0; twidth < iwidth + buffer; twidth *= 2, n++);

  // first reduce dimensions to a supported size if necessary
  do{
    //glTexImage2D(GL_PROXY_TEXTURE_2D, 0, colorFormat, twidth, theight,
    //         0, GL_RGBA, (tColorFormat == RGB5)? GL_UNSIGNED_SHORT_5_5_5_1 : GL_UNSIGNED_BYTE, NULL);   // include border
    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, colorFormat, twidth, theight,0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);   // include border
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    if(w == 0 || h == 0){
      twidth /= 2; n--;
      theight /= 2; m--;
    }
    texturesize = (theight < twidth? theight: twidth);
  }while((w == 0 || h == 0) && texturesize > 64);

  if (MainWindow::getInterfaceType() != GeoCoastPilot)
      std::cerr << "Texture size:" << twidth << "x" << theight << std::endl;

  // store  number of levels in mipmap
  miplevels = (m > n? m: n);

  if (!textureID) //texture creation moved here because there was flakiness when glGenTextures was called from a non-rendering thread (mdp 2/3/2005)
    glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // create the mip-maps to be used as texture
  if(usemipmaps) 
  {
        makemipmap();
    }
    else
    {
        maketexture();
    }
    if(!image.empty())
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if(usemipmaps)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        // the image texture should not repeat over the surface
        if (repeat)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
    else
    {
        std::cerr << "Null Image!\n";
    }

    built = 1;
    deleteImage();
    if (MainWindow::getInterfaceType() != GeoCoastPilot)
        std::cerr << "Done Build\n";
}

/********************************************************************/
void ImageMap::deleteImage()
{
    if (!gzImage)
    {
        if (!gzSmallImage)
            //delete []image;
            image.reset();
        else
            gzSmallImage.reset();
    }
    else
        gzImage.reset();

    image.reset();
}

/********************************************************************/
void ImageMap::Destroy()
{
    deleteImage();

  twidth = theight = 0;
  iwidth = iheight = 0;

  built = 0;  
  loaded = 0;
  Scene3D::imageCache->itemUnloaded(this);
}

/********************************************************************/

//
// as default prepare to create a 512 x 512 mid-grey mipmap, with alpha = 0.5
// with DEFAULTSIZE as metric
//
void ImageMap::CreateDefault()
{
  int i, j;

  std::cerr << "CreateDefault\n";
  
  iwidth = iheight = 510;
  hsize = vsize = DEFAULTSIZE;
  eastref = northref = 0;

  //image = new Point4ub[iwidth * iheight];
  //image.Resize(iwidth,iheight);
  image = gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> >(iwidth,iheight,gz4d::Pixel<gz4d::RGBA8>(gz4d::Color(.5,.5,.5,.5)));
  //if(image == NULL)
  if(image.empty())
    {
        std::cerr << "Out of memory allocating space for image texture!" << std::endl;
      built = 1;
      return;
    }

//   for(i = 0; i < iwidth * iheight; i++)
//     for(j = 0; j < 4; j++)
//       image.GetRaster().begin()[i][j] = 127;

    Enable();
    Build();
    Disable();
}

/********************************************************************/

void ImageMap::Print() const
{
    std::cout << "image dimensions: [" << iheight << ", " << iwidth << "] ";
    std::cout << "texture dimensions: [" << theight << ", " << twidth << "] ";
}

/********************************************************************/

void ImageMap::Draw(bool asQuad)
{
  if(!imageIsReady()) return;

//  glViewport(0, 0, iwidth, iheight);
//  glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    gluOrtho2D(0, iwidth, 0, iheight);
//  glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();

//  glRasterPos3i(0, 0, -1);
  if(asQuad)
  {
      float w = 1.0;
      float h = 1.0;
      if(iwidth > iheight)
          h = iheight/float(iwidth);
      if(iheight > iwidth)
          w = iwidth/float(iheight);

      Enable();
      glColor3f(1.0,1.0,1.0);
      glBegin(GL_QUADS);
        glTexCoord2f(0.0,0.0);
        glVertex2f(0.0,0.0);
        
        glTexCoord2f(1.0,0.0);
        glVertex2f(w,0.0);
        
        glTexCoord2f(1.0,1.0);
        glVertex2f(w,h);
        
        glTexCoord2f(0.0,1.0);
        glVertex2f(0.0,h);
      glEnd();
      Disable();
  }
  else
  {
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, transparentBlendThreshold);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDrawPixels(iwidth, iheight, colorFormat, (tColorFormat == RGB5)? GL_UNSIGNED_SHORT_5_5_5_1 : GL_UNSIGNED_BYTE, image);
    glDrawPixels(iwidth, iheight, colorFormat, GL_UNSIGNED_BYTE, image.raw());
    glDisable(GL_ALPHA_TEST);
    glAlphaFunc(GL_ALWAYS, 0);
    glBlendFunc(GL_ONE, GL_ZERO);
    if (canBeCached)
      cacheDistances[cacheLastDrawn] = -Scene3D::timeControl->getSysTime() - cacheDistanceBoost; //negative so that closest to "now" is smallest
  }
}


/********************************************************************/

void ImageMap::Display(Tcl_Interp *interp, float min , float max, char * target)
{
}

float ImageMap::GetTexT(float t)
{
    if (flipYcoord)
        return (float)((1.0-t) * heightRatio); // both jpg and tif should be flipped 
    else
        return (float)(t * heightRatio);
}

void ImageMap::Fuse(Gutm * gutm, float transparency)
{
    int i,j;

    //cerr << "Fuse: "  << eastref << " " << northref << " " << eastref+(iwidth*eastscale) << " " << northref+(iheight*northscale) << endl;
    
    if(!built)      // create OpenGL texture just in time
    {
        Enable();
        Build();
        Disable();
    }
    
    for(i=0;i<(iwidth);i++)
    {
        for(j=0;j<(iheight);j++)
        {
            if(gutm->HasValueInRange(eastref+(i*eastscale),northref+(j*northscale),
                                                             eastref+((i+1)*eastscale),northref+((j+1)*northscale)))
            {
//              image[(j*iwidth)+i][3]=(unsigned char)(256*transparency);
                //image[(j*iwidth)+i][3]= (short)(transparency * 256);
                image.GetRaster().GetRow(j)[i][3]= (short)(transparency * 256);
            }
        }
    }

    needsMake = true;
}

void ImageMap::checkMake()
{
        if(needsMake)
        {
          if(usemipmaps)
            {
                makemipmap();
            }
            else
            {
              maketexture();
            }
        }
        needsMake = false;
}

void ImageMap::SetRef(double e,double n,float es,float ns)
{
    eastref = e;
    northref = n;
    eastscale = es;
    northscale = ns;

    // record the horizontal and vertical image scales
    hsize = (float)fabs(eastscale) * iwidth;
    vsize = (float)fabs(northscale) * iheight;
    haveMetaInfo = true;
}

int ImageMap::TextureIdle(double time)
{
    if (!timeIndex.empty() && built)
    {
        if ((timeIndex[currentTimeStep] <= time) && (currentTimeStep < timeIndex.size()-1) && timeIndex[currentTimeStep+1]>time)
            return 0; // we are at the right time, do nothing!
        currentTimeStep = findTimeIndex(time);
        ncimage.SetCur(currentTimeStep,0,0,0);
        image.Resize(iwidth,iheight);
        ncimage.Get(reinterpret_cast<int8_t*>(image.raw()),1,iheight,iwidth,4);
        needsMake = true;
        return 1;
    }
    return 0;
}

GLboolean ImageMap::IsPlaying()
{
    if (!timeIndex.empty())
        return true;
    return false;
}

unsigned long ImageMap::findTimeIndex(double t)
{
    if(timeIndex.empty())
        return 0;
        
    // first look at the boundaries, and outside our range
    if(t <= timeIndex[0])
        return 0;
    if(t >= timeIndex.back())
        return -1;
        
    // assume regularly spaced times, guess a good starting point
    int ret = (int)(((t-timeIndex[0])/(timeIndex.back()-timeIndex[0]))*(timeIndex.size()-1));
    while(ret <  timeIndex.size()-2 && timeIndex[ret+1] <= t)
        ret++;
    while(ret > 0 && timeIndex[ret] > t)
        ret--;

    return ret;
}

/*-------------------------Cacheable Interface-------------------------------*/
/*---------------------------------------------------------------------------*/
float ImageMap::load(float min, float max, int policy)
{
    if (!canBeCached)
        return -1.0;

    if (!loaded)
    {
        forceLoad = true;
        Load();
        forceLoad = false;
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }
    return 1.0;
}

/*---------------------------------------------------------------------------*/
float ImageMap::unload(float min, int policy)
{
    if (!canBeCached)
        return -1.0;

    if (built)
    {
        if (!image.empty() && !GetFilename().empty())
            deleteImage();
        std::cerr << "Unloading " << GetFilename() << std::endl;
        loaded = 0;
        built = 0;
        needsMake = true;
        firstmake = true;
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    return 0.0;
}

/*---------------------------------------------------------------------------*/
float ImageMap::getCurrentSize()
{
    float amount = 0, pixelSize = 4;

    if (!image && !built)
        return 0.0;

//     if (tColorFormat == RGB5)
//         pixelSize = 2.0;

    if (image)
        amount = iwidth * iheight * pixelSize;
    if (textureID)
        amount += iwidth * iheight * pixelSize;

    return amount;
}

/*---------------------------------------------------------------------------*/
float ImageMap::getFullSize()
{
    float pixelSize = 4.0;

//     if (tColorFormat == RGB5)
//         pixelSize = 2.0;

    return iwidth * iheight * pixelSize;
}

/*---------------------------------------------------------------------------*/
double *ImageMap::getMeasures(int policy)
{
    if (canBeCached)
        return cacheDistances;
    else
        return NULL;
}

/*-------------------------Combining Textures--------------------------------*/
/*---------------------------------------------------------------------------*/
// Causes the current texture to be a tiled combination of the passed array of
// textures in row-major order, texturesWide tiles wide by texturesHigh tiles
// high, and scaled by a factor of scaleFactor.
// Created by mdp 12/26/2008
typedef GLubyte Point2ub[2];
typedef unsigned short uint16_t;
void ImageMap::CombineTextures(ImageMap *textures[], int texturesWide,
                               int texturesHigh,
                               float xScaleFactor,
                               float yScaleFactor)
{
    int row, col, tRow, tCol, tx = 0, ty = 0, tWidth, tHeight;
    float xPixel, yPixel;
    float xBucketAmount, yBucketAmount; //contribution to current bucket
    float xOverflow, yOverflow; //zero if all fits in bucket, otherwise, amount to flow into next bucket
    ImageMap *sourceTexture;
    //Point4ub *sourceImage;
    //Point2ub *sourceImage2=NULL;
    //Point2ub *image2=NULL;
    int idx, tdx, nidx, ydx;
    int xp, yPixelPrevious = 0;
    iwidth = 0;
    iheight = 0;

    if (yScaleFactor == 0.0)
        yScaleFactor = xScaleFactor;

    //find out the new dimensions
    for (row = 0; row < texturesHigh; row++)
    {
        iheight += textures[row * texturesWide]->GetHeight();
    }
    for (col = 0; col < texturesWide; col++)
    {
        iwidth += textures[col]->GetWidth();
    }
    iheight = (int)ceil(iheight * yScaleFactor);
    iwidth = (int)ceil(iwidth * xScaleFactor);

    //allocate our image
//     if (tColorFormat == RGB5)
//     {
//         image2 = new Point2ub[iwidth * iheight];
//         image = (Point4ub *) (void *) image2;
//     }
//     else
//         image = new Point4ub[iwidth * iheight];
    image.Resize(iwidth,iheight);

    //allocate pixel "buckets" for our new image
    float (*(buckets[2]))[4];
    buckets[0] = new float[iwidth][4];
    buckets[1] = new float[iwidth][4];
    float zero[4] = {0.0, 0.0, 0.0, 0.0};
    enum {icurrent, ioverflow};
    for (xp = 0; xp < iwidth; xp++)
    {
        copyToPixel(buckets[icurrent][xp], zero);
        copyToPixel(buckets[ioverflow][xp], zero);
    }

    //Fill 'er up.  We could use OpenGL to help, but it would take just as
    //much work to stitch together the pieces after gluScaleImage...
    //Instead, we'll keep track of mapping the passed texture pixels in
    //their own coordinates (tx, ty) to image pixels in floating point
    //coordinates.  We'll accumulate values in image pixel buckets, and
    //then not write them to the buffer until the bucket has all possible
    //contributions.
    ty = 0;
    for (row = texturesHigh - 1; row >= 0; row--) //for each row of textures
    {
        tHeight = textures[row * texturesWide]->GetHeight();
        for (tRow = 0; tRow < tHeight; tRow++) //loop through each texel row
        {
            determineContributions(ty, yScaleFactor, yPixel, yBucketAmount, yOverflow);
            tx = 0;
            for (col = 0; col < texturesWide; col++) //for each column of textures
            {
                sourceTexture = textures[row * texturesWide + col];
                tWidth = sourceTexture->GetWidth();
                gz4d::Raster< gz4d::Pixel< gz4d::RGBA8 > >::Ptr sourceImage = sourceTexture->image.GetRasterPtr();
//                 if (tColorFormat == RGB5)
//                     sourceImage2 = (Point2ub *)(void *)sourceImage;
                for (tCol = 0; tCol < tWidth; tCol++) //loop through each texel column
                {
                    determineContributions(tx, xScaleFactor, xPixel,
                                           xBucketAmount, xOverflow);
                    idx = int(xPixel);
                    tdx = tRow * tWidth + tCol;
                    
                    //fill the current bucket
//                     addToPixel(buckets[icurrent][idx], (tColorFormat == RGB5)? sourceImage2[tdx] : sourceImage[tdx],
//                                 xBucketAmount * yBucketAmount);
                    addToPixel(buckets[icurrent][idx], sourceImage->begin()[tdx],
                               xBucketAmount * yBucketAmount);

                    //fill the next bucket to the right
                    nidx = idx + 1;
                    if (xOverflow > 0 && (nidx < iwidth))
                        //addToPixel(buckets[icurrent][nidx], (tColorFormat == RGB5)? sourceImage2[tdx] : sourceImage[tdx],
                          //          xOverflow * yBucketAmount);
                    addToPixel(buckets[icurrent][nidx], sourceImage->begin()[tdx],
                               xOverflow * yBucketAmount);

                    //fill the next bucket down if necessary
                    if (yOverflow > 0)
                    {
                        //addToPixel(buckets[ioverflow][idx], (tColorFormat == RGB5)? sourceImage2[tdx] : sourceImage[tdx],
                          //          xBucketAmount * yOverflow);
                          addToPixel(buckets[ioverflow][idx], sourceImage->begin()[tdx],
                                     xBucketAmount * yOverflow);

                        //finally, fill the bucket down and to the right
                        if (xOverflow > 0 && (nidx < iwidth))
//                             addToPixel(buckets[ioverflow][nidx],
//                                         (tColorFormat == RGB5)? sourceImage2[tdx] : sourceImage[tdx],
//                                         xOverflow * yOverflow);
                        addToPixel(buckets[ioverflow][nidx], sourceImage->begin()[tdx],
                                   xOverflow * yOverflow);
                    }
                    tx++;
                }
            }
            //end of texel row items...
            //copy the bucket row to the texture image if we're ready.
            if (int(yPixel + yScaleFactor) > yPixelPrevious)
            {
                ydx = yPixelPrevious * iwidth;
                for (xp = 0; xp < iwidth; xp++)
                {
                    //copyToPixel((tColorFormat == RGB5)? image2[ydx + xp] : image[ydx + xp], buckets[icurrent][xp]);
                    copyToPixel(&(reinterpret_cast<GLubyte*>(image.GetRaster().raw())[4*(ydx + xp)]), buckets[icurrent][xp]);
                    copyToPixel(buckets[icurrent][xp], buckets[ioverflow][xp]);
                    copyToPixel(buckets[ioverflow][xp], zero);
                }
                yPixelPrevious = int(yPixel + yScaleFactor);
            }
            ty++;
        }
    }

    loaded = true;
    canBeCached = false;
    delete [] buckets[0];
    delete [] buckets[1];
}

/*---------------------------------------------------------------------------*/
// Calculates the proper pixel coordinate and bucket/overflow contributions
// based on the passed parameters
void ImageMap::determineContributions(int tv, float scaleFactor,
                                        float &pixel, float &bucketAmount,
                                        float &overflow)
{
    float next;

    pixel = tv * scaleFactor;
    next = (tv + 1) * scaleFactor;
    if (int(pixel) == int(next))
    {
        bucketAmount = scaleFactor;
        overflow = 0;
    }
    else
    {
        overflow = next - int(next);
        bucketAmount = scaleFactor - overflow;
    }
}

/*---------------------------------------------------------------------------*/
// Add r, g, b, alpha components to passed pixel.
// void ImageMap::addToPixel(float pixel[4], GLubyte *sourcePixel,
//                             float factor)
void ImageMap::addToPixel(float pixel[4], gz4d::Pixel<gz4d::RGBA8> const &sourcePixel,float factor) const
{
//     if (tColorFormat == RGB5)
//     {
//         uint16_t val = *(uint16_t *) (void*) sourcePixel;
//         int mask = 31;
//         pixel[0] += factor * ((val >> 11) & mask); //red
//         pixel[1] += factor * ((val >> 6) & mask); //green
//         pixel[2] += factor * ((val >> 1) & mask); //blue
//         pixel[3] += factor * (val & 1); //alpha
//     }
//     else
//     {
        pixel[0] += sourcePixel[0] * factor;
        pixel[1] += sourcePixel[1] * factor;
        pixel[2] += sourcePixel[2] * factor;
        pixel[3] += sourcePixel[3] * factor;
//    }
}

/*---------------------------------------------------------------------------*/
// Copy r, g, b, alpha components to passed pixel.
void ImageMap::copyToPixel(GLubyte * pixel, float sourcePixel[4])
{
    if (tColorFormat == RGB5)
    {
        uint16_t *val = (uint16_t *) (void*) pixel;
        *val = uint16_t(sourcePixel[0] + 0.5) << 11; //red
        *val |= uint16_t(sourcePixel[1] + 0.5) << 6; //green
        *val |= uint16_t(sourcePixel[2] + 0.5) << 1; //blue
        *val |= (sourcePixel[3] >= 0.5 ? 1 : 0); //alpha
    }
    else
    {
        pixel[0] = (unsigned char) sourcePixel[0];// - 15.0; //for debugging quadtree
        pixel[1] = (unsigned char) sourcePixel[1];// + 15.0; //for debugging quadtree
        pixel[2] = (unsigned char) sourcePixel[2];
        pixel[3] = (unsigned char) sourcePixel[3];
    }
}

/*---------------------------------------------------------------------------*/
// Copy r, g, b, alpha components to passed pixel.
void ImageMap::copyToPixel(float pixel[4], float sourcePixel[4])
{
    pixel[0] = sourcePixel[0];
    pixel[1] = sourcePixel[1];
    pixel[2] = sourcePixel[2];
    pixel[3] = sourcePixel[3];
}
