set (TEST_SOURCES
    angle.cpp
    box.cpp
    color.cpp
    geocam.cpp
    matrix.cpp
    socket.cpp
    time.cpp
    timeSequence.cpp
    URI.cpp
)

# The .asc / WKT2 .prj check needs GDAL, which libgz4d finds optionally.
if(GDAL_FOUND)
    set(TEST_SOURCES ${TEST_SOURCES} gdal_asc.cpp)
endif()

add_executable(test_libgz4d ${TEST_SOURCES})
target_link_libraries(test_libgz4d GTest::GTest GTest::Main ${LIBS})
add_test(test_libgz4d test_libgz4d)

