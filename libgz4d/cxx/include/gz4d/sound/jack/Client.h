// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOUND_JACK_CLIENT_H
#define GZ4D_SOUND_JACK_CLIENT_H

#include <gz4d/gz4d.h>
#include <gz4d/sound/Jack.h>
#include "Port.h"
#include <gz4d/time.h>
#include <jack/jack.h>
#include <memory>
#include <vector>

namespace gz4d
{
    namespace jack
    {
        class Client
        {
            friend class Port;
            friend class InputPort;
            friend class OutputPort;
            friend class MidiInputPort;
            friend class MidiOutputPort;

            std::shared_ptr<jack_client_t> client;

            typedef std::vector<std::shared_ptr<InputPort> > InList;
            typedef std::vector<std::shared_ptr<OutputPort> > OutList;
            typedef std::vector<std::shared_ptr<MidiInputPort> > MidiInList;
            typedef std::vector<std::shared_ptr<MidiOutputPort> > MidiOutList;

            InList inPorts;
            OutList outPorts;
            MidiInList midiInPorts;
            MidiOutList midiOutPorts;

            jack_nframes_t startFrame;
            Time startTime;
            jack_time_t startJTime;

            static int ProcessCallback(jack_nframes_t nframes, void * c)
            {
                Client * client = reinterpret_cast<Client *>(c);
                jack_nframes_t f = jack_last_frame_time(client->client.get());
                Time t(client->startTime + time::Duration((jack_frames_to_time(client->client.get(),f)-client->startJTime)/1000000.0));
                jack_nframes_t sr = client->GetSampleRate();
                
                for(InList::iterator i = client->inPorts.begin(); i != client->inPorts.end(); ++i)
                    (*i)->Process(t,f,sr,nframes);
                
                for(MidiInList::iterator i = client->midiInPorts.begin(); i != client->midiInPorts.end(); ++i)
                    (*i)->Process(t,f,sr,nframes);
                
                return 0;
            }
            
            public:
                typedef std::shared_ptr<Client> Ptr;

                Client(std::string const &name)
                {
                    jack_status_t ret;
                    client = std::shared_ptr<jack_client_t>(jack_client_open(name.c_str(),JackNullOption,&ret),jack_client_close);
                    jack_set_process_callback(client.get(),ProcessCallback,this);
                }
                
                ~Client()
                {
                    jack_deactivate(client.get());
                }
                

                std::shared_ptr<InputPort> NewInputPort(std::string const &name)
                {
                    std::shared_ptr<InputPort> ret(new InputPort(client,name));
                    inPorts.push_back(ret);
                    return ret;
                }
                
                std::shared_ptr<OutputPort> NewOutputPort(std::string const &name)
                {
                    std::shared_ptr<OutputPort> ret(new OutputPort(client,name));
                    outPorts.push_back(ret);
                    return ret;
                }
                
                std::shared_ptr<MidiInputPort> NewMidiInputPort(std::string const &name)
                {
                    std::shared_ptr<MidiInputPort> ret(new MidiInputPort(client,name));
                    midiInPorts.push_back(ret);
                    return ret;
                }
                
                std::shared_ptr<MidiOutputPort> NewMidiOutputPort(std::string const &name)
                {
                    std::shared_ptr<MidiOutputPort> ret(new MidiOutputPort(client,name));
                    midiOutPorts.push_back(ret);
                    return ret;
                }
                

                void Activate()
                {
                    startTime = time::Now();
                    startFrame = jack_frame_time(client.get());
                    startJTime = jack_get_time();
                    if (jack_activate(client.get()))
                        throw(Exception("Problem activating Jack client."));
                }
                
                void Deactivate()
                {
                    if (jack_deactivate(client.get()))
                        throw(Exception("Problem Deactivating Jack client."));
                }
                

                uint32_t GetSampleRate()
                {
                    return jack_get_sample_rate(client.get());
                }
                
                uint32_t GetBufferSize()
                {
                    return jack_get_buffer_size(client.get());
                }
                
        };

    }
}

#endif
