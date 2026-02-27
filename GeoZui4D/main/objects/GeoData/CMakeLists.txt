set(GEOZUI4D_SRC
    ${GEOZUI4D_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/Cone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Cube.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Cylinder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DataLines.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DataPoints.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DataTriangles.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DoublePyramid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Glyph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InvertedCone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SphereGlyph.cpp
    PARENT_SCOPE
)

#add_library(gz_objects_geodata ${GEOZUI4D_OBJECTS_GEODATA_SRC})
