/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// TextureScreen.h
/*---------------------------------------------------------------------------*/
/** 
 *	This is a screen that can be placed in a scene to display a texture
 *
 *	Created by rja Jan 15, 2002
 */
/*---------------------------------------------------------------------------*/
class TextureScreen;

#ifndef SCREEN_H
#define SCREEN_H

#include "engine/Object.h"
#include "interactors/Pickable.h"
#include <gz4d/Image.h>
#include <gz4d/glo/Texture.h>
#include <gz4d/TimeSequence.h>
//#include "objects/RasterStream.h"
#include "engine/TextureMaps/TextureMap.h"
#include "engine/QuadTreeObject.h"

class PosableStateLock;

class TextureScreen : public QuadTreeObject
{
    public:
        TextureScreen();
        ~TextureScreen();
        DERIVE_OBJECT(TextureScreen);

	//File-related functions
        int readFrom(FILE *file);
        int writeTo(FILE *file);
        Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
        void SaveXML(FILE * out,const char * path);

	//Display-related functions
        void Idle(double time);
        void DrawLowRes(float degradation, Window3D *win);

        virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
        virtual int list(Tcl_Interp *interp);

    //Commands that apply right away
        void setScale(float newScale);
        void setLineColor(float *newColor) {borderColor[X] = newColor[X]; borderColor[Y] = newColor[Y]; borderColor[Z] = newColor[Z];}
        void setLocationOffset(gz4d::Point<double> const &pos);
        void setColorDepth(int nybblesPerColor = 2, bool transparency = true);
        void Fuse(Object *other);
        void applyChanges();
        bool loadTexture(fs::path fname);
        void buildTexture();

    //Commands that require a call to applyChanges()
        void setHeight(float newHeight) {screen_height = newHeight;}
        void setWidth(float newWidth) {screen_width = newWidth;}
        void setYaw(float newYaw) {orient.axis[Yaw] = newYaw;}
        void setPitch(float newPitch) {orient.axis[Pitch] = newPitch;}
        void setRoll(float newRoll) {orient.axis[Roll] = newRoll;}
        void overrideQuality(float newQuality) {quality = newQuality;}

        float getQuality() {return quality;}
        float getHeight() {return screen_height;}
        float getWidth() {return screen_width;}

    //QuadTreeObject methods
        virtual QuadTreeObject *offsetCopy(float xOffset, float yOffset, float scaleFactor = 1.0);
        virtual bool setXMLattribute(std::string const &name, std::string const &value,
                                     std::string const &path);
        virtual bool readContents();
        virtual bool combineChildrenOfTile();
        virtual double getCellsWide() {return (leftTexture)? (double)leftTexture->GetWidth() : 0.0;}
        virtual double getCellsTall() {return (leftTexture)? (double)leftTexture->GetHeight() : 0.0;}
        virtual double getPixelSize() {return pixelSize;}
        virtual void actAsTemplate();
        /*---Event notification---*/
        static unsigned int textureScreenEvent();

        int pickAction (Pickable::PickType pickType, InputInfo* inputInfo, GLuint *names, int hits);

    protected:
        void Play();
        void Stop();
        void Pause();
        void Loop(GLboolean repeat = GL_TRUE);

        bool setTextureFile(fs::path fname, std::shared_ptr<TextureMap> &texture);
        void adjustSize(std::shared_ptr<TextureMap> &texture);
        bool readWorldFile(std::shared_ptr<TextureMap> &texture, fs::path fname);
        bool loadLeftTexture();
        void generatePanelCoordinates();

        TextureMap::ColorFormat convertFormat();

        static int quadCornerIndex[4];
        Point3d *corners;
        Point2f *texCoords;
        int numPanels;
        float angleOffset;

        std::shared_ptr<TextureMap> leftTexture;
        std::shared_ptr<TextureMap> rightTexture;
        fs::path leftTextureName;
        int numNybblesPerColor;
        bool useTransparency;
        bool generatedInternally;

	/**
         * This is the filename of the distortion grid netCDF file.
         */
        fs::path dgFilename;

	/**
         * This is the distortion grid netCDF file.
         */
        gz4d::netcdf::File dgNcFile;

	/**
         * This is the distortion grid netCDF variable.
         */
        gz4d::netcdf::Var dgNcVar;


        typedef gz4d::Vector<float,2> TexCoordType;
        typedef gz4d::Raster<TexCoordType > TexCoordRaster;

        TexCoordRaster dgTextureCoordinates;

        typedef gz4d::Vector<float,3> VertexPosType;
        typedef gz4d::Raster<VertexPosType> VertexRaster;

        std::vector<VertexRaster> dgVertexCoordinates;

        typedef gz4d::TimeSequence<std::size_t> IndexTimeSequence;
        IndexTimeSequence dgTimes;
        IndexTimeSequence::const_iterator currentDGTimeStep;

        std::vector<GLsizei> dgElementCounts; ///< Number of elements per line for dg rendering
        std::vector< std::vector<GLuint> > dgIndecies;
        std::vector<const GLvoid *> dgIndexPointers;

        std::vector<GLuint> dgOutlineIndecies;

	/**
         * This is the width of the distortion grid.
         */
        int dgWidth;

	/**
         * This is the height of the distortion grid.
         */
        int dgHeight;


        static const EventType *textureScreenInputType;

        bool load;
        float screen_width, screen_height;
        float eastScale, northScale;
        float pixelSize;
        OrientationInfo orient;
        float quality;
        float childWidth, childHeight;
	
        bool seeTru;

        double borderColor[3];
        float borderWidth;
        double bgColor[3];

        float fusionTransparency;
        
        //gz4d::ImageSequence imageSequence;
        //gz4d::ImageSequence::iterator currentImage;
        //gz4d::Texture sequenceTexture;
        //ImageStream imageStream;
        //gz4d::Thread imageStreamThread;
        //RasterStream<gz4d::Raster<gz4d::Pixel> > imageSequence;
        //gz4d::Raster<gz4d::Pixel>::Ptr currentImage;

        static float emphasisFactor;

        bool fuseable;
        bool showBorder;
        bool twoFaced;
        bool initialized;

        enum ScreenParams {SCREEN_WIDTH = FirstObjectKeyword, 
            SCREEN_HEIGHT, YAW, PITCH, ROLL, LOAD, PANELS, PANEL_ANGLE, BORDER,
            TEXTURE_FILE};

            void init();
	/**
             * This method initializes the distortion grid variables.  If the
             * distortion grid filename is not null, then the first distortion grid
             * in the file is loaded.
         */
            void initDistortionGrid();

	/**
             * This method draws the distortion grid.
         */
            void drawDistortionGrid(std::shared_ptr<TextureMap> &texture);

	/**
             * This method draws the outline of the distortion grid.
         */
            void drawDistortionGridOutline();
};

#endif
