cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

add_definitions(-std=c++11 -pthread) #-v = verbose

enable_testing()

if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif()

include(${CMAKE_MODULE_PATH}/DownloadProject.cmake)
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

find_path(rdkafka_INC NAMES librdkafka/rdkafka.h librdkafka/rdkafkacpp.h PATHS /usr/local/include)

find_path(flatbuffers_INC NAMES flatbuffers/flatbuffers.h PATHS /usr/local/include)

include_directories(${plugin_SRC} ${driver_SRC} ${rdkafka_INC} ${flatbuffers_INC} ".")

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
    set(E_VER "3.14.12.5")
    set(E_OS "centos7-x86_64")
    message(STATUS "Assuming Linux machine running the ESS EPICS environment.")
    include_directories("$ENV{EPICS_MODULES_PATH}/adcore/2.5.1/${E_VER}/include/" "$ENV{EPICS_BASE}/include/compiler/gcc/" "$ENV{EPICS_BASE}/include/os/Linux/" "$ENV{EPICS_MODULES_PATH}/asyn/4.27.0/${E_VER}/include/")
    
    link_directories("$ENV{EPICS_MODULES_PATH}/adcore/2.5.1/${E_VER}/lib/${E_OS}" "$ENV{EPICS_MODULES_PATH}/asyn/4.27.0/${E_VER}/lib/${E_OS}" "$ENV{EPICS_BASE}/lib/${E_OS}")
endif()

include_directories("$ENV{EPICS_BASE}/include")

find_library(rdkafka_LIB NAMES rdkafka PATHS /usr/local/lib)
find_library(rdkafkacpp_LIB NAMES rdkafka++ PATHS /usr/local/lib)

add_executable(unit_tests ${TEST_FILES} "${plugin_SRC}jsoncpp.cpp" "${plugin_SRC}KafkaProducer.cpp" "${plugin_SRC}ParamUtility.cpp" "${plugin_SRC}NDArraySerializer.cpp" "${driver_SRC}/KafkaConsumer.cpp")

target_link_libraries(unit_tests gtest gmock_main NDPlugin ADBase asyn Com ${rdkafka_LIB} ${rdkafkacpp_LIB})

add_test(TestAll unit_tests)