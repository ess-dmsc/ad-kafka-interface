cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project("areaDetectorKafkaInterface")
set(kafkaplugin_VERSION_MAJOR 1)
set(kafkaplugin_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

add_subdirectory(unit_tests)
