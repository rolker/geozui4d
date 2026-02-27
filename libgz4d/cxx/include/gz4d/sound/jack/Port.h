#ifndef GZ4D_SOUND_JACK_PORT_H
#define GZ4D_SOUND_JACK_PORT_H

#include <gz4d/gz4d.h>
#include <gz4d/sound/Jack.h>
#include <gz4d/sound/AudioPacket.h>
#include <gz4d/thread/Producer.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <memory>
#include <gz4d/midi/Event.h>

namespace gz4d
{
    namespace jack
    {
        class Client;
        class Port
        {
            friend class Client;

            std::shared_ptr<jack_client_t> client;
            jack_port_t *port;

            protected:
                struct Buffer
                {
                    jack_default_audio_sample_t *samples;
                    jack_nframes_t size;
                };
                Port(std::shared_ptr<jack_client_t> c, std::string const &name, std::string const &type, unsigned long flags)
                {
                    client = c;
                    port = jack_port_register(c.get(),name.c_str(),type.c_str(),flags,0);
                }
                
                Buffer GetBuffer(uint32_t nframes)
                {
                    Buffer ret;
                    ret.size = nframes;
                    ret.samples = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(port,nframes));
                    return ret;
                }
                
                void * getRawBuffer(uint32_t nframes)
                {
                    return jack_port_get_buffer(port,nframes);
                }
                

            public:
                typedef std::shared_ptr<Port> Ptr;
                ~Port()
                {
                    jack_port_unregister(client.get(),port);
                }
                
                int Connected()
                {
                    return jack_port_connected(port);
                }
                
        };

        class  InputPort: public Port, public Producer<AudioPacket::Ptr>
        {
            friend class Client;

            InputPort(std::shared_ptr<jack_client_t> c, std::string const &name)
            :Port(c,name,JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput)
            {}
            void Process(Time t, jack_nframes_t f, jack_nframes_t sampleRate, jack_nframes_t count)
            {
                if(Connected())
                {
                    AudioPacket::Ptr ap(new AudioPacket);
                    ap->timestamp = t;
                    ap->sample_rate = sampleRate;
                    ap->sample_number = f;
                    
                    Buffer b = GetBuffer(count);
                    ap->samples.insert(ap->samples.begin(),b.samples,&(b.samples[count]));
                    Send(ap);
                }
            }
            
            
            public:
                typedef std::shared_ptr<InputPort> Ptr;

        };

        class  OutputPort: public Port
        {
            friend class Client;

            OutputPort(std::shared_ptr<jack_client_t> c, std::string const &name)
            :Port(c,name,JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput)
            {}
            public:
                typedef std::shared_ptr<OutputPort> Ptr;
        };

        class  MidiInputPort: public Port, public Producer<midi::Event>
        {
            friend class Client;

            MidiInputPort(std::shared_ptr<jack_client_t> c, std::string const &name)
            :Port(c,name,JACK_DEFAULT_MIDI_TYPE,JackPortIsInput)
            {}
            void Process(Time t, jack_nframes_t f, jack_nframes_t sampleRate, jack_nframes_t count)
            {
                if(Connected())
                {
                    void * buffer = getRawBuffer(count);
                    jack_nframes_t ec = jack_midi_get_event_count(buffer);
                    jack_midi_event_t me;
                    for(jack_nframes_t i = 0; i < ec; ++i)
                    {
                        if(!jack_midi_event_get(&me,buffer,i))
                        {
                            Send(midi::Event(t+gz4d::seconds((me.time-f)/double(sampleRate)), me.buffer,&me.buffer[me.size]));
                        }
                    }
                }
            }
            
            public:
                typedef std::shared_ptr<MidiInputPort> Ptr;
        };

        class  MidiOutputPort: public Port
        {
            friend class Client;

            MidiOutputPort(std::shared_ptr<jack_client_t> c, std::string const &name)
            :Port(c,name,JACK_DEFAULT_MIDI_TYPE,JackPortIsOutput)
            {}
            public:
                typedef std::shared_ptr<MidiOutputPort> Ptr;
        };

    }
}

#endif
