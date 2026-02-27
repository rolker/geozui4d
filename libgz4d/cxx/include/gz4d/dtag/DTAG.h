#ifndef GZ4D_DTAG_DTAG_H
#define GZ4D_DTAG_DTAG_H

#include <gz4d/gz4d.h>
#include <gz4d/time.h>
#include <gz4d/dtag/DataStream.h>
#include <gz4d/dtag/Chunk.h>

#include <vector>
#include <string>
#include <memory>
#include <boost/filesystem.hpp>


namespace gz4d
{
    namespace dtag
    {
        class Chip;
        typedef std::map<uint8_t, std::shared_ptr<Chip> > ChipMap;

        /// Encapsulates all the data from one DTAG.
        /// Contains the collection of .dtg files downloaded
        /// from a DTAG
        class  DTAG
        {
            ChipMap chips;
            float utcOffset;
            typedef std::map<uint16_t,std::shared_ptr<DataStream> > StreamMap;
            StreamMap streams;
            gz4d::Time endTime;

            void AddFile(std::string const &fileName)
            {
                std::shared_ptr<File> dtag(new File(fileName));
                uint8_t chipNumber = dtag->GetChip();
                if(!chips[chipNumber])
                {
                    std::shared_ptr<Chip> c(new Chip);
                    chips[chipNumber] = c;
                }
                chips[chipNumber]->AddFile(dtag);
            }
            
            void InitStreams()
            {
                std::cerr << "gz4d::dtag::DTAG::InitStreams";
                for(ChipMap::iterator c = chips.begin(); c != chips.end(); ++c)
                {
                    Chunk chunk(c->second,c->second->GetFirstChunkAddress(c->second->GetStartBlock()));
                    //Chunk chunk = GetFirstChunk();
                    while(chunk && chunk.IsConfig())
                    {
                        if(!streams[chunk.GetSourceID()])
                        {
                            streams[chunk.GetSourceID()] = std::shared_ptr<DataStream>(new DataStream(chunk.GetSourceID()));
                            streams[chunk.GetSourceID()]->SetConfig(chunk);
                            streams[chunk.GetSourceID()]->SetStartTime(GetStartTime());
                        }
                        else
                            streams[chunk.GetSourceID()]->AddConfig(chunk);
                        chunk = chunk.Next();
                    }
                }
                std::cerr << " done." << std::endl;
                Chunk chunk = GetLastChunk();
                endTime = Time(GetStartTime() + time::Duration(chunk.GetStartTime()/1000.0));
            }
            
            Chunk GetFirstChunk()
            {
                return Chunk(chips.begin()->second, chips.begin()->second->GetFirstChunkAddress(chips.begin()->second->GetStartBlock()));
            }
            
            Chunk GetLastChunk()
            {
                Chunk ret;
                uint16_t b = chips.rbegin()->second->GetEndBlock();
                while(true)
                {
                    try
                    {
                        Chunk next = Chunk(chips.rbegin()->second, chips.rbegin()->second->GetFirstChunkAddress(b));
                        do
                        {
                            ret = next;
                            next = next.Next();
                            if (next.GetAddress().block > b)
                                break;
                        }
                        while(next);
                        break;
                    }
                    catch (Exception e)
                    {
                        b = chips.rbegin()->second->PreviousBlock(b);
                    }
                }
                return ret;
            }
            
            public:
                typedef std::shared_ptr<DTAG> Ptr;

                DTAG(std::string const &dir)
                :utcOffset(0.0)
                {
                    boost::filesystem::path directory(dir);
                    if (is_directory(directory))
                    {
                        boost::filesystem::directory_iterator end_iter;
                        for (boost::filesystem::directory_iterator dir_itr(directory); dir_itr != end_iter; ++dir_itr)
                        {
                            if(dir_itr->path().extension()==".dtg" || dir_itr->path().extension()==".DTG")
                                AddFile(dir_itr->path().string());
                        }
                    }
                    else
                    {
                        throw(Exception("Not a valid directory"));
                    }
                    InitStreams();
                }
                
                DTAG(std::vector<std::string> const &fileList)
                :utcOffset(0.0)
                {
                    for (std::vector<std::string>::const_iterator f = fileList.begin(); f != fileList.end(); ++f)
                        AddFile(*f);
                }
                

                std::string GetInfo(bool blockInfo = false, bool chunkInfo = false)
                {
                    std::stringstream s;
                    if (chips.size() == 0)
                        s << "No valid dtg files\n";
                    else
                    {
                        s << "\nStart time: " << GetStartTime();
                        s << "\nEnd time: " << GetEndTime();
                        
                        s << "\t" << chips.size() << " chips\n\n";
                        for (ChipMap::iterator c = chips.begin(); c  != chips.end(); ++c)
                        {
                            s << c->second->GetInfo(blockInfo, chunkInfo);
                            if(chunkInfo)
                            {
                                Chunk chunk(c->second,c->second->GetFirstChunkAddress(c->second->GetStartBlock()));
                                while(chunk)
                                {
                                    s << "  " << chunk.GetInfo() << std::endl;
                                    uint32_t b = chunk.GetAddress().block;
                                    while(chunk && chunk.GetAddress().block == b)
                                        chunk = chunk.Next();
                                }
                            }
                        }
                        s << "\nStreams:" << std::endl;
                        for (StreamMap::iterator si = streams.begin(); si != streams.end(); ++si)
                            s << si->second->GetInfo();
                    }
                    
                    return s.str();
                }
                
                gz4d::Time GetStartTime()
                {
                    if(!chips.empty())
                        return chips.begin()->second->GetStartTime();
                }
                
                gz4d::Time GetEndTime() const {return endTime;}

                int GetID()
                {
                    return chips.begin()->second->GetID();
                }
                
                

                void SetTimeOffset(float offset)
                {
                    utcOffset = offset;
                }
                
                template <typename T> bool GetStream(T &stream)
                {
                    if(streams[stream.GetSourceID()])
                    {
                        stream = *streams[stream.GetSourceID()];
                        return true;
                    }
                    return false;
                }
        };
    }
}

#endif
