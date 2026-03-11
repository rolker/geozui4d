// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "engine/Scene3D.h"
#include "TextureMap.h"
#include "engine/InputMediator.h"
//#include "MovieScreen.h"
//#include "MovieMap.h"
#include <filesystem>
#include "engine/TextureMaps/TextureMap.h"
#include "engine/TextureMaps/Clut.h"
#include "engine/TextureMaps/ImageMap.h"
#include "engine/TextureMaps/LatTexture.h"
#include "engine/TextureMaps/Grid.h"

float TextureMap::transparentBlendThreshold = 0.0f; //added by mdp 9/17/2001

TextureMap::TextureMap()
{
  built = 0;
  textureID = 0;
  twidth = theight = 0;
  iwidth = iheight = 0;
  hOffsetScale = vOffsetScale = hsize = vsize = 0;
  loaded = 0;
  applyMode = GL_DECAL;

  widthRatio = heightRatio = 1.0;
  setColorFormat();
}

TextureMap::~TextureMap()
{
    if (textureID)
        glDeleteTextures(1, &textureID);
}

int TextureMap::Reload()
{
  Destroy();

  return Load();
}


int TextureMap::Type()
{
  return type;
}

void TextureMap::EnsureBuilt()
{
        if (!built)
                Build();
}

/*---------------------------------------------------------------------------*/
void TextureMap::setTransparentBlendThreshold(float newValue)
{
	transparentBlendThreshold = newValue;
	globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
}


/*---------------------------------------------------------------------------*/
// Gloabal function that creates and returns the proper texture based on 
// filename.    rja, Feb 1, 2002
/*---------------------------------------------------------------------------*/


std::shared_ptr<TextureMap> CreateTexture(std::string const &fname)
{
    std::string tag[] = {"clut", "grid", "lat", "tfw", "jgw",  "jpg", "tif", "mgw", "mpg", "mp3", "nc", "png", "pgw", ""};
    unsigned int j;

    std::shared_ptr<TextureMap> ret;

    if(fname.empty()) return ret;

    j = 0;
    while(!tag[j].empty())
    {
        if(fs::path(fname).extension().string() == '.'+tag[j])
            switch(j)
            {
                case 0:
                    ret = std::shared_ptr<TextureMap>(new Clut);
                    return ret;
                case 1:
                    ret = std::shared_ptr<TextureMap>(new Grid);
                    return ret;
                case 2:
                    ret = std::shared_ptr<TextureMap>(new LatTexture);
                    return ret;
                case 3:
                case 4:
                case 5:
                case 6:
                    ret = std::shared_ptr<TextureMap>(new ImageMap);
                    return ret;
        /*			case 7:
                case 8:
                case 9:
                    return new MovieMap;*/
                case 10:
                case 11:
                case 12:
                    ret = std::shared_ptr<TextureMap>(new ImageMap);
                    return ret;
                default:
                    break;
            }
        j++;
    }
    return ret;
}

float TextureMap::GetTexT(float t)
{
	return t*heightRatio;
}

void TextureMap::beginTextureOffset(float x, float y, float z, float scale)
{
	GLint matrixMode;

	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glTranslatef(x / hOffsetScale, y / vOffsetScale, z);
	glScalef(scale, scale, scale);
	glMatrixMode(matrixMode);
}

void TextureMap::endTextureOffset()
{
	GLint matrixMode;

	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(matrixMode);
}

void TextureMap::SaveXML(FILE * out, std::string path)
{
	fprintf(out,"<Object ");
    if(!GetFilename().empty())
		fprintf(out,"filename=\"%s\" ",(fs::path(path)/GetFilenameAsPath()).string().c_str());
	if(!GetLabel().empty())
		fprintf(out,"label=\"%s\" ",GetLabel().c_str());
	fprintf(out,"/>\n");
}

/********************************************************************/
void TextureMap::setColorFormat(ColorFormat newFormat)
{
    tColorFormat = newFormat;
    switch (newFormat) {
        case RGBA16:
            colorFormat = GL_RGBA16;
            break;
        case RGBA8:
            colorFormat = GL_RGBA8;
            break;
        case RGBA4:
            colorFormat = GL_RGBA4;
            break;
        case RGB5:
            colorFormat = GL_RGB5_A1;
            break;
        case RGB_byte:
            colorFormat = GL_R3_G3_B2;
            break;
        default:
            colorFormat = GL_RGBA8;
            tColorFormat = RGBA8;
            break;
    }
}

/********************************************************************/
void TextureMap::SetDims(int width, int height)
{
    iwidth = width;
    iheight = height;
}

