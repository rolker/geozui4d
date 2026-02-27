#ifndef GZ4D_DTAG_CHIP_H
#define GZ4D_DTAG_CHIP_H

#include<gz4d/gz4d.h>
#include <gz4d/dtag/File.h>

#include<map>
#include <memory>

namespace gz4d
{
    namespace dtag
    {
        class File;

        /// Encapsulates functionality for reading a .dtg file
        class Chip
        {
            typedef std::pair<uint32_t,uint32_t> BlockRange;
            typedef std::map<BlockRange, std::shared_ptr<File> >  FileMap; ///< maps starting block to file
            FileMap files;

            public:

                void AddFile(std::shared_ptr<File> &file)
                {
                    if(files.count(BlockRange(file->GetStartBlock(),file->GetEndBlock())))
                        throw(Exception("Trying to add file with same chip number and block range as previously added file."));
                    files[ BlockRange(file->GetStartBlock(),file->GetEndBlock())]=file;
                }
                
                std::shared_ptr<File> &GetBlockContainer(uint16_t block)
                {
                    if (block >= files.begin()->first.first && block <= files.rbegin()->first.second)
                        for(FileMap::iterator f = files.begin(); f != files.end(); ++f)
                            if(block >= f->first.first && block <= f->first.second)
                                return f->second;
                            throw(Exception("Block out of range"));
                }
                
                std::string GetInfo(bool blockInfo = false, bool chunkInfo = false)
                {
                    std::stringstream s;
                    
                    s << "Chip# " << (int)files.begin()->second->GetChip() << std::endl;
                    s << "Start time: " << GetStartTime() << "\n" << std::endl;
                    
                    for (FileMap::iterator f = files.begin(); f != files.end(); ++f)
                        s << f->second->GetInfo(blockInfo,chunkInfo);
                    
                    return s.str();
                }
                
                gz4d::Time GetStartTime()
                {
                    return files.begin()->second->GetBlockTime(files.begin()->first.first);
                }
                
                gz4d::Time GetBlockTime(uint16_t block)
                {
                    return GetBlockContainer(block)->GetBlockTime(block);
                }
                
                uint8_t GetID()
                {
                    return files.begin()->second->GetID();
                }
                
                BlockAddress GetFirstChunkAddress(uint16_t block)
                {
                    return GetBlockContainer(block)->GetFirstChunkAddress(block);
                }
                
                uint16_t GetStartBlock()
                {
                    return files.begin()->second->GetStartBlock();
                }
                
                uint16_t GetEndBlock()
                {
                    return files.rbegin()->second->GetEndBlock();
                }
                
                uint16_t NextBlock(uint16_t block)
                {
                    if(block >= GetEndBlock())
                        throw(Exception("End of chip"));
                    
                    for( FileMap::iterator f = files.begin(); f != files.end(); ++f)
                        if(block >= f->first.first)
                            if(block < f->first.second)
                                return f->second->NextBlock(block);
                            else if (block == f->first.second)
                            {
                                ++f;
                                return f->second->GetStartBlock();
                            }
                }
                
                uint16_t PreviousBlock(uint16_t block)
                {
                    if(block <= GetStartBlock())
                        throw(Exception("Start of chip"));
                    
                    for( FileMap::iterator f = files.begin(); f != files.end(); ++f)
                        if(block > f->first.first)
                            if(block <= f->first.second)
                                return f->second->PreviousBlock(block);
                            else if (block == f->first.first)
                            {
                                --f;
                                return f->second->GetStartBlock();
                            }
                }
                
                bool HasBlock(uint16_t block)
                {
                    return GetBlockContainer(block)->HasBlock(block);
                }
                
                

                template<typename T> T Read(BlockAddress const & address, uint32_t offset=0)
                {
                    BlockAddress start = Chip::CalculateAddress(address,offset);
                    BlockAddress end = CalculateAddress(start,sizeof(T));
                    BlockAddress endBlock = end;
                    endBlock.byte = 0;
                    if(start.block != end.block)
                    {
                        std::vector<uint8_t> ret = Read(address, sizeof(T), offset);
                        T retval = *reinterpret_cast<T const*>(&ret.front());
                        return Swap(retval);
                    }
                    else
                    {
                        return GetBlockContainer(start.block)->Read<T>(start);
                    }
                }

                template<typename T> std::vector<T> ReadVector(BlockAddress const &address, uint16_t size, uint32_t offset=0)
                {
                    std::vector<uint8_t> data = Read(address, sizeof(T)*size, offset);
                    std::vector<T> ret(size);
                    for(std::size_t i = 0; i < size; ++i)
                        ret[i] = Swap( reinterpret_cast<T const*>(&data.front())[i]);
                    return ret;
                }


                std::vector<uint8_t> Read(BlockAddress const &address, uint16_t size, uint32_t offset=0)
                {
                    BlockAddress start = Chip::CalculateAddress(address,offset);
                    BlockAddress end = CalculateAddress(start,size);
                    BlockAddress endBlock = end;
                    endBlock.byte = 0;
                    if(start.block != end.block)
                    {
                        uint16_t startSize = GetBlockContainer(start.block)->GetBlockSize()-16-start.byte;
                        std::vector<uint8_t> ret = GetBlockContainer(start.block)->Read(start,startSize);
                        std::vector<uint8_t> ret2 = GetBlockContainer(end.block)->Read(endBlock,size-startSize);
                        ret.insert(ret.end(),ret2.begin(),ret2.end()); 
                        return ret;
                    }
                    return GetBlockContainer(start.block)->Read(start,size);
                }
                
                BlockAddress CalculateAddress(BlockAddress const &address, uint32_t offset)
                {
                    BlockAddress ret = address;
                    ret.byte += offset;
                    uint32_t bs = files.begin()->second->GetBlockSize() - 16;
                    while(ret.byte >= bs)
                    {
                        ret.block =  NextBlock(ret.block);
                        ret.byte -= bs;
                    }
                    if(!GetBlockContainer(ret.block)->CheckAddress(ret))
                        ret.block = 0xffff;
                    return ret;
                }
                

        };
    }
}

#endif
