#ifndef GZ4D_AUDIODEVICE_H
#define GZ4D_AUDIODEVICE_H

//gz4d_cmake depends portaudio gz4d_sound

#include <gz4d/gz4d.h>

#include <gz4d/Exception.h>
#include <gz4d/time.h>
#include <portaudio.h>
#include <string>
#include <string.h>

namespace gz4d
{
    /** Allows the use of a sound card for recording or playback.
    \ingroup devices
    */
    class GZ4DEXPORT AudioDevice
    {
        public:
            class GZ4DEXPORT Exception: public gz4d::Exception
            {
                public:
                    Exception(std::string msg):gz4d::Exception(msg){}
            };


            class Device;

            class HostApi
            {
                public:
                    HostApi(){id = Pa_GetDefaultHostApi();}
                    HostApi(PaHostApiIndex i):id(i){}
                    HostApi(HostApi const &h):id(h.id){}
                    operator PaHostApiIndex(){return id;}
                    std::string Info();
                    std::string GetName();
                    Device GetDefaultInputDevice();
                    Device GetDefaultOutputDevice();
                    Device operator[](int index);
                    int GetDeviceCount();
                private:
                    PaHostApiInfo const *GetInfo();
                    PaHostApiIndex id;
            };

            class GZ4DEXPORT Device
            {
                public:
                    Device(){id = Pa_GetDefaultInputDevice();}
                    Device(PaDeviceIndex i):id(i){}
                    Device(Device const &d):id(d.id){}
                    operator PaDeviceIndex(){return id;}
                    std::string Info();
                    std::string GetName(){return GetInfo()->name;}
                    HostApi GetHostApi(){return GetInfo()->hostApi;}
                    int GetMaxInputChannels(){return GetInfo()->maxInputChannels;}
                    int GetMaxOutputChannels(){return GetInfo()->maxOutputChannels;}
                    Time GetDefaultHighInputLatency(){return GetInfo()->defaultHighInputLatency;}
                    Time GetDefaultHighOutputLatency(){return GetInfo()->defaultHighOutputLatency;}
                    Time GetDefaultLowInputLatency(){return GetInfo()->defaultLowInputLatency;}
                    Time GetDefaultLowOutputLatency(){return GetInfo()->defaultLowOutputLatency;}
                private:
                    PaDeviceInfo const *GetInfo();
                    PaDeviceIndex id;
            };

            class GZ4DEXPORT Stream
            {
                public:
                    class Parameters
                    {
                        public:
                            Parameters(){memset(&parameters,0,sizeof(parameters)); parameters.sampleFormat = paFloat32;}
                            Parameters(Parameters const &p){parameters=p.parameters;}
                            void SetDevice(Device d){parameters.device = d;}
                            Device GetDevice(){return Device(parameters.device);}
                            void SetChannelCount(int c){parameters.channelCount = c;}
                            int GetChannelCount(){return parameters.channelCount;}
                            void SetSuggestedLatency(Time t){parameters.suggestedLatency = t;}
                            Time GetSuggestedLatency(){return parameters.suggestedLatency;}
                            operator PaStreamParameters const *()const{if(!parameters.channelCount)return NULL; return &parameters;}
                        protected:
                            PaStreamParameters parameters;
                    };
                    class DefaultInputParameters:public Parameters
                    {
                        public:
                            DefaultInputParameters(){parameters.device = GetDefaultInputDevice(), parameters.channelCount = 1; parameters.suggestedLatency = Device(parameters.device).GetDefaultHighInputLatency();}
                    };
                    class DefaultOutputParameters:public Parameters
                    {
                        public:
                            DefaultOutputParameters(){parameters.device = GetDefaultOutputDevice(), parameters.channelCount = 1; parameters.suggestedLatency = Device(parameters.device).GetDefaultHighOutputLatency();}
                    };
                    class Format
                    {
                        public:
                            Format(){}
                            Format(Parameters const &in, Parameters const &out, double rate):input(in), output(out), sampleRate(rate){}
                            Format(Format const &f):input(f.input), output(f.output), sampleRate(f.sampleRate){}
                            bool IsSupported();
                            void SetInputParameters(Parameters const &p){input = p;}
                            void SetOutputParameters(Parameters const &p){output = p;}
                            void SetSampleRate(double r){sampleRate = r;}
                            Parameters const &GetInputParameters()const{return input;}
                            Parameters const &GetOutputParameters()const{return output;}
                            double GetSampleRate()const{return sampleRate;}
                        private:
                            Parameters input;
                            Parameters output;
                            double sampleRate;
                    };

                    void Open(Format const &format, AudioDevice *ad);
                    void Close();
                    void Start();
                    void Stop();
                    void Abort();
                    bool IsStopped();
                    bool IsActive();
                    Time GetTime();
                    double GetCpuLoad();
                    Time GetInputLatency(){return GetInfo()->inputLatency;}
                    Time GetOutputLatency(){return GetInfo()->outputLatency;}
                    double GetSampleRate(){return GetInfo()->sampleRate;}
                    operator bool(){return stream;}
                private:
                    PaStream *stream;
                    PaStreamInfo const *GetInfo();
            };

            AudioDevice();
            virtual ~AudioDevice();

            static PaDeviceIndex GetDeviceCount();
            static void PrintInfo();
            static Device GetDefaultInputDevice();
            static Device GetDefaultOutputDevice();
        private:
            static void CheckError(PaError err,std::string phase="");
            virtual bool ConsumeData(float const *  buffer, unsigned long size){return true;};
            virtual bool ProduceData(float * buffer, unsigned long size){memset(buffer,0,sizeof(float)*size); return true;}
            static int AudioCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

    };
}

#endif
