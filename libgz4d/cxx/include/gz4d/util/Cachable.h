#ifndef GZ4D_CACHABLE_H
#define GZ4D_CACHABLE_H

#include<gz4d/gz4d.h>
#include<gz4d/util/Cache.h>
#error

namespace gz4d
{
    class GZ4DEXPORT Cachable
    {
        public:
            Cachable(Cache *cache);
            virtual ~Cachable();
            virtual void UnloadToCache()=0;
            virtual void LoadFromCache()=0;
        protected:
            Cache *cache;
            Cache::Id cacheId;
    };
}

#endif
