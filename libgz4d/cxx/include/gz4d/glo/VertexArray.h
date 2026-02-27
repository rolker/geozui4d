#ifndef GZ4D_GLO_VERTEXARRAY_H
#define GZ4D_GLO_VERTEXARRAY_H

#include <gz4d/glo.h>
#include <gz4d/glo/Error.h>

namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        class VertexArray
        {
#pragma pack(1)
            template<typename T> struct C3
            {
                T r,g,b;
            };
            template<typename T> struct C4: public C3<T>
            {
                T a;
            };
            struct N3F
            {
                float nx,ny,nz;
            };
            struct T2F
            {
                float ts,tt;
            };
            struct T4F: public T2F
            {
                float tr,tq;
            };

            public:
                struct V2F
                {
                    float x,y;
                };
                struct V3F: public V2F
                {
                    float z;
                };
                struct V4F: public V3F
                {
                    float w;
                };

                struct C4UB_V2F: public C4<uint8_t>, public V2F{};
                struct C4UB_V3F: public C4<uint8_t>, public V3F{};
                struct C3F_V3F: public C3<float>, public V3F{};
                struct N3F_V3F: public N3F, public V3F{};
                struct C4F_N3F_V3F: public C4<float>, public N3F, public V3F{};
                struct T2F_V3F: public T2F, public V3F{};
                struct T4F_V4F: public T4F, public V4F{};
                struct T2F_C4UB_V3F: public T2F, public C4<uint8_t>, public V3F{};
                struct T2F_C3F_V3F: public T2F, public C3<float>, public V3F{};
                struct T2F_N3F_V3F: public T2F, public N3F, public V3F{};
                struct T2F_C4F_N3F_V3F: public T2F, public C4<float>, public N3F, public V3F{};
                struct T4F_C4F_N3F_V4F: public T4F, public C4<float>, public N3F, public V4F{};
#pragma pack()


        };
    }
}

#endif
