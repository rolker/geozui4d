// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_PIXEL_H
#define GZ4D_PIXEL_H

#include <gz4d/Vector.h>
#include <gz4d/Color.h>
#include <limits>

namespace gz4d
{
    struct PixelFormat
    {
        virtual ~PixelFormat(){}
    };

    template<std::size_t N, typename T, std::size_t R,  std::size_t G, std::size_t B, std::size_t A=N, std::size_t L=N> struct VectorPixelFormat: public PixelFormat
    {
        typedef Vector<T, N> container_type;
        typedef T raw_type;
        static const std::size_t raw_size = N;
        

        BOOST_STATIC_ASSERT(!((R<N||G<N||B<N)&&L<N));

        size_t ChannelSize() const {return sizeof(T);}
        size_t ChannelCount() const {return N;}
        bool HasAlpha() const {return A < N;}
        bool HasLuminance() const {return L < N;}
        Vector<T,N> InitialValue() const {return Vector<T,N>(std::numeric_limits<T>::max());}
        void SetColor(Vector<T,N> &pixel, Color const &color) const;
        Color operator()(Vector<T, N> const &pixel) const;
        static std::string String()
        {
            std::string ret;
            ret.resize(N);
            if(R<N)
                ret[R]='R';
            if(G<N)
                ret[G]='G';
            if(B<N)
                ret[B]='B';
            if(A<N)
                ret[A]='A';
            if(L<N)
                ret[L]='L';
            ret += boost::lexical_cast<std::string>(8*sizeof(T));
            return ret;
        }
        
        bool operator()(std::string const &f) const {return f == String();}
    };
    
    template<std::size_t N, std::size_t R,  std::size_t G, std::size_t B, std::size_t A, std::size_t L> struct VectorPixelFormat<N,float,R,G,B,A,L>
    {
        typedef Vector<float, N> container_type;
        typedef float raw_type;
        
        size_t ChannelSize() const {return sizeof(float);}
        size_t ChannelCount() const {return N;}
        bool HasAlpha() const {return A < N;}
        bool HasLuminance() const {return L < N;}
        Vector<float,N> InitialValue() const {return Vector<float,N>(1.0);}
        
        
        void SetColor(Vector<float,N> &pixel, Color const &color) const
        {
            float a = 1.0;
            if(HasAlpha())
            {
                pixel[A] = color[3];
            }
            else
                a = color[3];
            
            if(HasLuminance()) // convert to grey scale
            {
                pixel[L] = (color[0]+color[1]+color[2])/3.0;
            }
            else
            {
                if(R<N)
                    pixel[R] = color[0];
                if(G<N)
                    pixel[G] = color[1];
                if(B<N)
                    pixel[B] = color[2];
            }
        }
        
        Color operator()(Vector<float,N> const &pixel) const
        {
            Color ret = Black();
            if(VectorPixelFormat<N,float,R,G,B,A,L>::HasAlpha())
                ret[3] = pixel[A];
            else
                ret[3] = 1.0;
            
            if(VectorPixelFormat<N,float,R,G,B,A,L>::HasLuminance())
            {
                ret[0] = pixel[L];
                ret[1] = pixel[L];
                ret[2] = pixel[L];
            }
            else
            {
                if(R<N)
                    ret[0] = pixel[R];
                if(G<N)
                    ret[1] = pixel[G];
                if(B<N)
                    ret[2] = pixel[B];
            }
            return ret;
        }
        
    };
    
    
    template<std::size_t N, typename T, std::size_t R,  std::size_t G, std::size_t B, std::size_t A, std::size_t L> inline void VectorPixelFormat<N,T,R,G,B,A,L>::SetColor(Vector<T,N> &pixel, Color const &color) const
    {
        float a = 1.0;
        if(HasAlpha())
        {
            pixel[A] = std::min(std::max(float(0.0),color[3]),float(1.0))*std::numeric_limits<T>::max();
        }
        else
            a = color[3];

        if(HasLuminance()) // convert to grey scale
        {
            float g = (color[0]+color[1]+color[2])/3.0;
            pixel[L] = std::min(std::max(float(0.0),g*a),float(1.0))*std::numeric_limits<T>::max();
        }
        else
        {
            if(R<N)
                pixel[R] = std::min(std::max(float(0.0),color[0]*a),float(1.0))*std::numeric_limits<T>::max();
            if(G<N)
                pixel[G] = std::min(std::max(float(0.0),color[1]*a),float(1.0))*std::numeric_limits<T>::max();
            if(B<N)
                pixel[B] = std::min(std::max(float(0.0),color[2]*a),float(1.0))*std::numeric_limits<T>::max();
        }
    }

    template<std::size_t N, typename T, std::size_t R,  std::size_t G, std::size_t B, std::size_t A, std::size_t L> inline Color VectorPixelFormat<N,T,R,G,B,A,L>::operator()(Vector<T,N> const &pixel) const
    {
        Color ret = Black();
        if(HasAlpha())
            ret[3] = pixel[A]/float(std::numeric_limits<T>::max());
        else
            ret[3] = 1.0;

        if(HasLuminance())
        {
            float g = pixel[L]/float(std::numeric_limits<T>::max());
            ret[0] = g;
            ret[1] = g;
            ret[2] = g;
        }
        else
        {
            if(R<N)
                ret[0] = pixel[R]/float(std::numeric_limits<T>::max());
            if(G<N)
                ret[1] = pixel[G]/float(std::numeric_limits<T>::max());
            if(B<N)
                ret[2] = pixel[B]/float(std::numeric_limits<T>::max());
        }
        return ret;
    }

    

    typedef VectorPixelFormat<3,uint8_t,0,1,2> RGB8;
    typedef VectorPixelFormat<3,uint8_t,2,1,0> BGR8;
    typedef VectorPixelFormat<4,uint8_t,0,1,2,3> RGBA8;
    typedef VectorPixelFormat<3,uint16_t,0,1,2> RGB16;
    typedef VectorPixelFormat<4,uint16_t,0,1,2,3> RGBA16;
    typedef VectorPixelFormat<1,uint8_t,1,1,1,1,0> L8;
    typedef VectorPixelFormat<2,uint8_t,2,2,2,1,0> LA8;
    typedef VectorPixelFormat<1,uint16_t,1,1,1,1,0> L16;
    typedef VectorPixelFormat<2,uint16_t,2,2,2,1,0> LA16;
    typedef VectorPixelFormat<1,float,1,1,1,1,0> L32;
    
    struct YUV8: public PixelFormat
    {
        typedef Vector<uint8_t,3> container_type;
        typedef uint8_t raw_type;
        static const std::size_t raw_size = 3;
        
        
        size_t ChannelSize() const {return sizeof(uint8_t);}
        size_t ChannelCount() const {return 3;}
        bool HasAlpha() const {return false;}
        bool HasLuminance() const {return true;}
        container_type InitialValue() const {return container_type(235,128,128);}
        void SetColor(container_type &pixel, Color const &color) const
        {
            pixel[0] = 16+65.738*color[0]+129.057*color[1]+25.064*color[2];
            pixel[1] = 128-37.945*color[0]-74.494*color[1]+112.439*color[2];
            pixel[2] = 128+112.439*color[0]-94.154*color[1]-18.285*color[2];
        }
        Color operator()(container_type const &pixel) const
        {
            double y = 0.004566207  *pixel[0];
            double cb = pixel[1]/256.0;
            double cr = pixel[2]/256.0;
            return Color(y+1.602286275*cr-0.8742, y-0.393298039*cb-0.816156863*cr+0.531670588, y+2.025145098*cb-1.085631373);
        }
        std::string String() const {return "YUV8";}
        bool operator()(std::string const &f) const {return f == "YUV8";}

    };
    
    struct RGBA5_5_5_1: public PixelFormat
    {
        typedef Vector<uint16_t, 1> container_type;
        typedef uint8_t raw_type;
        static const std::size_t raw_size = 2;
        
#ifdef GL_UNSIGNED_SHORT_5_5_5_1
        size_t ChannelSize() const {return GL_UNSIGNED_SHORT_5_5_5_1;}
#else
        size_t ChannelSize() const {return 2;}
#endif
        size_t ChannelCount() const {return 4;}
        bool HasAlpha() const {return true;}
        bool HasLuminance() const {return false;}
        container_type InitialValue() const {return container_type(std::numeric_limits<uint16_t>::max());}
        void SetColor(container_type &pixel, Color const &color) const;
        Color operator()(container_type const &pixel) const;
        std::string String() const {return "RGBA_5_5_5_1";}
        bool operator()(std::string const &f) const {return f == String();}
    };

    inline void RGBA5_5_5_1::SetColor(RGBA5_5_5_1::container_type &pixel, Color const &color) const
    {
        const int max = 31;
        uint16_t val = 0;
        val |= static_cast<uint8_t>(color[0] * max + 0.5) << 11; //Red
        val |= static_cast<uint8_t>(color[1] * max + 0.5) << 6;//Green
        val |= static_cast<uint8_t>(color[2] * max + 0.5) << 1;//Blue
        val = (color[3] >= 0.5 ? 1 : 0); //Alpha
        pixel[0] = val;
    }

    inline Color RGBA5_5_5_1::operator()(RGBA5_5_5_1::container_type const &pixel) const
    {
        const int mask = 31;
        float max = 31.0;
        uint16_t val = pixel[0];

        Color ret = Black();
        ret[0] = ((val >> 11) & mask) / max; //Red
        ret[1] = ((val >> 6) & mask) / max; //Green
        ret[2] = ((val >> 1) & mask) / max; //Blue
        ret[3] = val & 1; //Alpha
        return ret;
    }

    template<typename PF> class Pixel: public PF::container_type
    {
        public:
            typedef PF pixel_format;
            Pixel():PF::container_type(PF().InitialValue()){}
            Pixel(typename PF::container_type const &p):PF::container_type(p){}
            explicit Pixel(Color const &c){PF().SetColor(*this,c);}

            Pixel const &operator=(Color const &c);
            template <typename OF>  Pixel const &operator=(Pixel<OF> const &other);
            operator Color() const;
    };

    template<typename PF> inline Pixel<PF> const &Pixel<PF>::operator=(Color const &c)
    {
        PF().SetColor(*this,c);
        return *this;
    }

    template<typename PF> template <typename OF> inline Pixel<PF> const &Pixel<PF>::operator=(Pixel<OF> const &other)
    {
        Color c = other;
        *this = c;
        return *this;
    }

    template<typename PF> inline Pixel<PF>::operator Color() const
    {
        return PF()(*this);
    }

/*    template<std::size_t N, typename T=uint8_t> class Pixel: public Vector<T, N>
    {
        public:
            typedef T basetype;

            Pixel(T val=std::numeric_limits<T>::max()) :Vector<T, N>(val) {}

            Pixel const &operator=(Color<N> const &c);
            template <std::size_t ON, typename OT>  Pixel const &operator=(Pixel<ON,OT> const &other);
            operator Color<N>() const;
    };

    template<std::size_t N, typename T> inline Pixel<N,T> const &Pixel<N,T>::operator=(Color<N> const &c)
    {
        for(std::size_t i = 0; i < N; ++i)
            Vector<unsigned char,N>::values[i] = std::min(std::max(float(0.0),c[i]),float(1.0))*std::numeric_limits<T>::max();
        return *this;
    }

    template<std::size_t N, typename T> template <std::size_t ON, typename OT> inline Pixel<N,T> const &Pixel<N,T>::operator=(Pixel<ON,OT> const &other)
    {
        Color<ON> oc = other;
        Color<N> c = oc;
        *this = c;
        return *this;
    }

    template<std::size_t N, typename T> inline Pixel<N,T>::operator Color<N>() const
    {
        Color<N> ret;
        for(std::size_t i = 0; i < N; ++i)
            ret[i] = Vector<T,N>::values[i]/float(std::numeric_limits<T>::max());
        return ret;
    }

    typedef Pixel<3> PixelRGB8;
    typedef Pixel<4> PixelRGBA8;
    typedef PixelRGB8 PixelRGB;
    typedef PixelRGBA8 PixelRGBA;
    typedef Pixel<3,uint16_t> PixelRGB16;
    typedef Pixel<4,uint16_t> PixelRGBA16;*/
}

#endif
