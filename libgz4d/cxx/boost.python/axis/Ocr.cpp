#include "../boost_python.h"
#include <gz4d/axis/ocr.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Ocr)
{
    def("ocr",&gz4d::axis::ocr);
}
