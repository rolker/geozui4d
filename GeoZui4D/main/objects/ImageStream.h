#ifndef IMAGESTREAM_H
#define IMAGESTREAM_H

#include "RasterStream.h"
#include "engine/Object.h"
#include "engine/TextureMaps/TextureMap.h"
#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>
#include <gz4d/glo/Texture.h>

class ImageStream: public RasterStream<gz4d::Raster<gz4d::Pixel> >, public Object, public TextureMap
{
    public:
        ImageStream();
        void Idle(double time);
        DERIVE_OBJECT(ImageStream);
        void DrawLowRes(float degradation, Window3D *win);
        virtual int perform(int argc, const char **argv, Tcl_Interp *interp);

        void Enable();
        void Disable();

        int Load(){return 1;}
        void Init(){}
        int Save(std::string const &fn = bfs::path()){return 1;}
        void Enable(const Point3d offset){Enable();}
        void Build(){}
        void Destroy(){};

        void CreateDefault(){}

        void Print() const {}
        void Draw(bool asQuad = false) {}
        void Display(Tcl_Interp *interp, float min , float max, char * target){}
        void InitDims(){}

    private:
        gz4d::Texture texture;
        gz4d::Raster<gz4d::Pixel>::Ptr currentImage;
};

#endif
