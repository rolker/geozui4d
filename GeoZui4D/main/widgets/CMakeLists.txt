set(GEOZUI4D_SRC
    ${GEOZUI4D_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/LightDirector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Sv_Matrix4.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Sv_Pointer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Sv_Quaternion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Sv_Raster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Sv_Slider3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Sv_Widgets.cpp
    PARENT_SCOPE
)

#add_library(gz_widgets STATIC ${GEOZUI4D_WIDGETS_SRC})
