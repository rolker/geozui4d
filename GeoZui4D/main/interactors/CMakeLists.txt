set(GEOZUI4D_SRC
    ${GEOZUI4D_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/Animator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DoubleClickInteractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EventHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ImitatingAnimator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InputTentacle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PathAnimator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Pickable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PickInteractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PlanarPointHandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PlaybackTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PointHandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PositionDrag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RecordingTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RubberbandInteractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RulerTool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScaleDrag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SDV_Cursor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Tool3D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Tool3DPallette.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ValueDrag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VectorAnimator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ViewNudger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WindowAnchor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZoomInteractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZoomWindowTool.cpp
    PARENT_SCOPE
)

#add_library(gz_interactors STATIC ${GEOZUI4D_INTERACTORS_SRC})

