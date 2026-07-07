cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
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

FetchContent_Declare(
  csv2
  GIT_SHALLOW    TRUE
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
endif()

add_executable(spatparsecli
  cli.cpp
)

target_link_libraries(spatparsecli PRIVATE speaker-layout-converter-lib args)

# Qt GUI Application
find_package(Qt6 REQUIRED COMPONENTS Core Widgets QUIET)
if(Qt6_FOUND)
  set(CMAKE_AUTOMOC ON)

  qt_add_executable(speaker-layout-converter
    gui_main.cpp
    mainwindow.cpp
    mainwindow.hpp
  )
  
  find_library(BROTLICOMMON_LIBRARY NAMES brotlicommon brotlicommon-static)
  find_library(BROTLIDEC_LIBRARY NAMES brotlidec brotlidec-static)
  find_library(BROTLIENC_LIBRARY NAMES brotlienc brotlienc-static)

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
  )

  qt_add_resources(speaker-layout-converter "fonts"
    PREFIX "/fonts"
    BASE resources/
    FILES 
      resources/DMSans-VariableFont_opsz,wght.ttf
      resources/DMSans-Italic-VariableFont_opsz,wght.ttf 
  )
  qt_finalize_project()
else()
  message(STATUS "Qt6 not found, GUI will not be built")
endif()
