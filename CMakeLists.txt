cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

# Must be set before project(): CMake reads it while probing the compiler.
# Left unset, it defaults to whatever the build machine runs, which stamps the binaries
# with that version as their minimum -- a build on macOS 26 produced "minos 26.0" and
# refused to launch on anything older, which is every Mac we ship to. 12.0 is what the
# SDK's own Qt is built against (qt6-static/mkspecs/qconfig.pri:
# QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.0), so it is the floor the toolchain supports.
if(APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")
endif()

project(speaker-layout-converter
  VERSION 1.0.0
  DESCRIPTION "Speaker Layout Converter: convert loudspeaker layouts between spatialization tools (SpatGRIS, IEM, IRCAM Spat, EASE, 4DSOUND, Spat Revolution, CSV)"
  LANGUAGES CXX
  HOMEPAGE_URL "https://github.com/sat-mtl/speaker-setup-format-converter"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BUILD_SHARED_LIBS OFF)

if(EMSCRIPTEN)
  add_link_options("--bind")
endif()

include(FetchContent)

if(BUILD_TESTING)
FetchContent_Declare(
  Catch2
  GIT_SHALLOW    TRUE
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.6.0)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
endif()

# No GIT_SHALLOW here, unlike the entries above: this is the one dependency pinned to a
# raw commit rather than a tag, and a shallow clone is `clone --depth 1 --branch <GIT_TAG>`,
# which only resolves branch and tag names. Configuring from an empty build directory
# therefore failed on every platform with "Remote branch 4f3c41db... not found in upstream
# origin"; it only looked healthy where _deps was already populated.
FetchContent_Declare(
  csv2
  GIT_REPOSITORY https://github.com/p-ranav/csv2.git
  GIT_TAG        4f3c41db6457465e94b92b91fc560b911c16a16a
)
FetchContent_MakeAvailable(csv2)

FetchContent_Declare(
  ctre
  GIT_SHALLOW    TRUE
  GIT_REPOSITORY https://github.com/hanickadot/compile-time-regular-expressions
  GIT_TAG        v3.10.0
)
FetchContent_MakeAvailable(ctre)

# args builds its examples and unit tests by default, with -Werror -pedantic:
# recent compilers reject its bundled Catch header, so keep only the library.
# Cache entries, not plain variables: args declares these with option() under
# cmake_minimum_required(VERSION 3.10), where CMP0077 is OLD.
set(ARGS_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(ARGS_BUILD_UNITTESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  args
  GIT_SHALLOW    TRUE
  GIT_REPOSITORY https://github.com/Taywee/args
  GIT_TAG        6.4.7
)
FetchContent_MakeAvailable(args)

FetchContent_Declare(
  pugixml
  GIT_SHALLOW    TRUE
  GIT_REPOSITORY https://github.com/zeux/pugixml
  GIT_TAG        v1.15
)
FetchContent_MakeAvailable(pugixml)

add_library(speaker-layout-converter-lib
  converter.hpp
  converter.cpp

  ease_parser.hpp
  ease_parser.cpp
  ease_model.hpp

  aiira_parser.hpp
  aiira_parser.cpp
  aiira_model.hpp

  spat_parser.hpp
  spat_parser.cpp
  spat_model.hpp

  csv_parser.hpp
  csv_parser.cpp
  csv_model.hpp

  speakerview_parser.hpp
  speakerview_parser.cpp
  speakerview_model.hpp

  fourdsound_parser.hpp
  fourdsound_parser.cpp
  fourdsound_model.hpp

  spat_revolution_parser.hpp
  spat_revolution_parser.cpp
  spat_revolution_model.hpp
)

target_compile_definitions(speaker-layout-converter-lib
  PUBLIC
    BOOST_ALL_NO_LIB=1
    BOOST_NO_RTTI=1
    BOOST_MATH_DISABLE_FLOAT128=1
)
if(BOOST_ROOT)
  target_include_directories(speaker-layout-converter-lib
    PUBLIC
      "${BOOST_ROOT}"
  )
elseif(Boost_INCLUDE_DIR)
  target_include_directories(speaker-layout-converter-lib
    PUBLIC
      "${Boost_INCLUDE_DIR}"
  )
endif()

target_link_libraries(speaker-layout-converter-lib
  PRIVATE
    ctre::ctre
    csv2::csv2
    pugixml-static
)

if(BUILD_TESTING)
enable_testing()

add_executable(ease-test
  tests/test_ease.cpp
  tests/ease_sample.ease
)

add_executable(aiira-test
  tests/test_aiira.cpp
  tests/aiira_sample.json
)

add_executable(spat-test
  tests/test_spat.cpp
  tests/spat_panoramix_sample.rtf
)

add_executable(csv-test
  tests/test_csv.cpp
  tests/csv_sample.csv
)

add_executable(speakerview-test
  tests/test_speakerview.cpp
  tests/speakerview_sample.xml
)

add_executable(converter-test
  tests/test_converter.cpp
)

add_executable(azimuth-test
  tests/test_azimuth_conversion.cpp
)

add_executable(spat-revolution-test
  tests/test_spat_revolution.cpp
  tests/spat_revolution.ioconfig
)

add_executable(crlf-test
  tests/test_crlf.cpp
)
target_link_libraries(ease-test PRIVATE speaker-layout-converter-lib)
target_link_libraries(aiira-test PRIVATE speaker-layout-converter-lib)
target_link_libraries(spat-test PRIVATE speaker-layout-converter-lib)
target_link_libraries(csv-test PRIVATE speaker-layout-converter-lib)
target_link_libraries(speakerview-test PRIVATE speaker-layout-converter-lib)
target_link_libraries(converter-test PRIVATE speaker-layout-converter-lib)
if(TARGET Catch2::Catch2WithMain)
  target_link_libraries(azimuth-test PRIVATE speaker-layout-converter-lib Catch2::Catch2WithMain)
endif()
target_link_libraries(spat-revolution-test PRIVATE speaker-layout-converter-lib)
target_link_libraries(crlf-test PRIVATE speaker-layout-converter-lib)

# spat-revolution-test opens its sample by relative path, the others #embed theirs:
# run them all from tests/ so the working directory is never a factor.
# Exercises the CLI executable's own file reading rather than the library's parsers: on
# Windows a CRLF file opened in text mode yields fewer bytes than file_size() reported and
# the tail of the buffer stays NUL, which every parser rejects as trailing garbage. The
# library never sees that, so only a test that goes through the binary can catch it. On
# Linux and macOS text and binary mode are the same thing and this simply passes.
add_test(NAME cli-crlf COMMAND spatparsecli
  --in-format=iem --out-format=csv
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/aiira_sample_crlf.json")

foreach(test IN ITEMS
    ease-test
    aiira-test
    spat-test
    csv-test
    speakerview-test
    converter-test
    azimuth-test
    spat-revolution-test
    crlf-test)
  add_test(NAME ${test} COMMAND ${test} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests")
endforeach()
endif()

add_executable(spatparsecli
  cli.cpp
)

target_link_libraries(spatparsecli PRIVATE speaker-layout-converter-lib args)

# Qt GUI Application
# Not QUIET: when a Qt component fails to resolve, the reason is what we need to read.
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
set(CMAKE_AUTOMOC ON)

qt_add_executable(speaker-layout-converter
  gui_main.cpp
  mainwindow.cpp
  mainwindow.hpp
)

# brotli comes in only as a transitive dependency of the SDK's freetype (WOFF2 support),
# so it must be taken from the same prefix as the rest of the toolchain. Searching the
# system paths as well is how the macOS bundle ended up with ten absolute
# /opt/homebrew/opt/... load commands: Homebrew brotli is a dylib, and a Mac without
# Homebrew then cannot launch the app. NO_CMAKE_SYSTEM_PATH/NO_SYSTEM_ENVIRONMENT_PATH
# keep the search inside CMAKE_PREFIX_PATH, which is where the SDK lives on all three
# platforms; when the SDK's freetype needs no brotli the variables stay unset and nothing
# is linked.
set(_brotli_scope NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
find_library(BROTLICOMMON_LIBRARY NAMES brotlicommon brotlicommon-static ${_brotli_scope})
find_library(BROTLIDEC_LIBRARY NAMES brotlidec brotlidec-static ${_brotli_scope})
find_library(BROTLIENC_LIBRARY NAMES brotlienc brotlienc-static ${_brotli_scope})

target_link_libraries(speaker-layout-converter PRIVATE
  speaker-layout-converter-lib
  Qt6::Core
  Qt6::Widgets
)

if(BROTLICOMMON_LIBRARY)
  target_link_libraries(speaker-layout-converter PRIVATE
    ${BROTLICOMMON_LIBRARY}
    ${BROTLIDEC_LIBRARY}
    ${BROTLIENC_LIBRARY}
  )
endif()

set_target_properties(speaker-layout-converter PROPERTIES
  WIN32_EXECUTABLE TRUE
  MACOSX_BUNDLE TRUE
  MACOSX_BUNDLE_BUNDLE_NAME "Speaker Layout Converter"
  MACOSX_BUNDLE_GUI_IDENTIFIER "ca.qc.sat.speaker-layout-converter"
  MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
  MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
  MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
  MACOSX_BUNDLE_COPYRIGHT "Societe des Arts Technologiques"
  MACOSX_BUNDLE_INFO_STRING "${PROJECT_DESCRIPTION}"
  MACOSX_BUNDLE_ICON_FILE "speaker-layout-converter.icns"
)

if(APPLE)
  # Finder labels a bundle by its directory name, so the display name has to be the
  # target's output name -- same as "ossia score.app" and "Domeport Pro.app".
  set_target_properties(speaker-layout-converter PROPERTIES
    OUTPUT_NAME "Speaker Layout Converter")

  # The .icns has to be inside the bundle for CFBundleIconFile to resolve.
  target_sources(speaker-layout-converter PRIVATE packaging/speaker-layout-converter.icns)
  set_source_files_properties(packaging/speaker-layout-converter.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
endif()

if(WIN32)
  # llvm-rc / windres is what the ossia SDK toolchain ships; without a working RC
  # compiler the icon and the version block are simply left out rather than failing
  # the build.
  include(CheckLanguage)
  check_language(RC)
  if(CMAKE_RC_COMPILER)
    enable_language(RC)
    configure_file(packaging/speaker-layout-converter.rc.in
      "${CMAKE_CURRENT_BINARY_DIR}/speaker-layout-converter.rc" @ONLY)
    target_sources(speaker-layout-converter PRIVATE
      "${CMAKE_CURRENT_BINARY_DIR}/speaker-layout-converter.rc")
  endif()
endif()

qt_add_resources(speaker-layout-converter "fonts"
  PREFIX "/fonts"
  BASE resources/
  FILES
    resources/DMSans-VariableFont_opsz,wght.ttf
    resources/DMSans-Italic-VariableFont_opsz,wght.ttf
)
# Install tree, shared by the three packaging scripts in packaging/. On Linux it is also
# the AppDir layout appimagetool expects, which is why the icon and the .desktop entry are
# installed rather than copied by the script.
include(GNUInstallDirs)

# Everything here goes in one component, and the packaging scripts install that component
# rather than the whole project: ctre and pugixml come in through FetchContent with their
# own install rules, so a plain `cmake --install` also drops their headers, libpugixml.a
# and the pkgconfig/cmake package files into the prefix. Those are SDK artifacts for
# someone building against the libraries, and they were ending up in the .dmg next to
# the .app.
set(SLC_COMPONENT speaker-layout-converter)

install(TARGETS spatparsecli
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT "${SLC_COMPONENT}")

if(APPLE)
  install(TARGETS speaker-layout-converter
    BUNDLE DESTINATION . COMPONENT "${SLC_COMPONENT}")
else()
  install(TARGETS speaker-layout-converter
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT "${SLC_COMPONENT}")
endif()

install(FILES README
  DESTINATION "${CMAKE_INSTALL_DOCDIR}" RENAME README.md
  COMPONENT "${SLC_COMPONENT}")

if(NOT APPLE AND NOT WIN32)
  install(FILES packaging/speaker-layout-converter.desktop
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications" COMPONENT "${SLC_COMPONENT}")
  install(FILES packaging/speaker-layout-converter.png
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps"
    COMPONENT "${SLC_COMPONENT}")
  install(FILES packaging/speaker-layout-converter.svg
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
    COMPONENT "${SLC_COMPONENT}")
endif()

qt_finalize_project()
