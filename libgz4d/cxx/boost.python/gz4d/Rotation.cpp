#include "../boost_python.h"
#include <gz4d/Rotation.h>

using namespace boost::python;

template<typename T> inline void WrapRotation(char const *name)
{
    class_<gz4d::Rotation<T>, boost::python::bases<boost::math::quaternion<T> > >(name,init<>())
            .def(init<gz4d::Angle<T,gz4d::Radian>,gz4d::Point<T> >())
            .def(init<boost::math::quaternion<T> >())
            .def(init<gz4d::Point<T>, gz4d::Point<T> >())
            .def("GetMatrix", &gz4d::Rotation<T>::GetMatrix)
            .def("GetInverseMatrix", &gz4d::Rotation<T>::GetInverseMatrix)
            .def("Inverse", &gz4d::Rotation<T>::Inverse)
            .def("__call__", &gz4d::Rotation<T>::operator())
        ;
}

BOOST_PYTHON_MODULE(Rotation)
{
    WrapRotation<double>("Rotationd");
    WrapRotation<float>("Rotationf");
}

