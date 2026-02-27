#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DImageTexture.h>

#include <filesystem>
#include <gz4d/X3D.h>
#include <gz4d/image/io.h>
#include <iostream>

gz4d::X3DImageTexture::X3DImageTexture(gz4d::glo::Context::Ptr c):texture(c)
{
    typeName = "ImageTexture";
}

void gz4d::X3DImageTexture::Load(const boost::property_tree::ptree& x3d, gz4d::X3D* root)
{
    LoadArray( x3d, "url", url);
    X3DTexture2DNode::Load( x3d, root);

    
    std::filesystem::path fn(root->getFilename());

    gz4d::image::Info i = gz4d::image::info((fn.parent_path()/url[0]).string());
    if(L8()(i.format)||L16()(i.format))
        image_l.Load((fn.parent_path()/url[0]).string());
    else if(LA8()(i.format)||LA16()(i.format))
             image_la.Load((fn.parent_path()/url[0]).string());
    else if(RGB8()(i.format)||RGB16()(i.format))
        image_rgb.Load((fn.parent_path()/url[0]).string());
    else if(RGBA8()(i.format)||RGBA16()(i.format))
        image_rgba.Load((fn.parent_path()/url[0]).string());
    
    //std::string fn(url[0].substr(1,url[0].size()-2));
    //image.Load(root->getFilename().branch_path()/url[0]); //&&&we need to loop through until we find one, rather than only try first one
}

void gz4d::X3DImageTexture::Draw()
{
    glEnable(GL_TEXTURE_2D);
    if(!texture)
    {
        texture.Bind(GL_TEXTURE_2D);
        if(image_l)
            texture.Image(image_l.GetRaster());
        else if(image_la)
            texture.Image(image_la.GetRaster());
        else if(image_rgb)
            texture.Image(image_rgb.GetRaster());
        else if(image_rgba)
            texture.Image(image_rgba.GetRaster());
        if(repeatS)
            texture.WrapS(GL_REPEAT);
        else
            texture.WrapS(GL_CLAMP);
        if(repeatT)
            texture.WrapT(GL_REPEAT);
        else
            texture.WrapT(GL_CLAMP);
        texture.MinFilter(GL_LINEAR);
        texture.MagFilter(GL_NEAREST);
    }
    else
    {
        texture.Bind();
    }

}

