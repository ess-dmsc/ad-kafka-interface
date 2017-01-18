cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("areaDetectorKafkaInterface")
set(m-epics-kafkaplugin_VERSION_MAJOR 1)
set(m-epics-kafkaplugin_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

add_subdirectory(unit_tests)
