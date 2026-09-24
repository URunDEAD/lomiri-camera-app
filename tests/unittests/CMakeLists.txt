find_program(XVFB_RUN_BIN
    NAMES xvfb-run
)

set(XVFB_RUN_CMD ${XVFB_RUN_BIN} -a -s "-screen 0 1024x768x24")

find_package(Qt5Test REQUIRED)
find_package(Qt5QuickTest REQUIRED)

add_executable(tst_QmlTests tst_QmlTests.cpp)
target_link_libraries(tst_QmlTests
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    Qt5::Test
    Qt5::QuickTest
    ${TPL_QT5_LIBRARIES}
)
add_test(tst_QmlTests ${XVFB_RUN_CMD} ${CMAKE_CURRENT_BINARY_DIR}/tst_QmlTests -import ${CMAKE_SOURCE_DIR} -import ${CMAKE_BINARY_DIR})

# copy qml test files to build dir
file(GLOB qmlTestFiles RELATIVE ${CMAKE_SOURCE_DIR}/tests/unittests/ *qml)
foreach(qmlTestFile ${qmlTestFiles})
  file(COPY ${qmlTestFile} DESTINATION ${CMAKE_BINARY_DIR}/tests/unittests/)
endforeach(qmlTestFile)

if(NOT CTEST_TESTING_TIMEOUT)
    set(CTEST_TESTING_TIMEOUT 60)
endif()

include_directories(
    ${CMAKE_BINARY_DIR}
    ${camera_plugin_SOURCE_DIR}
    )

add_executable(tst_storagemonitor
    tst_storagemonitor.cpp
    ${camera_plugin_SOURCE_DIR}/storagemonitor.cpp
    qstorageinfo_stub.cpp
    storageinfocontrol.cpp
    )

add_test(tst_storagemonitor tst_storagemonitor -xunitxml -o test.xml)
set_tests_properties(tst_storagemonitor PROPERTIES
    TIMEOUT ${CTEST_TESTING_TIMEOUT}
    ENVIRONMENT "QT_QPA_PLATFORM=minimal"
    )

target_link_libraries(tst_storagemonitor
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
    Qt5::Test
    Qt5::Widgets
    camera-qml
    stdc++
)
