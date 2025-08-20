cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(spat-format-converter
  VERSION 1.0.0
  DESCRIPTION "A cross-platform spatialization format converter"
  LANGUAGES CXX
  HOMEPAGE_URL "https://gitlab.com/sat-mtl/audio"
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

add_library(spat-format-converter
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

target_link_libraries(spat-format-converter
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
target_link_libraries(ease-test PRIVATE spat-format-converter)
target_link_libraries(aiira-test PRIVATE spat-format-converter)
target_link_libraries(spat-test PRIVATE spat-format-converter)
target_link_libraries(csv-test PRIVATE spat-format-converter)
target_link_libraries(speakerview-test PRIVATE spat-format-converter)
target_link_libraries(converter-test PRIVATE spat-format-converter)
if(TARGET Catch2::Catch2WithMain)
  target_link_libraries(azimuth-test PRIVATE spat-format-converter Catch2::Catch2WithMain)
endif()
target_link_libraries(spat-revolution-test PRIVATE spat-format-converter)
endif()

add_executable(spatparsecli
  cli.cpp
)

target_link_libraries(spatparsecli PRIVATE spat-format-converter args)

# Qt GUI Application
find_package(Qt6 COMPONENTS Core Widgets QUIET)
if(Qt6_FOUND)
  set(CMAKE_AUTOMOC ON)

  qt_add_executable(spatconverter-gui
    gui_main.cpp
    mainwindow.cpp
    mainwindow.hpp
  )
  
  target_link_libraries(spatconverter-gui PRIVATE
    spat-format-converter
    Qt6::Core
    Qt6::Widgets
  )
  
  set_target_properties(spatconverter-gui PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
  )
  qt_finalize_project()
else()
  message(STATUS "Qt6 not found, GUI will not be built")
endif()
