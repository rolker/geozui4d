// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/glo/Texture.h>

using namespace boost::python;

void (gz4d::glo::Texture::*ImageRGB8)(gz4d::Raster<gz4d::Pixel<gz4d::RGB8> > const &, GLint) = &gz4d::glo::Texture::Image;
void (gz4d::glo::Texture::*ImageRGBA8)(gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> > const &, GLint) = &gz4d::glo::Texture::Image;

void (gz4d::glo::Texture::*Bind)() = &gz4d::glo::Texture::Bind;
void (gz4d::glo::Texture::*BindT)(GLenum) = &gz4d::glo::Texture::Bind;
void (gz4d::glo::Texture::*Unbind)() = &gz4d::glo::Texture::Unbind;
void (gz4d::glo::Texture::*UnbindT)(GLenum) = &gz4d::glo::Texture::Unbind;
void (gz4d::glo::Texture::*Parameteri)(GLenum, GLint) = &gz4d::glo::Texture::Parameter;
void (gz4d::glo::Texture::*Parameterf)(GLenum, GLfloat) = &gz4d::glo::Texture::Parameter;

template <typename T> void Parameter(gz4d::glo::Texture &t, GLenum param, tuple values)
{
    std::vector<T> v;

    for(int i = 0; i < len(values); ++i)
        v.push_back(extract<T>(values[i]));

    t.Parameter(param,v);
}

template <typename T> void TextureImage(gz4d::glo::Texture &t, std::shared_ptr<T> r)
{
    t.Image(*r);
}

template <typename T> void TextureSubImage(gz4d::glo::Texture &t, std::shared_ptr<T> r)
{
    t.SubImage(*r);
}

BOOST_PYTHON_MODULE(Texture)
{
    class_<gz4d::glo::Texture, bases<>, std::shared_ptr<gz4d::glo::Texture> >("Texture", init<gz4d::glo::Context::Ptr>())
        .def(init<gz4d::glo::Context::Ptr,GLenum>())
        .def(init<gz4d::glo::Context::Ptr,GLenum,GLuint>())
        .def("Image",ImageRGB8)
        .def("Image",ImageRGBA8)
        .def("Image",TextureImage<gz4d::Raster<gz4d::Pixel<gz4d::RGB8> > >)
        .def("Image",TextureImage<gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> > >)
        .def("SubImage",TextureSubImage<gz4d::Raster<gz4d::Pixel<gz4d::RGB8> > >)
        .def("SubImage",TextureSubImage<gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> > >)
        .def("Bind",Bind)
        .def("Bind",BindT)
        .def("Unbind",Unbind)
        .def("Unbind",UnbindT)
        .def("MinFilter",&gz4d::glo::Texture::MinFilter)
        .def("MagFilter",&gz4d::glo::Texture::MagFilter)
        .def("WrapS",&gz4d::glo::Texture::WrapS)
        .def("WrapT",&gz4d::glo::Texture::WrapT)
        .def("Parameter",Parameterf)
        .def("Parameter",Parameteri)
        .def("Parameterfv",Parameter<GLfloat>)
        .def("Parameteriv",Parameter<GLint>)
        ;
}

