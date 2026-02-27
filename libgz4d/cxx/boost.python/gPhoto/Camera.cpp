#include "../boost_python.h"
#include <gz4d/gPhoto/Camera.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Camera)
{

    class_<gz4d::gphoto::Camera>("Camera")
            .def("Capture", &gz4d::gphoto::Camera::Capture)
            ;

}
