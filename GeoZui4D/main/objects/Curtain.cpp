/* Copyright (c) 2000 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
////////////////////////////////////////////////////////////////////
// Curtain.cpp: implementation of the Curtain display class.
////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "Curtain.h"
#include "engine/Window3D.h"
#include "engine/TextureMaps/TextureMap.h"
#include "engine/TextureMaps/Clut.h"
#include "widgets/Sv_Standards.h"
#include <filesystem>

using namespace std;

extern int dominantEye;

REGISTER_OBJECT(Curtain, "ctn");

#define sqr(x)  ((x) * (x))

#define MAX_PANEL_WIDTH 512 // the maximum width of a curtain panel

const char *CurtainNames[] = {OBJECT_NAMES, "CLUT", "RGB", "RGBA", 
                  "USE_LIGHTING", "XY_ONLY", 
                  "NCOLS", "Z_STEP", "NTH", NULL};

//////////////////////////////////////////////////////////////////////
// some utility print routines
//////////////////////////////////////////////////////////////////////

void printvec(const Point3f &v)
{
  cout << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
}

void printcolor(const Point4ub &c)
{
  cout << "[" << int(c[0]) << ", " << int(c[1]) << ", " << 
    int(c[2]) << ", " << int(c[3]) << "]";
}

//////////////////////////////////////////////////////////////////////
// Methods for ColumnData structure
//////////////////////////////////////////////////////////////////////

//ColumnData::ColumnData(double x, double y, double z, int nvals)
ColumnData::ColumnData(int nvals)
{
  data = NULL;
  colors = NULL;

  SetSize(nvals, INDEX);

  normal[0] = normal[1] = normal[2] = 0;
}

ColumnData::~ColumnData()
{
  delete []data;
  delete []colors;
}

void ColumnData::SetSize(int nvals, int datatype)
{

  nSteps = nvals;

  delete []data;
  delete []colors;

  if(nSteps == 0)
    {
      data = NULL;
      colors = NULL;
    }
  else
    {
      if(datatype == INDEX)
    data = new unsigned char[nSteps];
      else
    colors = new Point4ub[nSteps];

      if(data == NULL && colors == NULL)
    {
      cerr << "Out of memory allocating color table for curtain!" << endl;
      nSteps = 0;
    }
    }
}

void ColumnData::Print()
{
//  cout << "ColumnData location: "; printvec(location); 
  cout << " normal: "; printvec(normal);
  cout << " nSteps: " << nSteps << "\n";
  for(int i = 0; i < nSteps; i++)
    cout << int(data[i]) << " ";
  cout << endl;
}

//////////////////////////////////////////////////////////////////////
// Methods for PanelTexture structure
//////////////////////////////////////////////////////////////////////
PanelTexture::PanelTexture(int w, int h)
{
  glGenTextures(1, &textureID);

  image = NULL;

  width = height = -1;
  SetSize(w, h);
}

PanelTexture::~PanelTexture()
{
  glDeleteTextures(1, &textureID);  // free up existing texture
    
  delete []image;
}

void PanelTexture::SetSize(int w, int h)
{
  if(width != w || height != h)
    {
      delete []image;

      width = w;
      height = h;

      if(width == 0 || height == 0)
    image = NULL;
      else
    {
      image = new Point4ub[width * height];
      if(image == NULL)
        {
          cerr << "Out of memory allocating space for curtain!" << endl;
          width = height = 0;
        }
      // make sure we have a clean blank transparent image
      else //moved inside so we only blank it out once!
        memset(image, 0, height * width * 4);
    }
    }
}

void PanelTexture::Print()
{
  cout << "PanelTexture textureID: " << int(textureID);
  cout << " width: " << width << " height: " << height << "\n";

  for(int col = 0; col < 3; col++)
    {
      for(int row = 0; row < 3; row++)
    printcolor(image[row * width + col]); cout << " ";
      cout << "\n";
    }
  cout << endl;
}

//////////////////////////////////////////////////////////////////////
// Methods for Panel structure
//////////////////////////////////////////////////////////////////////
Panel::Panel(int n)
{
  startColumn = 0;
  nSegs = maxHeight = 0;

  segments = NULL;

  texture = NULL;

  nSegs = -1;
  SetSize(n);
}

Panel::~Panel()
{
  delete []segments;
  
  delete texture;
}

void Panel::SetSize(int n)
{
  if(n != nSegs)
    {
      delete []segments;
  
      nSegs = n;
      maxHeight = 0;

      if(nSegs == 0)
    segments = NULL;
      else
    {
      segments = new Point3f[nSegs + 1];
      if(segments == NULL)
        {
          cerr << "Out of memory allocating space for curtain!" << endl;
          delete []segments;
          nSegs = 0;
        }
    }
    }
}

void Panel::Print()
{
  int i;

  cout << "Panel startColumn: " << startColumn << " nSegs: " << nSegs << 
    " maxHeight: " << maxHeight << "\n";

  for(i = 0; i <= nSegs; i++)
    {
      printvec(segments[i]); cout << " ";
    }
  cout << "\n";
}

//////////////////////////////////////////////////////////////////////
// Methods for ColorTable structure
//////////////////////////////////////////////////////////////////////

ColorTable::ColorTable(int nentries)
{
  colors = NULL;

  nColors = -1;
  SetSize(nentries);
}

ColorTable::~ColorTable()
{
  delete []colors;
}

void ColorTable::SetSize(int nentries)
{
  if(nentries < 256)        // assure at least 256 entries
    nentries = 256;

  if(nColors != nentries)
    {
      nColors = nentries;
      delete []colors;

      colors = new Point4ub[nColors];
      if(colors == NULL)
    {
      cerr << "Out of memory allocating space for curtain!" << endl;
      nColors = 0;
    }
    }

  // fill color table with opaque black as default
  for(int i = 0; i < nColors; i++)
    {
      colors[i][0] = colors[i][1] = colors[i][2] = 0;
      colors[i][3] = 255;
    }
}

void ColorTable::Print()
{
  cout << "ColorTable nColors: " << nColors << "\n";
  for(int i = 0; i < nColors; i++)
    {
      printcolor(colors[i]); cout << " ";
    }
  cout << endl;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Curtain::Curtain()
{
  SetTypeAndVersion("CURTAIN", 1, 0);
  SetKeywordTable(CurtainNames);

  lastStart = 0;

  dataType = INDEX;
  useLighting = GL_FALSE;

  xyonlyformat = GL_FALSE;
  zStep = zHeight = 0;

  nColumns = 0;
  curtain = NULL;

  nPanels = 0;
  panels = NULL;

  pathLength = hStep = 0;

  degradationFlag = NTH;

  minSamples = maxSamples = 0;
}

Curtain::~Curtain()
{
  delete []curtain;
  delete []panels;
}

//////////////////////////////////////////////////////////////////////
// Methods to load and save the curtain data from/to a file. 
//////////////////////////////////////////////////////////////////////

// convert 2 byte hex string to an integer
int fromhex(char *hexstr)
{
  int val = 0;
  int ch, digit;

  for(int i = 0; i < 2; i++)
    {
      ch = toupper(hexstr[i]);
      
      if(ch >= '0' && ch <= '9')
    digit = ch - '0';
      else if(ch >= 'A' && ch <= 'F')
    digit = 10 + (ch - 'A');
      else
    return -1;
      
      val = val * 16 + digit;
    }

  return val;
}

/*--------------------------------------------------------------------------*/
int Curtain::processField(int option, char **fields)
{
    int i = 1;

    switch(option)
    {
        case CLUT:
          dataType = INDEX;
          break;
        case RGB:
          dataType = COLOR3;
          break;
        case RGBA:
          dataType = COLOR4;
          break;
        case USE_LIGHTING:
          useLighting = GL_TRUE;
          break;
        case XY_ONLY:
          xyonlyformat = GL_TRUE;
          break;
        case NCOLS:
          nColumns = ConvertInt(i);
          break;
        case Z_STEP:
          zStep = ConvertFloat(i);
          break;
        default:
          return Object::processField(option, fields);
          break;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/
// documented in Object
int Curtain::readFrom(FILE *infile)
{
    char **fields = NULL;
    int nfields;
    int i;
    
    int colidx, stepidx;
    double x, y, z;
    int nvals =0;
    int nbytes =0;
    
    double minx, miny, minz;
    double maxx, maxy, maxz;
    int minn, maxn;
    
    double dx, dy;
    int idx;
    
    int ip;
    
    //parse header
    if (Object::readFrom(infile))
        return 1;
    
    if(zStep == 0)
    {
        cerr << "Z_STEP cannot be 0" << endl;
        return 1;
    }
    
    if(nColumns == 0)
    {
        cerr << "Curtain contains no columns. No data loaded.\n" << endl;
        return 1;
    }

    switch (dataType)
    {
        case INDEX:
          nbytes = 1;
          break;
        case COLOR3:
          nbytes = 3;
          break;
        case COLOR4:
          nbytes = 4;
          transparencyLevel = 1;
          break;
    }
    
    // allocate the data structure to hold column data
    curtain = new ColumnData[nColumns];
    location.resize(nColumns);
    if(curtain == NULL)
    {
        cerr << "Out of memory allocating space for curtain!" << endl;      
        return 1;
    }
    
    // prepare to keep track of various extents
    minx = miny = minz = 1.0e20;
    maxx = maxy = maxz = -1.0e20;
    minn = 1000000;
    maxn = 0;
    
    // read the column data lines from the file and copy to columns
    fields = NextLine();
    createProgress();
    for(colidx = 0; colidx < nColumns; colidx++, fields = NextLine())
    {
        if(fields == NULL)
        {
            cerr << "Error reading curtain column " << colidx + 1 << "!" <<
                endl;
            killProgress();
            return 1;
        }
        nfields = getNumFields();
        
        // the xy only data format does not include a height (it is assumed 0)
        // xy only format: x y nsamples hexdata
        // default format: x y z nsamples hexdata
        if((!xyonlyformat && (nfields < 4 || nfields > 5)) || 
            (xyonlyformat && (nfields < 3 || nfields > 4)))
        {
            cerr << "Curtain column " << colidx + 1 << 
                " has missing/extra fields, should be <x, y, z, n, data>!" <<
                endl;
            killProgress();
            return 1;
        }
        
        i = 0;          // initialize field index
        if (reversedCoordinates)
        {
            y = ConvertDouble(i);
            x = ConvertDouble(i);
        }
        else
        {
            x = ConvertDouble(i);
            y = ConvertDouble(i);
        }
        if(x < minx) minx = x;
        if(x > maxx) maxx = x;
        
        if(y < miny) miny = y;
        if(y > maxy) maxy = y;
        
        if(xyonlyformat)
            z = 0; 
        else
            z = ConvertFloat(i);
        if(z - nvals * zStep < minz) minz = z - nvals * zStep;
        if(z > maxz) maxz = z;
        
        nvals = ConvertInt(i);
        if(nvals < minn) minn = nvals;
        if(nvals > maxn) maxn = nvals;
        
        // if the hexdata field is missing, number or samples must be zero
        if((!xyonlyformat && nfields == 4) || (xyonlyformat && nfields == 3))
        {
            if(nvals != 0)
            {
                cerr << "Curtain column " << colidx + 1 << " missing data!" <<
                    endl;
                killProgress();
                return 1;
            }
            else
            {
                curtain[colidx].SetSize(0, dataType);
                SET_POINT(location[colidx], x, y, z);
            }
        }
        else    // hexdata is present
        {
            // must be right amount of data on input line
            if(int(strlen(fields[i])) != nvals * nbytes * 2)
            {
                cerr << "Curtain column " << colidx + 1 <<
                    " number of samples " << nvals << 
                    " does not match amount of data " << 
                    int(strlen(fields[i])) / (2 * nbytes) << "!" << endl;
                killProgress();
                return 1;
            }
            
            // create the new data column (bag it if no memory)
            curtain[colidx].SetSize(nvals, dataType);
            SET_POINT(location[colidx], x, y, z);
            if(curtain[colidx].nSteps != nvals)
            {
                killProgress();
                return 1;
            }
            
            updateProgress((90.0 * colidx)/nColumns);

            // decode the hex data for the column and store in the column
            for(stepidx = 0; stepidx < nvals; stepidx++)
            {
                idx = fromhex(&(fields[i][2 * nbytes * stepidx]));
                if(idx < 0)
                {
                    cerr << "Column " << colidx + 1 << 
                        " invalid hex number in data line!" << endl;
                    killProgress();
                    return 1;
                }
                switch(dataType)
                {
                case INDEX:
                    curtain[colidx].data[stepidx] = idx;
                    break;
                case COLOR3:
                case COLOR4:
                    curtain[colidx].colors[stepidx][0] = idx;
                    idx = fromhex(&(fields[i][2 * nbytes * stepidx + 2]));
                    curtain[colidx].colors[stepidx][1] = idx;
                    idx = fromhex(&(fields[i][2 * nbytes * stepidx + 4]));
                    curtain[colidx].colors[stepidx][2] = idx;
                    if(dataType == COLOR4)
                        idx = fromhex(&(fields[i][2 * nbytes * stepidx + 6]));
                    else
                        idx = 255;
                    curtain[colidx].colors[stepidx][3] = idx;
                    break;
                }
            }
        }
    }
    
    // record the bounds on the number of samples per column
    minSamples = minn;
    maxSamples = maxn;
    
    // record the height of the curtain at longest column
    zHeight = maxSamples * zStep;
    
    // expand the bounding box to fit the data
    bbox.w = minx; bbox.e = maxx;
    bbox.s = miny; bbox.n = maxy;
    bbox.b = minz; bbox.t = maxz;

    projectXY(location);

    // record the horizontal path length of the curtain,
    // extrapolating to get additional length at 2 extreme ends to account
    // for width of tile around data samples
    for(pathLength = 0, i = 0; i < nColumns - 1; i++)
    {
        dx = location[i + 1][X] - location[i][X];
        dy = location[i + 1][Y] - location[i][Y];
        pathLength += sqrt(sqr(dx) + sqr(dy));
    }
    dx = (location[0][X] - location[1][X]) / 2;
    dy = (location[0][Y] - location[1][Y]) / 2;
    pathLength += sqrt(sqr(dx) + sqr(dy));
    dx = (location[nColumns - 1][X] - location[nColumns-2][X]) / 2;
    dy = (location[nColumns - 1][Y] - location[nColumns-2][Y]) / 2;
    pathLength += sqrt(sqr(dx) + sqr(dy));
    
    // record the average horizontal distance between columns
    if(nColumns > 1)
        hStep = pathLength / (nColumns - 1);
    else
        hStep = 0;
    

    // Get the surface normal for each curtain column
    CalculateNormals();
    
    // build the panels for the polygonal surface for drawing
    BuildPanelSurfaces();
    
    // forget it if there were any memory allocation errors
    if(panels == NULL)
    {
        killProgress();
        return 1;
    }
    for(ip = 0; ip < nPanels; ip++)
    {
        if(panels[ip].nSegs == 0)
        {
            delete []panels;
            killProgress();
            return 1;
        }
    }
        
    // build the texture maps for the panels
    cout << "Creating panels..." << endl;
    for(ip = 0; ip < nPanels; ip++)
    {
        BuildPanelTexture(panels[ip]);
        updateProgress((10.0 * ip) / nPanels + 90.0);
    }
    
    //for(i = 0; i < nColumns - 1; i++){
    //  cout << "col " << i << " "; curtain[i].Print(); cout << endl;
    //}
    
    Print();
//  printf("\nDone loading.\n");
    
    editedFlag = GL_FALSE;
    killProgress();

    return 0;
}

/*--------------------------------------------------------------------------*/
int Curtain::writeTo(FILE *file)
{
  return 0;
}

/*--------------------------------------------------------------------------*/
void Curtain::Print()
{
  char const *kind[] = {"CLUT", "RGB", "RGBA"};

  cout << "Curtain: " << GetFilenameAsPath().filename() << "\n";
  cout << "  data type " << kind[dataType] << ", " << 
    nColumns << " columns, depth interval =  " << zStep << "\n";
  cout << "  path length = " << pathLength << 
    " mean horizontal interval = " << hStep << "\n";
//  cout << "  local origin = [" << geoRef[0] << ", " << geoRef[1] << ", " << 
//    geoRef[2] << "]" << "\n";
  cout << "  bounding box = [" << bbox.w << ", " << bbox.s << ", " << 
    bbox.b <<"]  [" << bbox.e << ", " << bbox.n << ", " << bbox.t << "]" << 
    endl;
}

//////////////////////////////////////////////////////////////////////
// Rendering Methods
//////////////////////////////////////////////////////////////////////

void Curtain::DrawLowRes(float degradation, Window3D *)
{
  int ip, panelSkip = 1;
  int skipcount, pow2;

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  // general shading preperation stuff
  glEnable(GL_LIGHTING);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glShadeModel(GL_FLAT);
  glColor4f(1, 1, 1, 1);

  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_ALWAYS, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
  if(useLighting)
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  else
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  // end of general shading preparation stuff

  // in degradation mode, need to skip columns
  if (degradationFlag != OFF)
  {
    pow2 = int(degradation * 8 - 0.001) + 1;    // [1, 2, 3, 4, 5, 6, 7, 8]
    skipcount = 1 << pow2;          // [2, 4, 8, 16, 32, 64, 128, 256]
  }
  else
    skipcount = 1;

  /*
  if (degradation > .9)
  {
      panelSkip = 2;
      //lastStart ^= 1;
      lastStart = 1;
  }
  else
      lastStart = 0;
  */

  // draw each texture mapped panel
  for(ip = lastStart; ip < nPanels; ip += panelSkip)
    DrawPanel(panels[ip], skipcount);

  // clean up shading stuff
  glDisable(GL_ALPHA_TEST);
//    glDisable(GL_BLEND); //commented out by mdp 8/31/2004 (we always blend)

  // restore default lighting parameters
  glPopAttrib();
}

int Curtain::DrawHighRes(int eye, Window3D *win)
{
  if (!doneFlag)
  {
      DrawLowRes(0,win);
      if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
        doneFlag = 1;
  }

  return 1;
}

// This function resets this object to start over drawing high res the next
//   time DrawHighRes is called.
//
void Curtain::ResetHighRes()
{
  doneFlag = 0;
}

//////////////////////////////////////////////////////////////////////
// Methods relating to building the surface or its texture maps
//////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------*/
void Curtain::useNewHeightExaggeration(GLfloat heightEx)
{
  Object::useNewHeightExaggeration(heightEx);
  //if (dem)
  //  CalculateNormals(heightEx);
}

/*---------------------------------------------------------------------------*/
double Curtain::GetHeightAt(double x, double y, Point3d coord, GLboolean secondary)
{
  return invalidHeight;
}

/*---------------------------------------------------------------------------*/
void Curtain::UpdateTexture(TextureMap *ptr)
{
  Clut *clutptr;
  const Point4ub *clr;
  int n;
  int i, c;
  int ip;

  // can't do anything about it if not a new CLUT
  // and can't accept it if we are in COLOR3 or COLOR4 mode, since they
  // do not use a CLUT
  if(ptr == NULL || ptr->Type() != CLUTTEXTURE || dataType != INDEX)
    return;

  clutptr = (Clut *)ptr;
  n = clutptr->GetNumEntries();
  clut.SetSize(n);

  for(i = 0; i < n; i++)
    {
      clr = clutptr->GetColorAtIndex(i);
      for(c = 0; c < 4; c++)
    clut.colors[i][c] = (*clr)[c];
    }

  // rebuild the texture maps to use the new colors
  createProgress("Texturing panels...");
  for(ip = 0; ip < nPanels; ip++)
  {
    //if ((ip % 5) == 0 && ip != 0)
    //  cout << "Texturing panel " << ip << endl;
    BuildPanelTexture(panels[ip]);
    updateProgress((100.0 * ip)/nPanels);
  }
  killProgress();
}

/*---------------------------------------------------------------------------*/
void Curtain::CalculateNormals()
{
  Point2f tangent;
  double len;
  int i, j;

  if(nColumns == 0)
    return;
  if(nColumns == 1)
    {
      curtain[0].normal[0] = 1;
      curtain[0].normal[1] = curtain[0].normal[2] = 0;
      return;
    }

  for(i = 0; i < nColumns; i++)
    {
      if(i == 0)
    for(j = X; j < Z; j++)
      tangent[j] = location[i + 1][j] - location[i][j];
      else if(i == nColumns - 1)
    for(j = X; j < Z; j++)
      tangent[j] = location[i][j] - location[i - 1][j];
      else
    for(j = X; j < Z; j++)
      tangent[j] = location[i + 1][j] - location[i - 1][j];
      len = sqrt(sqr(tangent[0]) + sqr(tangent[1]));

      curtain[i].normal[0] = (float)(-tangent[1] / len);
      curtain[i].normal[1] = (float)(tangent[0] / len);
      curtain[i].normal[2] = 0;

      //cout << "nrml: [" << curtain[i].normal[0] << " " << 
      //  curtain[i].normal[1] << "]" << endl;
    }
}

/*---------------------------------------------------------------------------*/
// build a surface around the data points. Conceptually, each
// data point lies at the center of a grid cell.
// A segment is created, centered around each data column,
// that extends to the vertical midline between the two columns.
// x & y coordinates are interpolated between columns (or extrapolated at
// the two extreme ends. The left-hand z coordinate is set to the column
// height, and is the z coordinate that should be used to draw the surface
void Curtain::BuildSegments(Panel &panel, int startcol)
{
  int is, istart, imax;     // segment indices
  int ic;           // column index
  int j;            // component (0 = x, 1 = y, 2 = z)
  double delta;

  panel.startColumn = startcol; // remember the first data column for this seg

  istart = 0;           // starting segment vertex index
  imax = panel.nSegs;       // ending segment vertex index

  // first column in curtain must be extrapolated back
  if(panel.startColumn == 0)
    {
      istart = 1;
      startcol = 1;
      for(j = X; j < Z; j++)    // extrapolate ahead of first column
    {
      delta = location[0][j] - location[1][j];
      panel.segments[0][j] = (float)(location[0][j] + delta / 2);
    }
      panel.segments[0][Z] = location[0][Z];
    }

  // interpolate between internal columns
  for(is = istart, ic = startcol; ic < nColumns && is <= imax; is++, ic++)
    {
      for(j = X; j < Z; j++)
    panel.segments[is][j] = 
      (location[ic][j] + location[ic - 1][j]) / 2;
      panel.segments[is][Z] = location[ic][Z];
    }

  // last column in curtain must be extrapolated forward
  if(ic == nColumns)
    {
      for(j = X; j < Z; j++)    // extrapolate beyond last column
    {
      delta = location[nColumns - 1][j] - location[nColumns - 2][j];
      panel.segments[panel.nSegs][j] = (float)(
        location[nColumns - 1][j] + delta / 2);
    }
      panel.segments[panel.nSegs][Z] = location[nColumns - 1][Z];
    }

  //panel.Print(); cout << endl;
}

/*---------------------------------------------------------------------------*/
void Curtain::BuildPanelSurfaces()
{
  int ip;       // panel index
  int ic;       // column index

  if(nColumns == 0)
    return;

  // build enough panels to cover the entire data set
  nPanels = (nColumns + MAX_PANEL_WIDTH - 1) / MAX_PANEL_WIDTH;
  panels = new Panel[nPanels];
  if(panels == NULL)
    {
      cerr << "Out of memory allocating space for curtain!" << endl;      
      nPanels = 0;
      return; 
    }

  for(ip = 0; ip < nPanels - 1; ip++)
    panels[ip].SetSize(MAX_PANEL_WIDTH);
  panels[ip].SetSize(nColumns - (nPanels - 1) * MAX_PANEL_WIDTH);

  // now, for each panel, build its segments, which describe its geometry
  for(ip = 0, ic = 0; ip < nPanels; ic += panels[ip].nSegs, ip++)
    BuildSegments(panels[ip], ic);
}

/*---------------------------------------------------------------------------*/
void Curtain::BuildPanelTexture(Panel &panel)
{
    int w, h;
    int row, col;
    int ic, i;
    Point4ub *image, *colors;
    int nSteps;
    unsigned char *data;
    
    // get the maximum number of samples in a column of this panel
    if (panel.maxHeight == 0) //changed to condition rather than always setting by mdp 12/21/2001
    {
        for(ic = panel.startColumn, i = 0; i < panel.nSegs; ic++, i++)
            if(curtain[ic].nSteps > panel.maxHeight)
                panel.maxHeight = curtain[ic].nSteps;
    }
    
    // get texture width and height as even powers of 2
    for(w = 1; w < panel.nSegs; w <<= 1);
    for(h = 1; h < panel.maxHeight; h <<= 1);
    
    if(panel.texture == NULL)
    {
        panel.texture = new PanelTexture(w, h);
        if(panel.texture == NULL)
        {
            cerr << "Out of memory allocating space for curtain!" << endl;
            return;
        }
    }
    else
        panel.texture->SetSize(w, h);
    
    if(panel.texture->image == NULL)
        return;
    image = panel.texture->image;
    
    if (dataType == INDEX) //CLUT
    {
        for(ic = panel.startColumn, col = 0; col < panel.nSegs; ic++, col++)
        {
            nSteps = curtain[ic].nSteps;
            data = curtain[ic].data;
            colors = clut.colors;
            for(row = 0; row < nSteps; row++)
                *(int *)(image[row * w]) = *(int*)(colors[data[row]]); //copy 4 bytes
            image++; //takes care of col calculation
        }
    }
    else //RGB, etc.
    {
        for(ic = panel.startColumn, col = 0; col < panel.nSegs; ic++, col++)
        {
            nSteps = curtain[ic].nSteps;
            colors = curtain[ic].colors;
            for(row = 0; row < nSteps; row++)
                *(int *)(image[row * w]) = *(int*)(colors[row]); //copy 4 bytes
            image++; //takes care of col calculation
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, panel.texture->textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    // nearby, we want to see the full detail -- do not interpolate!!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // far away, we want to avoid sparkling and other aliasing effects
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    // if the graphics card supports it, use only 16 bits per texel
//  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB5_A1, w, h, 
//      GL_RGBA, GL_UNSIGNED_BYTE, panel.texture->image);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h,
        GL_RGBA, GL_UNSIGNED_BYTE, panel.texture->image);
}

/*---------------------------------------------------------------------------*/
void Curtain::DrawPanel(const Panel &panel, int skipcount) const
{
  int ic;       // index into data columns
  int is, isend;        // index into segment

  float x1, x2, y1, y2;
  float ztop, zbottom;
  double ufraction = 1;
  float u1, u2;
  float vbottom = 1;
  int halfskip;

  // make sure that we at least advance by 1 column per strip
    if(skipcount < 2)
        skipcount = 1;

  // turn on texture mapping if there is a texture
    if(panel.texture != NULL)   // make sure we have a texture on panel
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, panel.texture->textureID);

        // multiplier for u coordinate of panel
        ufraction = 1.0 / panel.texture->width;
  }

  halfskip = skipcount / 2;
    for(ic = panel.startColumn, is = 0; is < panel.nSegs; 
      ic += skipcount, is += skipcount)
    {
        glBegin(GL_QUADS);

        isend = is + skipcount;
        if(isend > panel.nSegs)
        {
            isend = panel.nSegs - 1;
            skipcount = panel.nSegs - is;
            halfskip = skipcount / 2;
        }

        glNormal3f(curtain[ic].normal[X], curtain[ic].normal[Y], curtain[ic].normal[Z]);
    
        u1 = (float)(is * ufraction);
        u2 = (float)(isend * ufraction);

        if(panel.texture != NULL)   // make sure we have a texture on panel
            vbottom = float(curtain[ic].nSteps) / float(panel.texture->height);

        // horizontal coordinates from extreme ends
        x1 = float(panel.segments[is][X]);
        y1 = float(panel.segments[is][Y]);
        x2 = float(panel.segments[isend][X]);
        y2 = float(panel.segments[isend][Y]);

        // vertical coordinates from the middle
        ztop = float(panel.segments[is + halfskip][Z] + zStep / 2);
        zbottom = (float)(ztop - curtain[ic + halfskip].nSteps * zStep);

        glTexCoord2f(u1, 0);
        glVertex3f(x1, y1, ztop);
        glTexCoord2f(u1, vbottom);
        glVertex3f(x1, y1, zbottom);
        glTexCoord2f(u2, vbottom);
        glVertex3f(x2, y2, zbottom);
        glTexCoord2f(u2, 0);
        glVertex3f(x2, y2, ztop);

        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}
