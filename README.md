# Area detector Kafka interface

This repository contains two separate projects which facilitates the transmission of data between an EPICS IOC and an Apache Kafka broker. The two projects are:

* [An EPICS areaDetector driver](ADKafka_README.md) which acts as a Kafka consumer and makes NDArray data received from the broker available to the IOC.
* [An EPICS areaDetector plugin](ADPluginKafka_README.md) which connects to an areaDetector and serializes NDArray data it receives and sends it to a Kafka broker.

Apache Kafka is an open-source platform for handling streaming data using or more data brokers in order to maximize throughput and reliability. More information on Apache Kafka can be found at [the website of that project.](https://kafka.apache.org/intro)

For serializing and de-serializing the areaDetector (NDArray) data, [Google FlatBuffers](https://github.com/google/flatbuffers) is used. Serializing data using FlatBuffers is fast with a relatively small memory overhead while being easier to use than C-structs.

### Documentation
Instructions on how to compile and use the driver and the plugin can be found in the README-files in the corresponding directories ([ADKafka_README.md](ADKafka/ADKafka_README.md) and [ADPluginKafka_README.md](ADPluginKafka/ADPluginKafka_README.md)). The code is documented using the doxygen syntax. Simply run `doxygen` in the root directory of the repository to generate the HTML version of the documentation in `documentation/html`. The text in the README-files will also be included in the generated HTMl documentation.

### Unit tests
The repository contains a directory with code for unit tests of the two projects. Do note that the build system of the unit tests (specifically the CMake file) will most likely require some modification to work on your system. Due to differences in EPICS installations, the CMake file has only been tested on the development machine (running MacOSX).

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

