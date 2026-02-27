#ifndef GZ4D_DTAG_DTG_MULTIAUDIOSTREAM_H
#define GZ4D_DTAG_DTG_MULTIAUDIOSTREAM_H

#include <gz4d/dtag.h>
#include <gz4d/dtag/DataStream.h>
#include <gz4d/sound/AudioPacket.h>

#define MAX_CHANNEL 4
namespace gz4d
{
    namespace dtag
    {
        class MultiAudioStream: public DataStream
        {
            public:
                typedef std::shared_ptr<MultiAudioStream> Ptr;
                MultiAudioStream()
                : DataStream(0x30)
                {
                }
                
                using DataStream::operator=;
                
                std::vector<gz4d::AudioPacket::Ptr> Next()
                {
                    do
                    {
                        DataStream::Next();
                    }
                    while(currentChunk && currentChunk.IsConfig());
                    
                    std::vector<gz4d::AudioPacket::Ptr> ret;
                    if(currentChunk)
                    {
                        for(int c = 0; c < numberChannels; ++c)
                        {
                            AudioPacket::Ptr ap(new AudioPacket);
                            ret.push_back(ap);
                            ret[c]->sample_rate = sampleRate;
                            ret[c]->timestamp = startTime + time::Duration(currentChunk.GetStartTime()/1000.0);
                        }
                        
                        float scale = 1.0/float(1<<(nBits-1));
                        
                        
                        BitSpliter bs(currentChunk.GetData());
                        
                        while(bs.BytesRemaining()>2)
                        {
                            unsigned char header = bs.Read(4);
                            unsigned char channel = bs.Read(2);
                            if(channel >= numberChannels)
                                return std::vector<gz4d::AudioPacket::Ptr>();
                            unsigned char numBits = bs.Read(6);
                            std::vector<int> samples;
                            switch(header)
                            {
                                case 0x0:   // MSC
                {
                    short uncSampleCount = bs.Read(8); // number of uncompressed samples
                    for (int i = 0; i < overlap; i++)
                    {
                        int copy = bs.Read(numBits,true); // the first sample(s) is(are) a copy of the last one.
                        if (!ret[channel]->samples.empty())
                            if (copy*scale != ret[channel]->samples[ret[channel]->samples.size()-overlap+i])
                                std::cerr << "DtgMultiAudioStream::AddData --- Overlapping samples don't match!\n";
                    }
                    for (int i = 0; i < uncSampleCount; i++)
                    {
                        samples.push_back(bs.Read(numBits,true));
                    }
                    while(samples.size() < currentChunk.GetSampleCount() && bs.BytesRemaining()>2)
                    {
                        bool riceFlag = bs.Read(1);
                        bool deemphasisFlag;
                        if (riceFlag)
                        {
                            for(int i = 0; i < blockLength; i++)
                            {
                                try
                                {
                                    samples.push_back(bs.ReadRice(riceCode));
                                }
                                catch(gz4d::Exception)
                                {
                                    return std::vector<gz4d::AudioPacket::Ptr>();
                                }
                            }
                            deemphasisFlag = true;
                        }
                        else
                        {
                            numBits = bs.Read(4);
                            if(numBits == 0)
                            {
                                numBits = 16;
                                deemphasisFlag = false;
                            }
                            else
                                deemphasisFlag = true;
                            for(int i = 0; i < blockLength; i++)
                            {
                                try
                                {
                                    samples.push_back(bs.Read(numBits,true));
                                }
                                catch(gz4d::Exception)
                                {
                                    return std::vector<gz4d::AudioPacket::Ptr>();
                                }
                            }
                        }
                        if(deemphasisFlag)
                        {
                            switch (preEmphasisFilter)
                            {
                                case 1: //integrate
                                    for(int i = samples.size()-blockLength; i < samples.size(); i++)
                                        samples[i] += samples[i-1];
                                    break;
                                case 2: //double integrate
                                    for(int i = samples.size()-blockLength; i < samples.size(); i++)
                                        samples[i] += 2*samples[i-1]-samples[i-2];
                                    break;
                                default:
                                    break;
                            };
                        }
                    }
                }
                break;
                                default:
                                    bs.clear();
                                    //std::cerr << "DtgMultiAudioStream::AddData --- unknown audio block type\n";
                            }
                            for(std::vector<int>::iterator s = samples.begin(); s != samples.end(); ++s)
                                ret[channel]->samples.push_back(*s*scale);
                        }
                        for(int c = 0; c < numberChannels; ++c)
                        {
                            while(ret[c]->samples.size() < currentChunk.GetSampleCount())
                                ret[c]->samples.push_back(0.0);
                        }
                        
                    }
                    return ret;
                }
                
                uint32_t GetSampleRate() const {return sampleRate;}
                uint16_t GetChannelCount() const {return numberChannels;}
/*                bool SaveWAV(std::string fname);
                bool SaveASCII(std::string fname, float seconds = 0.5);
                bool SaveFLAC(std::string fname);
                bool SaveOggVorbis(std::string fname);*/
            private:
                void ParseConfig()
                {
                    sampleRate = currentConfig.Read<uint16_t>(0)*1000;
                    nBits = currentConfig.Read<uint16_t>(2);
                    adc = currentConfig.Read<uint16_t>(4);
                    channels = currentConfig.Read<uint16_t>(6);
                    gain[0] = currentConfig.Read<uint8_t>(8);
                    gain[1] = currentConfig.Read<uint8_t>(9);
                    filterMode = currentConfig.Read<uint16_t>(10);
                    compression = currentConfig.Read<uint16_t>(12);
                    blockLength = currentConfig.Read<uint16_t>(14);
                    riceCode = currentConfig.Read<uint16_t>(16);
                    preEmphasisFilter = currentConfig.Read<uint16_t>(18);
                    riceThreshold = currentConfig.Read<uint16_t>(20);
                    overlap = currentConfig.Read<uint16_t>(22);
                    numberChannels = currentConfig.Read<uint16_t>(24);
                    uint16_t s = currentConfig.Read<uint16_t>(26);
                    if(s >= 32768)
                        sensitivity = s - 65536;
                    else
                        sensitivity = s;
                    
                }
                

                uint32_t sampleRate;  ///< Sampling rate Hz.
                uint16_t nBits;       ///< Number of bits per conversion.
                uint16_t adc;         ///< Analog/Digital converter. 0=AD7853, 1=CS5333, 2=CS5341
                uint16_t channels;
                uint8_t gain[2];      ///< Preamp gains in dB
                uint16_t filterMode;  ///< Anti-alias filter - meaning depends on ADC	mode on CS5333
                uint16_t compression;
                uint16_t blockLength;
                uint16_t riceCode;
                uint16_t preEmphasisFilter;
                uint16_t riceThreshold;
                uint16_t overlap;
                uint16_t numberChannels;
                int32_t sensitivity;
    
//                 std::vector<std::vector<long> > sample; ///< arrays of samples.
//                 int bufferSize;
//                 std::vector<int> bufferCursor[MAX_CHANNEL]; ///< indicies into samples array, one for each channel.
//                 unsigned int bufferTime;
//                 double totalSampleTime;
//                 double progressMark;
//     
//                float timeOffset; ///< Difference between the chunk timestamp and time of last sample of first data chunk. Used to detect sync issues as stream progresses.
//                unsigned int zeroFillerCount; ///< If missing samples are detected, the number of guestimated samples to add to catch up. 
        };
    }
}

#endif
