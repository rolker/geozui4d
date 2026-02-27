#include "../boost_python.h"
#include <gz4d/glo/ScaleImage.h>

using namespace boost::python;


template <typename T> typename std::shared_ptr<gz4d::Raster<T> > sclimg(typename std::shared_ptr<gz4d::Raster<T> >& r, std::size_t w, std::size_t h)
{
    return gz4d::glo::ScaleImage(r,w,h);
}

BOOST_PYTHON_MODULE(ScaleImage)
{
    def("ScaleImage",&sclimg<gz4d::Pixel<gz4d::RGBA8> >);
    def("ScaleImage",&sclimg<gz4d::Pixel<gz4d::RGB8> >);
}

