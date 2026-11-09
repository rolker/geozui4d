project(GeoZui4D)

cmake_minimum_required(VERSION 3.14)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

find_path(gz4d_INCLUDE_DIR gz4d/gz4d.h)
if (gz4d_INCLUDE_DIR)
    include_directories(${gz4d_INCLUDE_DIR})
else()
    message(FATAL_ERROR,"libgz4d includes not found!")
endif()

foreach(GZLIB IN ITEMS gz4d_x3d)
    find_library(${GZLIB}_LIBRARY ${GZLIB})
    if(${GZLIB}_LIBRARY)
        set(LIBS ${LIBS} ${${GZLIB}_LIBRARY})
    else()
        message(FATAL_ERROR,"libgz4d library ${GZLIB} not found")
    endif()
endforeach()

if(NOT CMAKE_CROSSCOMPILING)
    find_path(spnav_INCLUDE_DIR spnav.h)
    find_library(spnav_LIBRARY spnav)
    if(spnav_INCLUDE_DIR AND spnav_LIBRARY)
        set(LIBS ${LIBS} ${spnav_LIBRARY})
    else()
        message(FATAL_ERROR,"spnav library (spaceball) not found")
    endif()
endif()

set(boost_components system program_options iostreams)
if(CMAKE_CROSSCOMPILING)
    set(boost_components ${boost_components} thread_win32)
else()
    set(boost_components ${boost_components} thread)
endif()

find_package(Boost 1.37.0 COMPONENTS ${boost_components} REQUIRED)
if(NOT MSVC)
    set(LIBS ${LIBS} ${Boost_LIBRARIES})
endif()

include_directories(${Boost_INCLUDE_DIR})

add_definitions(-DSTATIC_BUILD)
find_package(TCL REQUIRED)

include_directories(${TCL_INCLUDE_PATH} ${TK_INCLUDE_PATH})
set(LIBS ${LIBS} ${TCL_LIBRARY} ${TK_LIBRARY})

find_package(TclStub REQUIRED)
set(LIBS ${LIBS} ${TCL_STUB_LIBRARY} ${TK_STUB_LIBRARY})


find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIR})
set(LIBS ${LIBS} ${GDAL_LIBRARY})

find_package(JPEG)
if(JPEG_FOUND)
    set(LIBS ${LIBS} ${JPEG_LIBRARIES})
endif()

find_package(Jasper)
if(JASPER_FOUND)
    set(LIBS ${LIBS} ${JASPER_LIBRARIES})
endif()

find_package(TIFF)
if(TIFF_FOUND)
    set(LIBS ${LIBS} ${TIFF_LIBRARIES})
endif()

find_package(PNG)
if(PNG_FOUND)
    add_definitions(${PNG_DEFINITIONS})
    set(LIBS ${LIBS} ${PNG_LIBRARIES})
endif()

find_path(portaudio_INCLUDE_DIR portaudio.h)
find_library(portaudio_LIBRARY portaudio)
if(portaudio_INCLUDE_DIR AND portaudio_LIBRARY)
    set(portaudio_FOUND TRUE)
    include_directories(${portaudio_INCLUDE_DIR})
    set(LIBS ${LIBS} ${portaudio_LIBRARY})
else()
    set(portaudio_FOUND FALSE)
endif()

find_library(jack_LIBRARY jack)
if(jack_LIBRARY)
    set(LIBS ${LIBS} ${jack_LIBRARY})
endif()

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(AVFORMAT libavformat)
    pkg_check_modules(AVCODEC libavcodec)
    pkg_check_modules(AVUTIL libavutil)
endif()
if(AVFORMAT_FOUND AND AVCODEC_FOUND AND AVUTIL_FOUND)
    include_directories(${AVFORMAT_INCLUDE_DIRS} ${AVCODEC_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${AVUTIL_LIBRARIES})
    add_definitions(-DGZ4D_AV)
endif()

find_library(avcore_LIBRARY avcore)
if(avcore_LIBRARY)
    set(LIBS ${LIBS} ${avcore_LIBRARY})
endif()

#find_path(vorbis_INCLUDE_DIR vorbis/vorbisenc.h)
#find_library(vorbis_LIBRARY vorbis)
find_path(vorbisfile_INCLUDE_DIR vorbis/vorbisfile.h)
find_library(vorbisfile_LIBRARY vorbisfile)
if(vorbisfile_INCLUDE_DIR AND vorbisfile_LIBRARY)
    include_directories(${vorbisfile_INCLUDE_DIR})
    set(LIBS ${LIBS} ${vorbisfile_LIBRARY})
else()
    message("Vorbis not found.")
endif()

find_path(bluetooth_INCLUDE_DIR bluetooth/bluetooth.h)
find_library(bluetooth_LIBRARY bluetooth)
if(bluetooth_INCLUDE_DIR AND bluetooth_LIBRARY)
    set(LIBS ${LIBS} ${bluetooth_LIBRARY})
else()
    message("Bluetooth not found.")
endif()


find_package(GLUT)
if(GLUT_FOUND)
    add_definitions(-DFREEGLUT_STATIC -DGLUT_DISABLE_ATEXIT_HACK)
    set(LIBS ${LIBS} ${GLUT_LIBRARIES})
endif()

find_path(GLEW_INCLUDE_DIR GL/glew.h)
find_library(GLEW_LIBRARY GLEW)
if(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
    set(GLEW_FOUND TRUE)
    set(LIBS ${LIBS} ${GLEW_LIBRARY})
    add_definitions(-DGZ4D_GLEW)
else()
    set(GLEW_FOUND FALSE)
endif()


# Prefer legacy libGL over GLVND libOpenGL (see libgz4d/cxx/CMakeLists.txt)
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL)
if(OPENGL_FOUND)
    set(LIBS ${LIBS} ${OPENGL_LIBRARIES})
endif()


find_path(netcdf_INCLUDE_DIR netcdf.h)
find_library(netcdf_LIBRARY NAMES netcdf netcdfs)
if(netcdf_INCLUDE_DIR AND netcdf_LIBRARY)
    set(netcdf_FOUND TRUE)
    set(LIBS ${LIBS} ${netcdf_LIBRARY})
else()
    set(netcdf_FOUND FALSE)
    message("netcdf not found")
endif()

find_path(netcdf_cpp_INCLUDE_DIR ncFile.h)
find_library(netcdf_cpp_LIBRARY netcdf_c++4)
if(netcdf_cpp_INCLUDE_DIR AND netcdf_cpp_LIBRARY)
    set(netcdf_cpp_FOUND TRUE)
    set(LIBS ${LIBS} ${netcdf_cpp_LIBRARY})
else()
    set(netcdf_cpp_FOUND FALSE)
    message("netcdf_cpp not found")
endif()

find_path(ni_INCLUDE_DIR XnCppWrapper.h PATHS /usr/include/ni)
find_library(ni_LIBRARY OpenNI)
if(ni_INCLUDE_DIR AND ni_LIBRARY)
    add_definitions(-DGZ4D_KINECT)
    include_directories(${ni_INCLUDE_DIR})
    set(LIBS ${LIBS} ${ni_LIBRARY})
else()
    message("ni includes not found.")
endif()


if(CMAKE_CROSSCOMPILING)
    set(LIBS ${LIBS} winmm ws2_32 imm32 comctl32)
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR})


add_subdirectory(engine)
add_subdirectory(interactors)
add_subdirectory(objects)

add_subdirectory(Togl)
set(LIBS ${LIBS} gz_togl ${TCL_STUB_LIBRARY} ${TK_STUB_LIBRARY})

find_package(X11)
if(X11_FOUND)
    set(LIBS ${LIBS} ${X11_LIBRARIES})
endif()

add_subdirectory(utilities)
add_subdirectory(widgets)


# set(GZ_LIBS
#     gz_engine
#     gz_engine_psr
#     gz_engine_texture
#     gz_interactors
#     gz_objects
#     gz_objects_dxf
#     gz_objects_geodata
#     gz_objects_nodetypes
#     gz_objects_ogr
#     gz_objects_s57
#     gz_objects_sonar
#     gz_togl
#     gz_utilities
#     gz_widgets
# )

SET(CMAKE_SKIP_BUILD_RPATH  TRUE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(GEOZUI4D_SRC ${GEOZUI4D_SRC} main.cpp MainWindow.cpp SubWindow.cpp)

if(CMAKE_CROSSCOMPILING)
    # RC compiler
    enable_language(RC)
    find_path(tk_res_INCLUDE_DIR tk_base.rc PATH_SUFFIXES tk/rc)
    if(NOT tk_res_INCLUDE_DIR)
        message(FATAL_ERROR,"Could not find tk_base.rc")
    endif()
    include_directories(${tk_res_INCLUDE_DIR})
    set(GEOZUI4D_SRC ${GEOZUI4D_SRC} GeoZui4D.rc)

    find_path(init_tcl_DIR init.tcl PATH_SUFFIXES lib/tcl8.5)
    if(NOT init_tcl_DIR)
        message(FATAL_ERROR,"Could not find init.tcl directory")
    endif()
    install(DIRECTORY ${init_tcl_DIR} DESTINATION lib)

    find_path(tk_tcl_DIR tk.tcl PATH_SUFFIXES lib/tk8.5)
    if(NOT tk_tcl_DIR)
        message(FATAL_ERROR,"Could not find tk.tcl directory")
    endif()
    install(DIRECTORY ${tk_tcl_DIR} DESTINATION lib)
    
    set(CPACK_PACKAGE_EXECUTABLES "GeoZui4D" "GeoZui4D")

endif()

# Version from git: commit count (used as the CPack patch) and short sha.
# Falls back to a count of 0 when git or the repository is not available.
# The repository is not checked to be this project's: a tarball unpacked
# inside another git checkout takes that checkout's count and sha, and a
# shallow clone counts only the commits it has.
set(GZ4D_COMMIT_COUNT 0)
set(GZ4D_COMMIT_SHA "")
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE gz4d_git_count
        RESULT_VARIABLE gz4d_git_count_result
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE gz4d_git_sha
        RESULT_VARIABLE gz4d_git_sha_result
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    if(gz4d_git_count_result EQUAL 0 AND gz4d_git_sha_result EQUAL 0)
        set(GZ4D_COMMIT_COUNT ${gz4d_git_count})
        set(GZ4D_COMMIT_SHA ${gz4d_git_sha})
    endif()
endif()
if(GZ4D_COMMIT_SHA)
    set(gz4d_version "4.0.${GZ4D_COMMIT_COUNT}-g${GZ4D_COMMIT_SHA}")
else()
    set(gz4d_version "unknown")
endif()
message("version: ${gz4d_version}")

add_definitions(-DGZ4D_VERSION="${gz4d_version}")

add_executable(GeoZui4D ${GEOZUI4D_SRC})
target_link_libraries(GeoZui4D ${LIBS})
install(TARGETS GeoZui4D RUNTIME DESTINATION bin)

install(FILES
    blank.cur
    clutEditor.tcl
    CompassPoints.tif
    Compass_rose_pale.tif
    fullmenu.tcl
    GeoZui_splash.gif
    GeoZui.tcl
    notes.tcl
    subtogl.tcl
    tclLinkage.h
    visualizer_procs.tcl
    visualizer.tcl
    watermark.tif
DESTINATION lib/GeoZui4D)

install(DIRECTORY bitmaps DESTINATION lib/GeoZui4D PATTERN ".svn" EXCLUDE PATTERN "SConscript" EXCLUDE)

install(DIRECTORY BWidget-1.7.0 DESTINATION lib/GeoZui4D FILES_MATCHING
    PATTERN "*.tcl"
    PATTERN "*.xbm"
    PATTERN "*.gif"
    PATTERN "*.rc"
    PATTERN ".svn" EXCLUDE
    )

add_subdirectory(gzmod)
add_subdirectory(scripts)


set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GeoZui4D research visualization software")
set(CPACK_PACKAGE_VENDOR "Data Visualization Research Lab, Center for Coastal and Ocean Mapping, University of New Hampshire")
set(CPACK_PACKAGE_CONTACT "roland@ccom.unh.edu")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "4")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH ${GZ4D_COMMIT_COUNT})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "GeoZui4D")
if(NOT CMAKE_CROSSCOMPILING)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_NAME "GeoZui4D")
    set(CPACK_PACKAGE_FILE_NAME "GeoZui4D_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${CPACK_PACKAGE_VERSION_PATCH}_i386")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgz4d, tcl8.5, tk8.5, libflac++6, libvorbisenc2")
    if(NOT TARGET deb)
        add_custom_target(deb fakeroot make package)
    endif()
else()
    set(CPACK_GENERATOR "NSIS")
endif()

include(CPack)

