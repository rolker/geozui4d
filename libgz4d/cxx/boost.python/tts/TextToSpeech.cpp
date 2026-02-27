#include "../boost_python.h"
#include <gz4d/TextToSpeech.h>

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(TextToSpeech_overloads, frequency, 1, 2)

IMPLEMENT_GZ4D_TEXTTOSPEECH

using namespace boost::python;

BOOST_PYTHON_MODULE(TextToSpeech)
{
    class_<gz4d::TextToSpeech, bases<>, std::shared_ptr<gz4d::TextToSpeech>, boost::noncopyable>("TextToSpeech", init<>())
    .def("say", &gz4d::TextToSpeech::say)
    .def("setVoice", &gz4d::TextToSpeech::setVoice)
    .def("frequency", &gz4d::TextToSpeech::frequency, TextToSpeech_overloads())
    .def("rate",&gz4d::TextToSpeech::rate)
    ;
}
