cmake_minimum_required(VERSION 2.8)
project(gz4d)

# Disable AIS (libais API has diverged)
set(ais_INCLUDE_DIR "" CACHE STRING "" FORCE)
set(ais_LIBRARY "" CACHE STRING "" FORCE)

# Build libgz4d first
add_subdirectory(libgz4d/cxx)

# Point GeoZui4D at the libgz4d we just built
set(gz4d_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libgz4d/cxx/include" CACHE PATH "")
set(gz4d_x3d_LIBRARY gz4d_x3d CACHE STRING "")

# Build GeoZui4D
add_subdirectory(GeoZui4D/main)
