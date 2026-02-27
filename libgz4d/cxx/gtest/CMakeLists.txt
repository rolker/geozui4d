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


message(${GTEST_LIBS})

add_executable(test_libgz4d ${TEST_SOURCES})
target_link_libraries(test_libgz4d ${GTEST_LIBS} ${LIBS})
add_test(test_libgz4d test_libgz4d)

