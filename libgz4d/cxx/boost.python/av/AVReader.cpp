#include "../boost_python.h"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <gz4d/av/AVReader.h>

using namespace boost::python;

bool (gz4d::AVReader::*Open1)(std::string const &in) = &gz4d::AVReader::Open;

BOOST_PYTHON_MODULE(AVReader)
{
    class_<gz4d::AVReader::Stream, bases<>, std::shared_ptr<gz4d::AVReader::Stream> >("Stream", no_init)
        .def("GetDuration", &gz4d::AVReader::Stream::GetDuration)
        //.def("Seek", &gz4d::AVReader::Stream::Seek)
    ;

    class_<gz4d::AVReader::AudioStream, bases<gz4d::AVReader::Stream>, std::shared_ptr<gz4d::AVReader::AudioStream> >("AudioStream", no_init)
    ;

    class_<std::vector<std::shared_ptr<gz4d::AVReader::AudioStream> > >("VectorAudioStream")
    .def(vector_indexing_suite<std::vector<std::shared_ptr<gz4d::AVReader::AudioStream> >, true >())
    ;
    
    
    class_<gz4d::AVReader::VideoFrame, bases<>, std::shared_ptr<gz4d::AVReader::VideoFrame> >("VideoFrame")
        .def_readwrite("presentationTime", &gz4d::AVReader::VideoFrame::presentationTime)
        .def_readwrite("decompressionTime", &gz4d::AVReader::VideoFrame::decompressionTime)
        .def_readwrite("duration", &gz4d::AVReader::VideoFrame::duration)
        .def_readwrite("convergenceDuration", &gz4d::AVReader::VideoFrame::convergenceDuration)
        .def_readwrite("isKeyFrame", &gz4d::AVReader::VideoFrame::isKeyframe)
        .def_readwrite("frame", &gz4d::AVReader::VideoFrame::frame)
    ;

    gz4d::WrapProducer<gz4d::AVReader::VideoFrame>("VideoFrameProducer");
    gz4d::WrapThreadedQueue<gz4d::AVReader::VideoFrame>("VideoFrameThreadedQueue");
    
    class_<gz4d::AVReader::VideoStream, bases<gz4d::Producer<gz4d::AVReader::VideoFrame>, gz4d::AVReader::Stream >, std::shared_ptr<gz4d::AVReader::VideoStream> >("VideoStream", no_init)
        .def("GetWidth", &gz4d::AVReader::VideoStream::GetWidth)
        .def("GetHeight", &gz4d::AVReader::VideoStream::GetHeight)
    ;

    
    
    class_<std::vector<std::shared_ptr<gz4d::AVReader::VideoStream> > >("VectorVideoStream")
    .def(vector_indexing_suite<std::vector<std::shared_ptr<gz4d::AVReader::VideoStream> >, true >())
    ;

    class_<gz4d::AVReader::SubtitleFrame, bases<>, std::shared_ptr<gz4d::AVReader::SubtitleFrame> >("SubtitleFrame")
        .def_readwrite("presentationTime", &gz4d::AVReader::SubtitleFrame::presentationTime)
        .def_readwrite("data", &gz4d::AVReader::SubtitleFrame::data)
    ;
    
    gz4d::WrapProducer<gz4d::AVReader::SubtitleFrame>("SubtitleFrameProducer");
    gz4d::WrapThreadedQueue<gz4d::AVReader::SubtitleFrame>("SubtitleFrameThreadedQueue");
    
    class_<gz4d::AVReader::SubtitleStream, bases<gz4d::AVReader::Stream, gz4d::Producer<gz4d::AVReader::SubtitleFrame> >, std::shared_ptr<gz4d::AVReader::SubtitleStream> >("SubtitleStream", no_init)
    ;
    
    class_<std::vector<std::shared_ptr<gz4d::AVReader::SubtitleStream> > >("VectorSubtitleStream")
        .def(vector_indexing_suite<std::vector<std::shared_ptr<gz4d::AVReader::SubtitleStream> >, true >())
    ;
    
    
    class_<gz4d::AVReader>("AVReader")
        .def("Open", Open1)
        .def("ReadFrame", &gz4d::AVReader::ReadFrame)
        .def("GetAudioStreams", &gz4d::AVReader::GetAudioStreams,return_value_policy<copy_const_reference>())
        .def("GetVideoStreams", &gz4d::AVReader::GetVideoStreams,return_value_policy<copy_const_reference>())
        .def("GetSubtitleStreams", &gz4d::AVReader::GetSubtitleStreams,return_value_policy<copy_const_reference>())
        .def("Seek", &gz4d::AVReader::Seek)
    ;
}
