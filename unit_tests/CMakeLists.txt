cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

add_definitions(-std=c++11 -pthread) #-v = verbose

enable_testing()

set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

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

find_package(LibRDKafka)

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

set(Common_SRC
  jsoncpp.cpp
)

set(Common_INC
  base.h
  flatbuffers.h
  json.h
  stl_emulation.h
  NDArray_schema_generated.h
  ParamUtility.h
)

list(TRANSFORM Common_SRC PREPEND "../ADKafka/ADKafkaApp/src/")
list(TRANSFORM Common_INC PREPEND "../ADKafka/ADKafkaApp/src/")

add_library(Common OBJECT ${Common_SRC} ${Common_INC})

set(Driver_SRC
  KafkaConsumer.cpp
  KafkaDriver.cpp
  NDArrayDeSerializer.cpp
)

set(Driver_INC
  KafkaConsumer.h
  KafkaDriver.h
  NDArrayDeSerializer.h
)

list(TRANSFORM Driver_SRC PREPEND "../ADKafka/ADKafkaApp/src/")
list(TRANSFORM Driver_INC PREPEND "../ADKafka/ADKafkaApp/src/")

add_library(Driver OBJECT ${Driver_SRC} ${Driver_INC})
target_include_directories(Driver PRIVATE ${LibRDKafka_INCLUDE_DIR})

set(Plugin_SRC
  KafkaProducer.cpp
  KafkaPlugin.cpp
  NDArraySerializer.cpp
)

set(Plugin_INC
  KafkaProducer.h
  KafkaPlugin.h
  NDArraySerializer.h
)

list(TRANSFORM Plugin_SRC PREPEND "../ADPluginKafka/ADPluginKafkaApp/src/")
list(TRANSFORM Plugin_INC PREPEND "../ADPluginKafka/ADPluginKafkaApp/src/")

add_library(Plugin OBJECT ${Plugin_SRC} ${Plugin_INC})
target_include_directories(Plugin PRIVATE ${LibRDKafka_INCLUDE_DIR})

set(Test_SRC
  RunTests.cpp
  GenerateNDArray.cpp
  KafkaConsumerTest.cpp
  KafkaDriverTest.cpp
  KafkaPluginTest.cpp
  KafkaProducerTest.cpp
  NDArraySerializerTest.cpp
  ParamUtilityTest.cpp
  PortName.cpp
  $<TARGET_OBJECTS:Driver>
  $<TARGET_OBJECTS:Plugin>
  $<TARGET_OBJECTS:Common>
)

set(Test_INC
  GenerateNDArray.h
  PortName.h
)

add_executable(unit_tests ${Test_SRC} ${Test_INC})
target_include_directories(unit_tests PRIVATE "../ADPluginKafka/ADPluginKafkaApp/src/" "../ADKafka/ADKafkaApp/src/" ${LibRDKafka_INCLUDE_DIR})

if (${APPLE})
    target_link_libraries(unit_tests gtest gmock_main NDPlugin ADBase asyn Com ${LibRDKafka_LIBRARIES})
else()
    target_link_libraries(unit_tests gtest gmock_main xml sz busy calc seq ca dbCore ${ZLIB_LIBRARY} ${TIFF_LIBRARY} ${JPEG_LIBRARY} ${HDF5_LIBRARIES} normativeTypesCPP pvAccessCPP pvDataCPP pvDatabaseCPP adcore asyn Com ${LibRDKafka_LIBRARIES})
endif()

get_filename_component(TEST_DATA_PATH "someNDArray.data" DIRECTORY)
target_compile_definitions(unit_tests
    PRIVATE TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DATA_PATH}/")


add_test(TestAll unit_tests)
