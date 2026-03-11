// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/av/AVWriter.h>

using namespace boost::python;

inline void avw_setsize1(gz4d::AVWriter *avw, gz4d::Vector<std::size_t,2> const &s)
{
    avw->SetSize(s);
}

inline void avw_setsize2(gz4d::AVWriter *avw, int w, int h)
{
    avw->SetSize(w,h);
}

BOOST_PYTHON_MODULE(AVWriter)
{
    class_<gz4d::AVWriter>("AVWriter")
        .def("Open", &gz4d::AVWriter::Open)
        .def("Close", &gz4d::AVWriter::Close)
        .def("SetSize", avw_setsize1)
        .def("SetSize", avw_setsize2)
        .def("SetFPS", &gz4d::AVWriter::SetFPS)
        .def("SetVideoBitrate", &gz4d::AVWriter::SetVideoBitrate)
        .def("SetAudioBitrate", &gz4d::AVWriter::SetAudioBitrate)
        .def("SetAudioSampleRate", &gz4d::AVWriter::SetAudioSampleRate)
        .def("SetAudioChannels", &gz4d::AVWriter::SetAudioChannels)

        //.def("AddFrame", &gz4d::AVWriter::AddFrame)
        .def("AddAudio", &gz4d::AVWriter::AddAudio)

        .def("GetAudioFrameSize", &gz4d::AVWriter::GetAudioFrameSize)
    ;
}
