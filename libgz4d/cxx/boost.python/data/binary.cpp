#include "../boost_python.h"
#include <gz4d/data/Binary.h>
#include <gz4d/socket/UDPSender.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/thread/Producer.h>
#include <gz4d/nav/Fix.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(binary)
{

    class_<gz4d::binary::FloatArray>("FloatArray")
            .def_readwrite("size", &gz4d::binary::FloatArray::size)
            //.def_readwrite("values", &gz4d::binary::FloatArray::values)
            ;

    class_<gz4d::binary::Packet>("Packet")
            .def_readwrite("timestamp", &gz4d::binary::Packet::timestamp)
            ;

    gz4d::WrapThreadedQueue<std::shared_ptr<gz4d::binary::Packet> >("PacketThreadedQueue");
    gz4d::WrapProducer<std::shared_ptr<gz4d::binary::Packet> >("PacketProducer");

    class_<gz4d::binary::Navigation::Packet, bases<gz4d::binary::Packet> >("Navigation_Packet")
            .def_readwrite("navChannel", &gz4d::binary::Navigation::Packet::navChannel)
            ;

    class_<gz4d::binary::Navigation::Position6DOF::Packet, bases<gz4d::binary::Navigation::Packet> >("Navigation_Position6DOF_Packet")
            ;

    class_<gz4d::binary::Navigation::Position6DOF::Geographic, bases<gz4d::binary::Navigation::Position6DOF::Packet> >("Navigation_Position6DOF_Geographic")
            .def_readwrite("latitude", &gz4d::binary::Navigation::Position6DOF::Geographic::latitude)
            .def_readwrite("longitude", &gz4d::binary::Navigation::Position6DOF::Geographic::longitude)
            .def_readwrite("z", &gz4d::binary::Navigation::Position6DOF::Geographic::z)
            .def_readwrite("heading", &gz4d::binary::Navigation::Position6DOF::Geographic::heading)
            .def_readwrite("pitch", &gz4d::binary::Navigation::Position6DOF::Geographic::pitch)
            .def_readwrite("roll", &gz4d::binary::Navigation::Position6DOF::Geographic::roll)
            ;

    class_<gz4d::binary::Sonar::Packet, bases<gz4d::binary::Packet> >("Sonar_Packet")
            ;

    class_<gz4d::binary::Sonar::Channel::Packet, bases<gz4d::binary::Sonar::Packet> >("Sonar_Channel_Packet")
            ;

    class_<gz4d::binary::Sonar::Channel::BottomDetection, bases<gz4d::binary::Sonar::Channel::Packet> >("Sonar_Channel_BottomDetection")
            .def_readwrite("latitude", &gz4d::binary::Sonar::Channel::BottomDetection::latitude)
            .def_readwrite("longitude", &gz4d::binary::Sonar::Channel::BottomDetection::longitude)
            .def_readwrite("x", &gz4d::binary::Sonar::Channel::BottomDetection::x)
            .def_readwrite("y", &gz4d::binary::Sonar::Channel::BottomDetection::y)
            .def_readwrite("z", &gz4d::binary::Sonar::Channel::BottomDetection::z)
            //.def_readwrite("Ss", &gz4d::binary::Sonar::Channel::BottomDetection::Ss)
            //.def_readwrite("horizontal_variance", &gz4d::binary::Sonar::Channel::BottomDetection::horizontal_variance)
            //.def_readwrite("vertical_variance", &gz4d::binary::Sonar::Channel::BottomDetection::vertical_variance)
            ;
    
    class_<gz4d::binary::Sonar::Channel::Samples, bases<gz4d::binary::Sonar::Channel::Packet> >("Sonar_Channel_Samples")
            .def_readwrite("geometry", &gz4d::binary::Sonar::Channel::Samples::geometry)
            .def_readwrite("pulse", &gz4d::binary::Sonar::Channel::Samples::pulse)
            .def_readwrite("sampleInterval", &gz4d::binary::Sonar::Channel::Samples::sampleInterval)
            .def_readwrite("offset", &gz4d::binary::Sonar::Channel::Samples::offset)
            .def_readwrite("samples", &gz4d::binary::Sonar::Channel::Samples::samples)
            ;
}
