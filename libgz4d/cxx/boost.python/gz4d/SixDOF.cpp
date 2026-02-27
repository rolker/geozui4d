#include "../boost_python.h"
#include <gz4d/SixDOF.h>

using namespace boost::python;

template<typename T> gz4d::Point<T> call_p(gz4d::SixDOF<T> &s, gz4d::Point<T> const &p)
{
    return s(p);
}

template<typename T> gz4d::SixDOF<T> call_s(gz4d::SixDOF<T> &s, gz4d::SixDOF<T> const &os)
{
    return s(os);
}


template<typename T> void WrapSixDOF(char const *name)
{
    boost::python::class_<gz4d::SixDOF<T>  >(name)
            .def_readwrite("position", &gz4d::SixDOF<T>::position)
            .def_readwrite("orientation", &gz4d::SixDOF<T>::orientation)
            .def("reverse", &gz4d::SixDOF<T>::reverse)
            .def("__call__", &call_p<T>)
            .def("__call__", &call_s<T>)
                    ;
}

BOOST_PYTHON_MODULE(SixDOF)
{
    WrapSixDOF<float>("SixDOFf");
    WrapSixDOF<double>("SixDOFd");
}
