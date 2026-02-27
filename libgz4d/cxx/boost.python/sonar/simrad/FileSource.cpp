#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/file/FileSource.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(FileSource)
{
    class_<gz4d::Simrad::FileSource, bases<>, std::shared_ptr<gz4d::Simrad::FileSource>, boost::noncopyable>("FileSource", init<std::string const &>())
            .def("__str__",&gz4d::Simrad::FileSource::GetInfo)
            .def("ProcessNextDatagram",&gz4d::Simrad::FileSource::ProcessNextDatagram)
            .def("AddNavOutQueue",&gz4d::Simrad::FileSource::AddOutQueue<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >)
            ;
    //implicitly_convertible<std::string,boost::filesystem::path>();
}
