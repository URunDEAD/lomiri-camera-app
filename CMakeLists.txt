cmake_minimum_required(VERSION 3.10)
project(lomiri-camera-app VERSION 4.2.0 LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Standard install paths. Needs to be set before finding packages.
include(GNUInstallDirs)

find_package(PkgConfig)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5Multimedia)
find_package(Qt5Gui)
find_package(Qt5Svg)
find_package(Qt5QuickControls2)

if(PkgConfig_FOUND)
    pkg_check_modules(QZXING QZXing)
endif()

if(NOT QZXING_FOUND)
    message(WARNING "No QZXing package config found, will try to detect it.")
    list(APPEND QZXING_LIBRARY_DIRS "")

    # Not used for anything in this codebase?
    list(APPEND QZXING_CFLAGS_OTHER
        -DENABLE_ENCODER_GENERIC
        -DQZXING_QML
        -DQZXING_MULTIMEDIA
        -DQZXING_LIBRARY
        -DZXING_ICONV_CONST
        -DDISABLE_LIBRARY_FEATURES
        -DENABLE_DECODER_QR_CODE
        -DQ_OS_UBUNTUTOUCH
    )

    # Exact name depends on what was used to build it (QMake or CMake)
    find_library(QZXING_LIBRARIES NAMES QZXing qzxing)
    message(STATUS "QZXING_LIBRARIES: ${QZXING_LIBRARIES}")

    # Exact path depends on what was used to build it (QMake or CMake)
    find_path(QZXING_INCLUDE_DIRS QZXing.h PATH_SUFFIXES qzxing)
    message(STATUS "QZXING_INCLUDE_DIRS: ${QZXING_INCLUDE_DIRS}")

    if(NOT QZXING_LIBRARIES)
        message(FATAL_ERROR "Could not find QZXing library")
    endif()
    if(NOT QZXING_INCLUDE_DIRS)
        message(FATAL_ERROR "Could not find QZXing headers")
    endif()
endif()

include(FindGettext)
if(NOT GETTEXT_FOUND)
  message(FATAL_ERROR "Could not find gettext")
endif(NOT GETTEXT_FOUND)

option(CLICK_MODE "Installs to a contained location" on)
set(CLICK_VERSION "${PROJECT_VERSION}"
    CACHE STRING "Version of built Click. Meaningful only in Click mode.")

set(CAMERA_APP lomiri-camera-app)
set(AUTOPILOT_DIR lomiri_camera_app)
set(DESKTOP_FILE lomiri-camera-app.desktop)
set(READER_APP lomiri-barcode-reader-app)
set(DESKTOP_FILE_READER lomiri-barcode-reader-app.desktop)
set(UBUNTU_MANIFEST_PATH "manifest.json.in" CACHE INTERNAL "Tells QtCreator location and name of the manifest file")
set(CMAKE_VERBOSE_MAKEFILE ON)

if(CLICK_MODE)

    set(CMAKE_INSTALL_PREFIX /)
    set(CAMERA_APP_DIR /)
    set(CMAKE_INSTALL_BINDIR /)
    set(PLUGIN_BASE ${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_LIBRARY_ARCHITECTURE})
    set(PLUGIN_DIR ${PLUGIN_BASE}/CameraApp)
    set(DESKTOP_EXEC "lomiri-camera-app-migrate.py ${CAMERA_APP}")
    set(CAMERA_ICON "${CMAKE_INSTALL_DATADIR}/icons/lomiri-camera-app.svg")
    set(CAMERA_SPLASH "${CMAKE_INSTALL_DATADIR}/icons/lomiri-camera-app-splash.svg")
    install(FILES assets/lomiri-camera-app.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
    install(FILES assets/lomiri-camera-app-splash.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)

    set(READER_ICON "${CMAKE_INSTALL_DATADIR}/icons/lomiri-barcode-reader-app.svg")
    set(READER_SPLASH "${CMAKE_INSTALL_DATADIR}/icons/lomiri-barcode-reader-app-splash.svg")
    install(FILES assets/lomiri-camera-app.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
    install(FILES assets/lomiri-camera-app-splash.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
    install(FILES assets/lomiri-barcode-reader-app.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
    install(FILES assets/lomiri-barcode-reader-app-splash.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)

    add_definitions("-DCLICK_MODE=1")

    if(NOT SOURCE)
        execute_process(
            COMMAND git remote -v
            COMMAND grep origin
            COMMAND grep push
            COMMAND awk "{print $2}"
            OUTPUT_VARIABLE SOURCE
            RESULT_VARIABLE res
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
      message("-- Setting SOURCE to ${SOURCE} with result of : ${res}")
    endif(NOT SOURCE)

    if(NOT SOURCE)
        set(SOURCE lp:${APP_HARDCODE})
        message("-- Setting SOURCE to ${SOURCE}")
    endif(NOT SOURCE)

else(CLICK_MODE)
    set(CAMERA_APP_DIR ${CMAKE_INSTALL_FULL_DATADIR}/${CAMERA_APP})
    set(CAMERA_ICON lomiri-camera-app)
    set(CAMERA_SPLASH ${CAMERA_APP_DIR}/assets/lomiri-camera-app-splash.svg)
    install(FILES assets/lomiri-camera-app.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

    set(READER_ICON "lomiri-barcode-reader-app")
    set(READER_SPLASH "${CAMERA_APP_DIR}/assets/lomiri-barcode-reader-app-splash.svg")
    install(FILES assets/lomiri-barcode-reader-app.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

    set(PLUGIN_BASE ${CMAKE_INSTALL_FULL_LIBDIR}/${CAMERA_APP})
    set(PLUGIN_DIR ${PLUGIN_BASE}/CameraApp)
    set(DESKTOP_EXEC ${CAMERA_APP})
endif(CLICK_MODE)

install(PROGRAMS lomiri-camera-app-migrate.py DESTINATION "${CAMERA_APP_DIR}")

add_compile_options(
    ${QZXING_CFLAGS_OTHER}
)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Multimedia_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${QZXING_INCLUDE_DIRS}
    )

set(camera_app_HDRS
    cameraapplication.h)

set(camera_app_SRCS
    cameraapplication.cpp
    main.cpp)

add_executable(${CAMERA_APP}
    ${cameraapplication_MOC_SRCS}
    ${camera_app_SRCS}
    )

target_link_directories(${CAMERA_APP}
    PUBLIC
    ${QZXING_LIBRARY_DIRS}
)

target_link_libraries(${CAMERA_APP}
    ${Qt5Core_LIBRARIES}
    ${Qt5Quick_LIBRARIES}
    ${Qt5Multimedia_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    stdc++
    ${QZXING_LIBRARIES}
    )

install(TARGETS ${CAMERA_APP}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

file(GLOB QML_JS_FILES *.qml *.js *.png *.svg)

install(FILES ${QML_JS_FILES}
    DESTINATION ${CAMERA_APP_DIR}
    )
set(QML_DIR qml)
install(DIRECTORY ${QML_DIR}
    DESTINATION ${CAMERA_APP_DIR}
    )

# For tests
file(COPY ${QML_DIR} DESTINATION ${CMAKE_BINARY_DIR})

set(ASSETS_DIR assets)


install(DIRECTORY ${ASSETS_DIR}
    DESTINATION ${CAMERA_APP_DIR}
    )

install(DIRECTORY ${THEME_DIR}
    DESTINATION ${CAMERA_APP_DIR}
    )

configure_file(${DESKTOP_FILE}.in.in ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}.in)
add_custom_target(${DESKTOP_FILE} ALL
    COMMENT "Merging translations into ${DESKTOP_FILE}"
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
            --desktop --template=${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}.in
            -o ${DESKTOP_FILE}
            -d ${CMAKE_SOURCE_DIR}/po
    )

configure_file(${DESKTOP_FILE_READER}.in.in ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_READER}.in)
add_custom_target(${DESKTOP_FILE_READER} ALL
    COMMENT "Merging translations into ${DESKTOP_FILE_READER}"
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
            --desktop --template=${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_READER}.in
            -o ${DESKTOP_FILE_READER}
            -d ${CMAKE_SOURCE_DIR}/po
        )

# make non compiled files visible in qtcreator
file(GLOB_RECURSE NON_COMPILED_FILES *.qml *.js *.py *.svg *.png *.in *.json *.desktop *.pot *.apparmor qmldir)
add_custom_target(NON_COMPILED_TARGET ALL SOURCES ${NON_COMPILED_FILES})

if(CLICK_MODE)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}
        DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_READER}
        DESTINATION ${CMAKE_INSTALL_PREFIX})

    execute_process(
      COMMAND dpkg-architecture -qDEB_HOST_ARCH
      OUTPUT_VARIABLE CLICK_ARCH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    configure_file(manifest.json.in ${CMAKE_CURRENT_BINARY_DIR}/manifest.json)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifest.json
            DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES camera.apparmor
            DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES camera-contenthub.json
            DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES barcode-reader.apparmor
            DESTINATION ${CMAKE_INSTALL_PREFIX})

else(CLICK_MODE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_READER}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES camera-contenthub.json
            DESTINATION ${CMAKE_INSTALL_DATADIR}/lomiri-content-hub/peers
            RENAME lomiri-camera-app)
endif(CLICK_MODE)

add_subdirectory(CameraApp)
add_subdirectory(desktop)

# for dh_translations to extract the domain
# (regarding syntax consistency, see http://pad.lv/1181187)
set (GETTEXT_PACKAGE "lomiri-camera-app")

add_subdirectory(po)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
