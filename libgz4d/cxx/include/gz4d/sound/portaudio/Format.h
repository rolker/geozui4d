#ifndef GZ4D_SOUND_PORTAUDIO_FORMAT_H
#define GZ4D_SOUND_PORTAUDIO_FORMAT_H

#include "../Portaudio.h"
#include "Device.h"
#include "Stream.h"

namespace gz4d
{
    namespace portaudio
    {
        class Handle;
        class Context;
        
        class Format
        {
        public:
            class Parameters: public PaStreamParameters
            {
                friend class Format;
                Parameters(PaDeviceIndex d, PaTime latency)
                {
                    device = d;
                    channelCount = 1;
                    sampleFormat = paFloat32;
                    suggestedLatency = latency;
                    hostApiSpecificStreamInfo = NULL;
                }
                
                Parameters(Parameters const &p)
                {
                    device = p.device;
                    channelCount = p.channelCount;
                    sampleFormat = p.sampleFormat;
                    suggestedLatency = p.suggestedLatency;
                    hostApiSpecificStreamInfo = p.hostApiSpecificStreamInfo;
                }
                
                PaStreamParameters const * pointer() const {if(!channelCount)return NULL; return this;}
            public:
                void setDevice(Device const &d){device = d;}
                void setChannelCount(int c){channelCount = c;}
                int getChannelCount() const {return channelCount;}
                void setSuggestedLatency(time::Duration d){suggestedLatency = d.asSeconds();}
                time::Duration getSuggestedLatency()const{return time::Duration(suggestedLatency);}
            };
            
        private:
            friend class Context;
            
            std::shared_ptr<Handle> _handle;
            Parameters _inputParameters;
            Parameters _outputParameters;
            double _sampleRate;
            
            
            Format(std::shared_ptr<Handle> h, PaDeviceIndex inputDevice, PaTime inputLatency, PaDeviceIndex outputDevice, PaTime outputLatency, double sampleRate):_handle(h),_inputParameters(inputDevice,inputLatency),_outputParameters(outputDevice,outputLatency),_sampleRate(sampleRate)
            {
            }
        public:
            
            Format(Format const &f):_handle(f._handle),_inputParameters(f._inputParameters),_outputParameters(f._outputParameters),_sampleRate(f._sampleRate){}
            
            bool isSupported() const
            {
                PaError ret = Pa_IsFormatSupported(_inputParameters.pointer(),_outputParameters.pointer(),_sampleRate);
                return ret == paFormatIsSupported;
            }

            void setSampleRate(double r){_sampleRate = r;}
            double getSampleRate()const{return _sampleRate;}
            
            Parameters const &getInputParameters()const{return _inputParameters;}
            Parameters const &getOutputParameters()const{return _outputParameters;}
            Parameters &getInputParameters(){return _inputParameters;}
            Parameters &getOutputParameters(){return _outputParameters;}
          
            Stream open()
            {
                return Stream(_handle,_inputParameters.pointer(),_outputParameters.pointer(),_sampleRate);
            }
        };
        
    }
}



#endif
