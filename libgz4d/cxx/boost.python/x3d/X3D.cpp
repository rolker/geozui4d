#include "../boost_python.h"
#include <gz4d/X3D.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(X3D)
{
    class_<gz4d::X3D, bases<>, std::shared_ptr<gz4d::X3D> >("X3D", init<std::shared_ptr<gz4d::glo::Context> >())
            .def("Load", &gz4d::X3D::Load)
            ;

}
