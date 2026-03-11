// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOUND_PORTAUDIO_STREAM_H
#define GZ4D_SOUND_PORTAUDIO_STREAM_H

#include "Exception.h"
#include <gz4d/time.h>
#include <gz4d/thread/Producer.h>
#include <gz4d/sound/AudioPacket.h>

namespace gz4d
{
    namespace portaudio
    {
        class Handle;
        class Format;
        
        class Stream: public Producer<AudioPacket::Ptr>
        {
            friend class Format;
            
            std::shared_ptr<Handle> _handle;
            std::shared_ptr<PaStream> _stream;
            PaStreamInfo const * _info;


        public:
            typedef AudioPacket::Ptr input_type;
            typedef ThreadedQueue<input_type>::Ptr input_queue_type;
        private:
            input_queue_type inQueue;
            struct OutFragment
            {
                input_type  outgoingPacket;
                std::vector<float>::const_iterator cursor;
            };
            OutFragment outFragment;
            

            static int callback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
            {
                int ret = 0;
                
                Stream * self = reinterpret_cast<Stream*>(userData);
                
                if (inputBuffer && !self->NoOutputQueue())
                {
                    AudioPacket::Ptr data(new AudioPacket);
                    data->sample_rate = self->_info->sampleRate;
                    data->sample_number = 0;
                    float const *buffer = reinterpret_cast<float const*>(inputBuffer);
                    data->samples.insert(data->samples.begin(),buffer,&buffer[framesPerBuffer]);
                    self->Send(data);
                }
                    
                if (outputBuffer)
                {
                    unsigned long framesRemaining = framesPerBuffer;
                    while(framesRemaining)
                    {
                        float *buffer = reinterpret_cast<float *>(outputBuffer);
                        if(!self->outFragment.outgoingPacket)
                        {
                            if(!self->inQueue->empty())
                            {
                                self->outFragment.outgoingPacket = self->inQueue->pop();
                                self->outFragment.cursor = self->outFragment.outgoingPacket->samples.begin();
                            }
                            else
                            {
                                std::vector<float> z(framesRemaining);
                                memcpy(&buffer[framesPerBuffer-framesRemaining],&z.front(),framesRemaining*sizeof(float));
                                return ret;
                            }
                        }
                        std::vector<float>::const_iterator end = self->outFragment.outgoingPacket->samples.end();
                        if(end-self->outFragment.cursor > framesRemaining)
                            end = self->outFragment.cursor+framesRemaining;
                        
                        memcpy(&buffer[framesPerBuffer-framesRemaining],&(*self->outFragment.cursor),(end-self->outFragment.cursor)*sizeof(float));
                        framesRemaining -= end-self->outFragment.cursor;
                        self->outFragment.cursor = end;
                        if(self->outFragment.cursor == self->outFragment.outgoingPacket->samples.end())
                            self->outFragment.outgoingPacket.reset();
                    }
                }
                
                        
                return ret;
            }

            Stream(std::shared_ptr<Handle> h, PaStreamParameters const *ip, PaStreamParameters const *op, double sampleRate):_handle(h),inQueue(new ThreadedQueue<AudioPacket::Ptr>)
            {
                PaStream *s;
                PaError err = Pa_OpenStream(&s, ip, op, sampleRate, paFramesPerBufferUnspecified,paNoFlag,Stream::callback,this);
                if(err != paNoError)
                    throw Exception(err);
                _stream = std::shared_ptr<PaStream>(s,Pa_CloseStream);
                _info = Pa_GetStreamInfo(s);
            }

        public:
            void start()
            {
                PaError e = Pa_StartStream(_stream.get());
                if(e != paNoError)
                    throw Exception(e);
            }
            
            void stop()
            {
                PaError e = Pa_StopStream(_stream.get());
                if(e != paNoError)
                    throw Exception(e);
            }
            
            void abort()
            {
                PaError e = Pa_AbortStream(_stream.get());
                if(e != paNoError)
                    throw Exception(e);
            }
            
            bool isStopped()
            {
                PaError ret = Pa_IsStreamStopped(_stream.get());
                if(ret < 0)
                    throw Exception(ret);
                return ret;
            }
            
            bool isActive()
            {
                PaError ret = Pa_IsStreamActive(_stream.get());
                if(ret<0)
                    throw Exception(ret);
                return ret;
            }
            
            time::Duration getTime()
            {
                return seconds(Pa_GetStreamTime(_stream.get()));
            }
            
            double getCpuLoad()
            {
                return Pa_GetStreamCpuLoad(_stream.get());
            }
            
            time::Duration getInputLatency(){return seconds(_info->inputLatency);}
            time::Duration getOutputLatency(){return seconds(_info->outputLatency);}
            double getSampleRate(){return _info->sampleRate;}
            
            input_queue_type &getInQueue(){return inQueue;}
            
        };
        
        
    }
}

#endif
