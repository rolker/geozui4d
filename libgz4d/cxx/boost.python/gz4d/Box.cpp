#include "../boost_python.h"
#include <gz4d/Box.h>

using namespace boost::python;

template<typename T> inline void WrapBox(char const *name)
{
    class_<gz4d::Box<T> >(name,init<>())
            .def(init<T const &, T const &>())
            .def("getMin", &gz4d::Box<T>::getMin, return_value_policy<copy_const_reference>())
            .def("getMax", &gz4d::Box<T>::getMax, return_value_policy<copy_const_reference>())
            .def("setMin", &gz4d::Box<T>::setMin)
            .def("setMax", &gz4d::Box<T>::setMax)
            .def("getSizes", &gz4d::Box<T>::getSizes)
                    ;
}


BOOST_PYTHON_MODULE(Box)
{
    WrapBox<gz4d::Vector<float,2> >("Box2f");
    WrapBox<gz4d::Vector<double,2> >("Box2d");
    WrapBox<gz4d::Vector<float,3> >("Box3f");
    WrapBox<gz4d::Vector<double,3> >("Box3d");
}

