#ifndef GZ4D_DTAG_DTGFILE_H
#define GZ4D_DTAG_DTGFILE_H

#include <gz4d/gz4d.h>
#include <gz4d/dtag/Block.h>
#include <gz4d/time.h>
#include <gz4d/TimeSequence.h>
#include <gz4d/util/Swap.h>
#include <gz4d/time/ios.h>

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <boost/crc.hpp>
#include <boost/thread/mutex.hpp>
#include <filesystem>

namespace gz4d
{
    namespace dtag
    {
        /// Encapsulates functionality for reading a .dtg file
        class File
        {
            public:
                File(std::string const &fileName)
                :file(fileName.c_str(),std::ios::in|std::ios::binary),fileName(fileName)
                {
                    std::cerr << "Initializing " << std::filesystem::path(fileName).filename() << "...";
                    // read the last two bytes at the end of the file, the term.
                    file.seekg(-2,std::ios::end);
                    uint16_t term;
                    file.read(reinterpret_cast<char *>(&term),2);
                    fileSize = file.tellg();
                    
                    if (term != 0x0aaaa)  //OKAY_TERM
                        throw Exception("Found invalid terminator at the end of dtg file.");
                    if(GetBlockNumber(0) == 0xffff)
                        throw Exception("Invalid initial block.");
                    uint8_t flashType = Read<uint8_t>(26+4);
                    switch(flashType)
                    {
                        case 0x73:
                        case 0x75:
                        case 0x76:
                        case 0x79:
                            blockSize = 16896;
                            break;
                        case 0xF1:
                        case 0xDA:
                        case 0xD3:
                            blockSize = 135168;
                            break;
                        case 0xD5:
                            blockSize = 270336;
                            break;
                        default:
                            throw Exception("Unkown chip type.");
                    }
                    
                    uint16_t last = ((fileSize-26)/blockSize)-1;
                    if(GetBlockNumber(last) == 0xffff)
                        throw Exception("Invalid last block.");
                    /*    uint32_t i = 0;
                    *    while(true)
                    *    {
                    *        try
                    *        {
                    *            //if(IsBlockOK(26+i*blockSize))
                    *            {
                    *                uint16_t bn = GetBlockNumber(26+i*blockSize);
                    *                block2address[bn]=26+i*blockSize;
                    *                time2block[GetBlockTime(bn)]=bn;
                    }
                    ++i;
                    }
                    catch(...)
                    {
                        break;
                    }
                    }*/
                    std::cerr << "Done." << std::endl;
                }
         

                std::string GetInfo(bool blockInfo = false, bool chunkInfo = false)
                {
                    std::stringstream s;
                    s << "  File: " << std::filesystem::path(fileName).filename() << std::endl;
                    
                    s << "    " << GetVersion() << " ID: " << (int)GetID();
                    s << "    Uploaded: " << GetUploadTime() << "\tChip: " << (int)GetChip() << std::endl;
                    s << "    Blocks: (reported) " << GetReportedStartBlock() << " - " << GetReportedEndBlock() << " count: " << GetReportedBlockCount() << std::endl;
                    
                    s << "    Blocks: (actual)   " << GetStartBlock() << " - " << GetEndBlock() << " count: " << GetBlockCount() << std::endl;
                    if(blockInfo)
                    {
                        s << "\n";
                        uint16_t block = index2block[0];
                        while(block != index2block.rbegin()->second)
                        {
                            s << "      block " << block << "\t" << GetBlockTime(block) << std::endl;
                            block = NextBlock(block);
                        }
                    }
                    
                    return s.str();
                }
                
                std::string const &GetFilename() {return fileName;};

                gz4d::Time GetUploadTime()
                {
                    uint32_t secs = (Read<uint32_t>(12)&0x00ffffff);
                    return Time(Read<uint16_t>(9),Read<uint8_t>(11),Read<uint8_t>(12),0,0,secs);
                }
                
                
                uint8_t GetID()
                {
                    return Read<uint8_t>(8);
                }
                
                std::string GetVersion()
                {
                    std::vector<uint8_t> ret = Read(0,8);
                    return std::string(ret.begin(),ret.end());
                }
                
                uint8_t GetChip()
                {
                    return Read<uint8_t>(17);
                }
                
                uint16_t GetStartBlock()
                {
                    return block2index.begin()->first;
                }
                
                uint16_t GetEndBlock()
                {
                    return block2index.rbegin()->first;
                }
                
                uint32_t GetBlockCount()
                {
                    return (fileSize-26)/blockSize;
                }
                

                uint16_t GetReportedStartBlock()
                {
                    return Read<uint16_t>(18);
                }
                
                uint16_t GetReportedEndBlock()
                {
                    return Read<uint16_t>(20);
                }
                
                uint32_t GetReportedBlockCount()
                {
                    return Read<uint32_t>(22);
                }
                

                gz4d::Time GetBlockTime(uint16_t block_number)
                {
                    return Time(Read<uint32_t>(IndexToPosition(block2index[block_number])+8)-2208988800.0);
                }
                
                BlockAddress GetFirstChunkAddress(uint16_t block_number)
                {
                    uint16_t offset = Read<uint16_t>(IndexToPosition(block2index[block_number])+12);
                    if (offset > 0)
                        offset -= 2;
                    assert(offset < blockSize);
                    return BlockAddress(block_number, offset);
                }
                
                uint32_t GetBlockSize()
                {
                    return blockSize;
                }
                
                bool HasBlock(uint16_t block_number)
                {
                    if (block2index.begin()->first > block_number || block2index.rbegin()->first < block_number)
                        return false;
                    if(block2index.count(block_number))
                        return true;
                    uint16_t nextBlock = block2index.upper_bound(block_number)->first;
                    uint16_t prevBlock = (++std::map<uint16_t,uint16_t>::reverse_iterator(block2index.upper_bound(block_number)))->first;
                    uint16_t i = block2index[prevBlock] + block_number-prevBlock;
                    while(GetBlockNumber(i) < block_number && i < block2index[nextBlock])
                        ++i;
                    while(GetBlockNumber(i) > block_number && i > block2index[prevBlock])
                        --i;
                    if(GetBlockNumber(i) == block_number)
                        return true;
                    return false;
                }
                
                bool CheckAddress(BlockAddress const&address)
                {
                    if(HasBlock(address.block))
                    {
                        if(Address(address)< fileSize-2)
                            return true;
                    }
                    return false;
                }
                
                uint16_t NextBlock(uint16_t block)
                {
                    uint16_t i = block2index[block]+1;
                    while(i < index2block.rbegin()->first && GetBlockNumber(i) == 0xffff)
                        ++i;
                    if(i > index2block.rbegin()->first)
                        return 0xffff;
                    return GetBlockNumber(i);
                }
                
                uint16_t PreviousBlock(uint16_t block)
                {
                    uint16_t i = block2index[block]-1;
                    while(i > index2block.begin()->first && GetBlockNumber(i) == 0xffff)
                        --i;
                    if(i < index2block.begin()->first)
                        return 0xffff;
                    return GetBlockNumber(i);
                }
                

                template<typename T> T Read(BlockAddress const &address)
                {
                    return Read<T>(Address(address));
                    //return Read<T>(block2address[address.block]+16+address.byte);
                }
                std::vector<uint8_t> Read(BlockAddress const &address, uint16_t size)
                {
                    return Read(Address(address),size);
                }
                

            private:
                std::ifstream file;
                uint32_t fileSize;
                std::string fileName;
                boost::crc_optimal<16, 0x1021, 0xFFFF, 0, true, true>  crc;
                uint32_t blockSize;
                boost::mutex mutex;


                std::map<uint16_t,uint16_t> block2index;
                std::map<uint16_t,uint16_t> index2block;

                uint32_t Address(BlockAddress const &address)
                {
                    return IndexToPosition(block2index[address.block])+16+address.byte;
                }
                
                bool IsBlockOK(uint16_t index)
                {
                    uint32_t position = IndexToPosition(index);
                    uint16_t magicNumber = Read<uint16_t>(position);
                    if(magicNumber == 0x9A15)
                    {
                        uint16_t expected_crc = Read<uint16_t>(position+14);
                        crc.reset();
                        char header[14];
                        {
                            boost::mutex::scoped_lock lock(mutex);
                            file.seekg(position);
                            file.read(header,14);
                        }
                        crc.process_bytes(header,14);
                        if(crc.checksum() == expected_crc)
                        {
                            // make sure first chunk pointer points to a valid chunk
                            uint16_t fc = *reinterpret_cast<uint16_t*>(&header[12]);
                            fc = Swap(fc);
                            if(fc > 0)
                                fc -= 2;
                            uint16_t cm = Read<uint16_t>(position+16+fc);
                            if(cm == 0xA951)
                                return true;
                        }
                    }
                    return false;
                }
                
                uint16_t GetBlockNumber(uint16_t index)
                {
                    if(!index2block.count(index))
                    {
                        if(!IsBlockOK(index))
                            index2block[index] = 0xffff;
                        else
                        {
                            index2block[index] = Read<uint16_t>(IndexToPosition(index)+6);
                            block2index[index2block[index]] = index;
                        }
                    }
                    return index2block[index];
                }
                
                uint32_t IndexToPosition(uint16_t index)
                {
                    return 26+index*blockSize;
                }
                


                template<typename T> T Read(uint32_t position)
                {
                    boost::mutex::scoped_lock lock(mutex);
                    file.clear();
                    file.seekg(position,std::ios::beg);
                    T ret;
                    file.read(reinterpret_cast<char *>(&ret),sizeof(T));
                    if(file.eof())
                        throw (Exception("File read error"));
                    return Swap(ret);
                }
                
                std::vector<uint8_t> Read(uint32_t position, uint16_t size)
                {
                    std::vector<uint8_t> ret(size);
                    
                    boost::mutex::scoped_lock lock(mutex);
                    file.seekg(position,std::ios::beg);
                    file.read(reinterpret_cast<char*>(&ret.front()),size);
                    return ret;
                }
                
        };
    }
}

#endif
