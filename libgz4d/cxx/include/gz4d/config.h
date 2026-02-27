#ifndef GZ4D_CONFIG_H
#define GZ4D_CONFIG_H

#ifdef __ANDROID__
    #define __GLIBC__
    #include <boost/detail/endian.hpp>
    #undef __GLIBC__
    
    #define BOOST_FILESYSTEM_VERSION  2
#endif



#include <boost/config.hpp>

//#define BOOST_DYN_LINK
//#define BOOST_ALL_DYN_LINK

    #ifdef BOOST_MSVC
        #define NOMINMAX
    #endif
    #ifdef WIN32
        #ifdef GZ4D_DYN_LINK
            #ifdef GZ4D_INTERNAL
                #define GZ4DEXPORT __declspec(dllexport)
            #else
                #define GZ4DEXPORT __declspec(dllimport)
            #endif
        #else
            #define GZ4DEXPORT
        #endif
        #define GZ4DLOCAL
    #else
        #define GZ4DEXPORT __attribute__((visibility("default")))
        #define GZ4DLOCAL __attribute__((visibility("hidden")))
    #endif

    #ifdef BOOST_MSVC
        #define _USE_MATH_DEFINES 
        #include <float.h>

        
        //#define __STDC_LIMIT_MACROS
        //#include <stdint.h> // So msvc can have uint64_t and such
    #endif

//#include <stdint.h>
#include<boost/cstdint.hpp>
#if defined(BOOST_MSVC) && (BOOST_MSVC <= 1500)
#ifndef _MSC_STDINT_H_
	using boost::int8_t;
	using boost::uint8_t;
	using boost::int16_t;
	using boost::uint16_t;
	using boost::int32_t;
	using boost::uint32_t;
	using boost::int64_t;
	using boost::uint64_t;
#endif
#endif

#endif
