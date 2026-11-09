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

add_executable(test_libgz4d ${TEST_SOURCES})
target_link_libraries(test_libgz4d GTest::GTest GTest::Main ${LIBS})
add_test(test_libgz4d test_libgz4d)

