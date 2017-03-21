# Area detector Kafka interface

This repository contains two separate projects which facilitates the transmission of data between an EPICS IOC and a Kafka broker. The two projects are:

* [An EPICS areaDetector driver](m-epics-ADKafka/ADKafka_README.md) which acts as a Kafka consumer and makes NDArray data received from the broker available to the IOC.
* [An EPICS areaDetector plugin](m-epics-ADPluginKafka/ADPluginKafka_README.md) which connects to an areaDetector and serialises NDArray data it receives and sends it to a Kafka broker.

### Documentation
The two projects each have a README-file in their directories describing how the EPICS modules are compiled and used. The code is documented using the doxygen syntax. Simply run `doxygen` in the root directory of the repository. This will generate the HTML version of the documentation in `documentation/html`.

### Unit tests
The repository contains a directory with code for unit tests of the two projects. Do note that the build system of the unit tests (specifically the CMake file) will most likely require some modification to work on your system. Due to differences in EPICS installations, the CMake file has only been tested on the development machine (a MacOSX computer) and virtual machine using the ESS EPICS Environment (EEE).

The unit tests use GTest/GMock for running the test and this library will be downloaded and compiled by CMake when running it. The process for compiling and running the unit tests is as follows:

```
cd ad-kafka-interface
mkdir build
cd build
cmake ..
make
cd unit_tests
./unit_tests
```

