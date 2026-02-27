// Based on boost auto_link.h



#if defined(BOOST_MSVC)
    
#ifndef GZ4D_LIB_NAME
#  error "Macro GZ4D_LIB_NAME not set (internal error)"
#endif



//
// error check:
//
#if defined(__MSVC_RUNTIME_CHECKS) && !defined(_DEBUG)
#  pragma message("Using the /RTC option without specifying a debug runtime will lead to linker errors")
#  pragma message("Hint: go to the code generation options and switch to one of the debugging runtimes")
#  error "Incompatible build options"
#endif
//
// select toolset if not defined already:
//
#ifndef GZ4D_LIB_TOOLSET

#if defined(BOOST_MSVC) && (BOOST_MSVC == 1300)

   // vc7:
#  define GZ4D_LIB_TOOLSET "vc7"

#elif defined(BOOST_MSVC) && (BOOST_MSVC == 1310)

   // vc71:
#  define GZ4D_LIB_TOOLSET "vc71"

#elif defined(BOOST_MSVC) && (BOOST_MSVC == 1400)

   // vc80:
#  define GZ4D_LIB_TOOLSET "vc80"

#elif defined(BOOST_MSVC) && (BOOST_MSVC >= 1500)

   // vc90:
#  define GZ4D_LIB_TOOLSET "vc90"

#endif
#endif // GZ4D_LIB_TOOLSET


//
// check thread support:
//
#if !defined(_MT) && !defined(__MT__)
#  error "Non-threaded build not supported"
#endif


#if defined(_DEBUG)
#    define GZ4D_LIB_OPT "-d"
#else
#    define GZ4D_LIB_OPT
#endif

//
// select linkage opt:
//
#if (defined(_DLL) || defined(_RTLDLL)) && defined(GZ4D_DYN_LINK)
#  define GZ4D_LIB_PREFIX
#elif defined(GZ4D_DYN_LINK)
#  error "Mixing a dll library with a static runtime is a really bad idea..."
#else
#  define GZ4D_LIB_PREFIX "lib"
#endif

//
// now include the lib:
//
#if defined(GZ4D_LIB_NAME) \
      && defined(GZ4D_LIB_PREFIX) \
      && defined(GZ4D_LIB_TOOLSET) \
      && defined(GZ4D_LIB_OPT) \

//#  pragma comment(lib, GZ4D_LIB_PREFIX BOOST_STRINGIZE(GZ4D_LIB_NAME) "-" GZ4D_LIB_TOOLSET "-mt" GZ4D_LIB_OPT ".lib")
#  pragma comment(lib, BOOST_STRINGIZE(GZ4D_LIB_NAME) ".lib")
#  ifdef GZ4D_LIB_DIAGNOSTIC
#     pragma message ("Linking to lib file: " GZ4D_LIB_PREFIX BOOST_STRINGIZE(GZ4D_LIB_NAME) "-" GZ4D_LIB_TOOLSET "-mt" GZ4D_LIB_OPT ".lib")
#  endif

#else
#  error "some required macros where not defined (internal logic error)."
#endif

#endif

//
// finally undef any macros we may have set:
//
#ifdef GZ4D_LIB_PREFIX
#  undef GZ4D_LIB_PREFIX
#endif
#if defined(GZ4D_LIB_NAME)
#  undef GZ4D_LIB_NAME
#endif
#if defined(GZ4D_LIB_OPT)
#  undef GZ4D_LIB_OPT
#endif
