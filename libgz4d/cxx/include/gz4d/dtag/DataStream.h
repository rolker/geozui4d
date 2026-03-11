// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DTAG_DATASTREAM_H
#define GZ4D_DTAG_DATASTREAM_H

#include <gz4d/gz4d.h>
#include <memory>
#include <map>
#include <vector>
#include <gz4d/dtag/Chunk.h>
#include <boost/thread/mutex.hpp>

#define DTG_AUDIO_MONO_X3  0x10
#define DTG_AUDIO_MULTI_X3 0x40


namespace gz4d
{
    namespace dtag
    {

        class DataStream
        {
            uint16_t sourceID;
            boost::mutex mutex;

            public:
                DataStream(uint16_t sourceID)
                :sourceID(sourceID)
                {
                };
                
                //DataStream(uint16_t sourceID, DataStream const &ds);
                void SetConfig(Chunk &config_chunk)
                {
                    currentConfig = config_chunk;
                    currentChunk = config_chunk;
                    AddConfig(config_chunk);
                    this->ParseConfig();
                }
                
                void AddConfig(Chunk &config_chunk)
                {
                    if(!configs.count(config_chunk.GetStartTime()))
                        configs[config_chunk.GetStartTime()] = config_chunk;
                }
                
                
                std::string GetInfo()
                {
                    std::stringstream s;
                    s << "Data stream id: " << sourceID << std::endl;
                    s << "  config: " << currentConfig.GetInfo() << std::endl;
                    s << "    configs:" << std::endl;
                    for (std::map<uint32_t,Chunk>::iterator c = configs.begin(); c != configs.end(); ++c)
                        s << "      " << c->second.GetInfo() << std::endl;
                    return s.str();
                }
                
                uint16_t GetSourceID() const {return sourceID;}
                operator bool() const {return currentChunk;}
                void SetStartTime(gz4d::Time t) {startTime = t;}
                void Seek(uint32_t t)
                {
                    std::map<uint32_t,Chunk>::iterator cc = configs.begin();
                    if(cc != configs.end())
                    {   
                        std::map<uint32_t,Chunk>::iterator cn = cc;
                        ++cn;
                        while(cn != configs.end() && cn->first <= t)
                        {
                            cc = cn;
                            ++cn;
                        }
                        
                        Chunk targetChunk = cc->second.Seek(t);
                        boost::mutex::scoped_lock lock(mutex);
                        nextConfig = cc->second;
                        nextChunk = targetChunk;
                    }
                }
                

                DataStream const &operator=(DataStream const &ds)
                {
                    if (ds.sourceID == sourceID)
                    {
                        currentConfig = ds.currentConfig;
                        currentChunk = ds.currentChunk;
                        startTime = ds.startTime;
                        configs = ds.configs;
                        this->ParseConfig();
                    }
                    return *this;
                }
                
            protected:
                Chunk currentConfig;
                Chunk currentChunk;
                Chunk nextConfig;
                Chunk nextChunk;
                gz4d::Time startTime;
                std::map<uint32_t,Chunk> configs;

                void Next()
                {
                    boost::mutex::scoped_lock lock(mutex);
                    if(nextConfig)
                    {
                        SetConfig(nextConfig);
                        currentChunk = nextChunk;
                        nextConfig = Chunk();
                        nextChunk = Chunk();
                    }
                    do
                    {
                        currentChunk = currentChunk.Next();
                    }
                    while(currentChunk && currentChunk.GetSourceID() != sourceID);
                    
                    if(currentChunk && currentChunk.IsConfig())
                    {
                        SetConfig(currentChunk);
                    }
                }
                
                virtual void ParseConfig(){}

        /// Allows indexing contiguous bytes at the bit level.
                class BitSpliter
                {
                    public:
                        BitSpliter(std::vector<uint8_t> const &d):cursor(0),data(d){};
                        void Add(unsigned int byte, unsigned char bit = 0){cursor += (byte << 3) + bit;};
                        void Set(unsigned int byte=0, unsigned char bit = 0){cursor = (byte << 3) + bit;};
                        int Read(unsigned char bits, bool signedFlag = false)
                        {
                            unsigned char mask[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};///< used to select the number of bits needed from a byte
                            if(bits >=32)
                                throw(Exception("Too many bits to decode"));
                            if(data.size()*8<cursor+bits)
                                throw(Exception("Not enough bits remaining"));
                            
                            int ret = 0;
                            unsigned char bitsRemaining = bits;
                            while (bitsRemaining>0)
                            {
                                unsigned char bitsInByte = 8 - (cursor&0x7);   // bits remaining to be read from current byte of data.
                                if (bitsRemaining >= bitsInByte) // do we want more bits than are left in the current byte?
        {
            ret <<= bitsInByte;  // make room in our return value for the new bits
            ret |= (((data[cursor >> 3] << (cursor &0x7))& mask[bitsInByte-1]) >>(cursor &0x7)); // grab the current byte and shift left then right to eraser bits before the ones we want. Add the result to the return value.
            bitsRemaining -= bitsInByte; 
            cursor += bitsInByte;
        }
        else
        {
            ret <<= bitsRemaining;
            ret |= (((data[cursor >> 3] << (cursor &0x7))& mask[bitsRemaining-1]) >>(8-bitsRemaining)); // same as above, except we also shift further right to eliminate bits beyond the ones we want.
            cursor += bitsRemaining;
            bitsRemaining = 0;
        }
        
                            }
                            if (signedFlag)
                            {
                                unsigned int signBitMask = 1 << (bits-1);
                                if (ret & signBitMask)
                                    ret -=(signBitMask << 1);
                            }
                            return ret;
                        }
                        
                        int ReadRice(uint8_t mBits)
                        {
                            uint8_t q = 0;
                            while(Read(1))
                            {
                                q++;
                                if(q>=10)
                                    throw(Exception("Error reading rice code"));
                                
                            }
                            q = (q << mBits) | Read(mBits);
                            if (q&0x1)
                                return (q >> 1)+1;
                            else
                                return -(q >> 1);
                        }
                        
                        unsigned int BytesRemaining() { return data.size() - (cursor >> 3);};
                        void clear() {data.clear(); cursor = 0;}
                    private:
                        
                        unsigned int cursor; ///< index in bits into data. The last 3 bits represent the bit while the remaining bits represent the byte.
                        std::vector<uint8_t> data;
                };
        };

/*        template<typename T, typename CT, typename CVT, uint16_t SID> class Stream: public DataStream
        {
            CVT converter;
            public:
                Stream(DataStream &s)
                :DataStream(SID,s)
                {
                }

                struct DataFrame
                {
                    CT config;
                    std::vector<T> samples;
                };
                DataFrame Read(uint32_t n)
                {
                    DataFrame ret;
                    while(ret.samples.size() < n*currentConfig.channel_count)
                    {
                    }
                }
        };*/
    }
}
#endif
