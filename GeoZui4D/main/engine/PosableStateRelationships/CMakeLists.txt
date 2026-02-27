set(GEOZUI4D_SRC
    ${GEOZUI4D_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/AveragingOverview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Boundary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CompositeObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PosableStateLock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ProximityHighlighter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RelationshipManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RelationshipQueryResult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WindowLock.cpp
    PARENT_SCOPE
)

#add_library(gz_engine_psr STATIC ${GEOZUI4D_ENGINE_PSR_SRC})