#ifndef GZ4D_CACHE_H
#define GZ4D_CACHE_H

#include<gz4d/gz4d.h>
#include<cstdio>
#include<map>
#error

namespace gz4d
{
    /// \ingroup utilities
    class GZ4DEXPORT Cache
    {
        public:
            typedef unsigned long Id;
            Cache(unsigned long blockSize = 8192);
            ~Cache();
            operator FILE*(){return file;}

            /// Request cache space.
            /// Returns an id to use for later access and release of allocated blocks.
            /// Enough sequential blocks are allocated to 
            Id Allocate(unsigned long size);

            /// Release allocated space.
            /// Allows for the space to be reclaimed.
            void Release(Id id);
            /// Seeks to location allocated to id.
            void Seek(Id id);

            /// Prints out diagnostic info to stderr.
            void PrintDebug();
        protected:
            typedef unsigned long BlockId;
            typedef unsigned long BlockCount;
            struct Allocation
            {
                BlockId startBlock;
                BlockCount size;
            };

            void RemoveFromFreeSpace(std::map<BlockId, BlockCount>::iterator block);

            FILE* file;
            unsigned long blockSize;

            std::map<Id,Allocation> blockAllocationTable; ///< maps Ids to allocated blocks.
            std::map<BlockId, BlockCount> deallocatedBlockTable; ///< keeps track of freed space by block id.
            std::multimap<BlockCount,BlockId> freeSpaceTable; ///< keeps track of free space by size.

            Id nextId; ///< next available allocation id.
            BlockId nextBlock; ///< next available block.
    };
}

#endif
