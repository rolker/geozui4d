#include "../boost_python.h"
#include <gz4d/Scale.h>

using namespace boost::python;

template<typename T> void WrapScale(char const *name)
{
    boost::python::class_<gz4d::Scale<T>, boost::python::bases<gz4d::Vector<T,3> > >(name)
            .def(boost::python::init<T>())
            .def(boost::python::init<gz4d::Vector<T,3> const&>())
            .def(init<T,T,T>())
            .def("GetMatrix", &gz4d::Scale<T>::GetMatrix)
            .def("GetInverseMatrix", &gz4d::Scale<T>::GetInverseMatrix)
                    ;
}

BOOST_PYTHON_MODULE(Scale)
{
    WrapScale<float>("Scalef");
    WrapScale<double>("Scaled");
}
