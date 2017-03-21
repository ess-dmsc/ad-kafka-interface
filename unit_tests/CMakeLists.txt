cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

add_definitions(-std=c++11 -pthread) #-v = verbose

set(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")

enable_testing()

if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif()

include(DownloadProject)
download_project(PROJ                googletest
                 GIT_REPOSITORY      https://github.com/google/googletest.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

# When using CMake 2.8.11 or later, header path dependencies
# are automatically added to the gtest and gmock targets.
# For earlier CMake versions, we have to explicitly add the
# required directories to the header search path ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")
endif()


file(GLOB TEST_FILES "./*.cpp")

set(plugin_SRC "../m-epics-ADPluginKafka/ADPluginKafkaApp/src/")
set(driver_SRC "../m-epics-ADKafka/ADKafkaApp/src/")

file(GLOB PLUGIN_SRC_FILES "${plugin_SRC}*.cpp")

file(GLOB DRIVER_SRC_FILES "${driver_SRC}*.cpp")

# The file jsoncpp.cpp is present in both EPICS modules. Remove it from one of them to build the unit tests.
FOREACH(item ${DRIVER_SRC_FILES})
  IF(${item} MATCHES "jsoncpp.cpp")
    LIST(REMOVE_ITEM DRIVER_SRC_FILES ${item})
  ENDIF(${item} MATCHES "jsoncpp.cpp")
ENDFOREACH(item)

find_package(LibRDKafka)

find_path(flatbuffers_INC NAMES flatbuffers/flatbuffers.h PATHS /usr/local/include /opt/dm_group/usr/include)

include_directories(${plugin_SRC} ${driver_SRC} ${LibRDKafka_INCLUDE_DIR} ${flatbuffers_INC} ".")

if (NOT DEFINED ENV{EPICS_BASE})
    message(FATAL_ERROR "Missing environment variable \"EPICS_BASE\".")
endif()

if (NOT DEFINED ENV{EPICS_MODULES_PATH})
    message(FATAL_ERROR "Missing environment variable \"EPICS_MODULES_PATH\".")
endif()

if (${APPLE})
    message(STATUS "MacOS X detected, making relevant assumptions about file locations.")
    include_directories("$ENV{EPICS_MODULES_PATH}/areaDetector/ADCore/include/" "$ENV{EPICS_BASE}/include/compiler/clang/" "$ENV{EPICS_BASE}/include/os/Darwin/" "$ENV{EPICS_MODULES_PATH}/asyn/include/")
    
    link_directories("$ENV{EPICS_MODULES_PATH}/areaDetector/ADCore/lib/darwin-x86" "$ENV{EPICS_MODULES_PATH}/asyn/lib/darwin-x86" "$ENV{EPICS_BASE}/lib/darwin-x86")
else()
    set(E_VER "3.15.4")
    set(E_OS "centos7-x86_64")
    message(STATUS "Assuming Linux machine running the ESS EPICS environment.")
    include_directories("$ENV{EPICS_MODULES_PATH}/adcore/2.5.1/${E_VER}/include/" "$ENV{EPICS_BASE}/include/compiler/gcc/" "$ENV{EPICS_BASE}/include/os/Linux/" "$ENV{EPICS_MODULES_PATH}/asyn/4.27.0/${E_VER}/include/")
    
    link_directories("$ENV{EPICS_MODULES_PATH}/adcore/2.5.1/${E_VER}/lib/${E_OS}" "$ENV{EPICS_MODULES_PATH}/asyn/4.27.0/${E_VER}/lib/${E_OS}" "$ENV{EPICS_BASE}/lib/${E_OS}")
    link_directories("$ENV{EPICS_MODULES_PATH}/pvDataCPP/6.0.0/${E_VER}/lib/${E_OS}")
    link_directories("$ENV{EPICS_MODULES_PATH}/pvDatabaseCPP/4.2.0/${E_VER}/lib/${E_OS}")
    link_directories("$ENV{EPICS_MODULES_PATH}/pvAccessCPP/5.0.0/${E_VER}/lib/${E_OS}")
    link_directories("$ENV{EPICS_MODULES_PATH}/normativeTypesCPP/5.1.0/${E_VER}/lib/${E_OS}")
    link_directories("$ENV{EPICS_MODULES_PATH}/xml/2.9.2/${E_VER}/lib/${E_OS}")
    link_directories("$ENV{EPICS_MODULES_PATH}/sz/2.1.0/${E_VER}/lib/${E_OS}")
    link_directories("$ENV{EPICS_MODULES_PATH}/busy/1.6.0/${E_VER}/lib/${E_OS}")
    link_directories("$ENV{EPICS_MODULES_PATH}/calc/3.6.1/${E_VER}/lib/${E_OS}")
    link_directories("$ENV{EPICS_MODULES_PATH}/seq/2.1.10/${E_VER}/lib/${E_OS}")
    
    find_package(HDF5 REQUIRED)
    find_package(JPEG REQUIRED)
    find_package(TIFF REQUIRED)
    find_package(ZLIB REQUIRED)
endif()

include_directories("$ENV{EPICS_BASE}/include")

add_executable(unit_tests ${TEST_FILES} ${PLUGIN_SRC_FILES} ${DRIVER_SRC_FILES})

if (${APPLE})
    target_link_libraries(unit_tests gtest gmock_main NDPlugin ADBase asyn Com ${LibRDKafka_LIBRARIES})
else()
    target_link_libraries(unit_tests gtest gmock_main xml sz busy calc seq ca dbCore ${ZLIB_LIBRARY} ${TIFF_LIBRARY} ${JPEG_LIBRARY} ${HDF5_LIBRARIES} normativeTypesCPP pvAccessCPP pvDataCPP pvDatabaseCPP adcore asyn Com ${LibRDKafka_LIBRARIES})
endif()

configure_file("someNDArray.data" "." COPYONLY)

add_test(TestAll unit_tests)

include(CodeCoverage)

SETUP_TARGET_FOR_COVERAGE_COBERTURA(xml_coverage unit_tests coverage)
SETUP_TARGET_FOR_COVERAGE(html_coverage unit_tests coverage)
