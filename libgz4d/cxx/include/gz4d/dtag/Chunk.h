// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DTAG_CHUNK_H
#define GZ4D_DTAG_CHUNK_H

#include <gz4d/dtag/Block.h>
#include <gz4d/dtag/Chip.h>

#include <memory>
#include <vector>

namespace gz4d
{

    namespace dtag
    {
        //class Chip;

        class Chunk
        {
            std::shared_ptr<Chip> chip;
            BlockAddress address;
            BlockAddress dataAddress;

            uint16_t sourceID;
            uint16_t dataLength;
            uint16_t sampleCount;
            uint32_t startTime;
            bool isConfig;
            bool timingError;

            public:
                Chunk(){}
                Chunk(std::shared_ptr<Chip> &chip, BlockAddress const &address)
                :chip(chip),address(address)
                {
                    if(address)
                    {
                        dataAddress = chip->CalculateAddress(address, 16);
                        
                        uint16_t magic = chip->Read<uint16_t>(address);
                        if(magic != 0xA951)
                            throw(Exception("Invalid chunk magic number"));
                        
                        uint16_t headerChecksum = chip->Read<uint16_t>(address,12);
                        boost::crc_optimal<16, 0x1021, 0xFFFF, 0, true, true>  crc;
                        crc.reset();
                        std::vector<uint8_t> header = chip->Read(address,12);
                        crc.process_block(static_cast<unsigned char const *>(&header[0]),static_cast<unsigned char const *>(&header[12]));
                        if(crc.checksum() != headerChecksum)
                            throw(Exception("Invalid chunk header checksum"));
                        
                        uint16_t id = chip->Read<uint16_t>(address,2);
                        sourceID = 0x3FFF & id;
                        isConfig = 0x4000 & id;
                        timingError = 0x8000 & id;
                        
                        dataLength = chip->Read<uint16_t>(address,4);
                        sampleCount = chip->Read<uint16_t>(address,6);
                        startTime = chip->Read<uint32_t>(address,8);
                    }
                }
                

                std::vector<uint8_t> GetData()
                {
                    if(dataAddress)
                        return chip->Read(dataAddress,dataLength);
                    return std::vector<uint8_t>();
                }
                
                template<typename T> T Read(uint32_t offset=0)
                {
                    return chip->Read<T>(dataAddress,offset);
                };

                template<typename T> std::vector<T> ReadVector(uint16_t size, uint32_t offset=0)
                {
                    return chip->ReadVector<T>(dataAddress,size,offset);
                };


                uint16_t GetDataLength() const {return dataLength;}
                uint16_t GetSampleCount() const {return sampleCount;}
                uint16_t GetSourceID() const {return sourceID;}
                bool IsConfig() const {return isConfig;}
                bool HasTimingError() const {return timingError;}

                BlockAddress const & GetAddress() const {return address;}
                std::string GetInfo()
                {
                    std::stringstream s;
                    s << "Chunk: source id: " << GetSourceID() << "\tconfig? " << IsConfig() << "\ttiming error? " << HasTimingError() << "\tdata length: " <<  GetDataLength() << "\ttime since start: " << GetStartTime()/1000.0 << " s\taddress: " << address.block << ":" << address.byte << "\t" << chip->GetBlockTime(address.block);
                    return s.str(); 
                }
                

                /// Returns time of first sample in units of 1000 ticks per second since start or last 32bit rollover.
                uint32_t GetStartTime()  const {return startTime;} 

                Chunk Next()
                {
                    try
                    {
                        return Chunk(chip,chip->CalculateAddress(dataAddress,GetDataLength()));
                    }
                    catch(...)
                    {
                        if(dataAddress.block < chip->GetEndBlock())
                            return Chunk(chip,chip->GetBlockContainer(chip->NextBlock(dataAddress.block))->GetFirstChunkAddress(chip->NextBlock(dataAddress.block)));
                    }
                    return Chunk(chip,BlockAddress());
                    
                }
                
                Chunk Seek(uint32_t t)
                {
                    uint16_t sb = chip->GetStartBlock();
                    uint16_t eb = chip->GetEndBlock();
                    while(chip->NextBlock(sb) < eb)
                    {
                        Chunk sc(chip,chip->GetFirstChunkAddress(sb));
                        Chunk ec(chip,chip->GetFirstChunkAddress(eb));
                        float p = (t-sc.GetStartTime())/float(ec.GetStartTime()-sc.GetStartTime());
                        uint16_t tb = sb + (eb-sb)*p;
                        if(tb <= sb)
                            tb = chip->NextBlock(sb);
                        if(tb >= eb)
                            tb = chip->PreviousBlock(eb);
                        if(!chip->HasBlock(tb))
                        {
                            if(tb-sb < eb-tb)
                                tb = chip->NextBlock(sb);
                            else
                                tb = chip->PreviousBlock(eb);
                        }
                        Chunk tc(chip,chip->GetFirstChunkAddress(tb));
                        if(tc.GetStartTime() > t)
                            eb = tb;
                        else
                            sb = tb;
                    }
                    Chunk ret = Chunk(chip,chip->GetFirstChunkAddress(sb));
                    while(ret.GetSourceID() != sourceID && ret.Next())
                        ret = ret.Next();
                    Chunk nc = ret.Next();
                    while(nc && nc.GetSourceID() != sourceID)
                        nc = nc.Next();
                    while(nc && nc.GetStartTime() < t)
                    {
                        ret = nc;
                        nc = nc.Next();
                        while(nc.GetSourceID() != sourceID)
                            nc = nc.Next();
                    }
                    return ret;
                }
                

                operator bool() const {return address;}
        };
    }
}

#endif
