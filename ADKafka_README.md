# ADKafka
An EPICS areaDetector driver which consumes NDArray data serialised using flatbuffers from a Kafka broker. Basic functionality of the driver works but some bugs related to the setting of PVs have been encountered in testing.

## Requirements
The `GNUmakefile` used to build this plugin is only compatible with the **ESS EPICS Environment** (EEE) used at ESS in Lund. Make files for building the module when using a regular EPICS installation exist but have received limited testing.

For communicating with the Kafka broker, the C++ version of `librdkafka` is used. The source code for this library can be downloaded from [https://github.com/edenhill/librdkafka](https://github.com/edenhill/librdkafka). At least version 0.9.4 of `librdkafka` is required for all the features to work though earlier versions will also compile.

To simplify data handling, the plugin uses flatbuffers ([https://github.com/google/flatbuffers](https://github.com/google/flatbuffers)) for data serialisation. `librdkafka` produces statistics messages in JSON and these are parsed using `jsoncpp` ([https://github.com/open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)).

## Compiling and running the example
There are currently two sets of make files provided with the repository. One for use with the ESS EPICS Environment and one for use with a standard EPICS installation.

### ESS EPICS Environment
If the ESS EPICS Environment is available, the *EEEmakefile* should be used as an input to gnumake:

* `cd` to the `m-epics-ADKafka` directory.
* Run gnumake (`make -f EEEmakefile`).
* Install using gnumake (`sudo -E make -f EEEmakefile install`).

Note that the EEE make file assumes that librdkafka is installed in the standard location (i.e. `/usr/local/`). If this is not the case, the file `EEEmakefile` will have to be modified so that the variables `LIBRDKAFKA_LIB_PATH` and `LIBRDKAFKA_INC_PATH`  point to the locations of the library and header files.

#### Running an example IOC using the ESS EPICS Environment
A simple example illustrating how the plugin works is provided in the `m-epics-ADKafka/startup` directory. When the plugin is installed:

* `cd` to `m-epics-ADKafka/startup`.
* Run the startup script: `iocsh -r ADKafka,1.0.0-BETA -c "requireSnippet(ADKafka_demo.cmd)"`.

### Compiling and running the driver in a standard EPICS installation
The steps shown here worked on the development machine but has been tested nowhere else.

1. Copy the `m-epics-ADKafka` directory to your `$(EPICS_MODULES_PATH)/areaDetector` directory.
2. Modify the files `Makefile` and `start_epics` in the `m-epics-ADKafka/iocs/ADKafkaIOC/iocBoot/iocADKafka` directory to reflect the current platform.
3. Change (`cd`) to the `m-epics-ADKafka` directory and compile by running `make`.
4. Modify the `m-epics-ADKafka/iocs/ADKafkaIOC/iocBoot/iocADKafka/st.cmd` file to use the address of your Kafka broker.
5. Run `sh start_epics` from that directory.

## Process variables (PVs)
This plugin provides a few extra process variables besides the ones provided through inheritance from `NDPluginDriver`. These are:

* `$(P)$(R)KafkaBrokerAddress` and `$(P)$(R)KafkaBrokerAddress_RBV` are used to set the address of one or more Kafka broker.The address should include the port and have the following form:`address:port`. When using several addresses they should be seperated by a comma. Note that the text string is limited to 40 characters.
* `$(P)$(R)KafkaTopic` and `$(P)$(R)KafkaTopic_RBV` are used to set and retrieve the current topic. Limited to 40 characters.
* `$(P)$(R)ConnectionStatus_RBV` holds an integer corresponding to the current connection status. Se `ADPluginKafka.template` for possible values.
* `$(P)$(R)ConnectionMessage_RBV` is a PV that has a text message of at most 40 characters that gives information about the current connection status.
* `$(P)$(R)KafkaMaxMessageSize_RBV` is used to read the maximum message size allowed by librdkafka. This value should be updated automatically as message sizes exceeds their old values. The absolute maximum size is approx. 953 MB.
* `$(P)$(R)KafkaStatsIntervalTime` and `$(P)$(R)KafkaStatsIntervalTime_RBV` are used to set and read the time between Kafka broker connection stats. This value is given in milliseconds (ms). Setting a very short update time is not advised.
* `$(P)$(R)StartMessageOffset` and `$(P)$(R)StartMessageOffset_RBV` are used to set and read the starting offset used when first connecting to a topic. The options are **Beginning**, **Stored**, **Manual** and **End**. A more complete explanation is given in the source code documentation.
* `$(P)$(R)CurrentMessageOffset` and `$(P)$(R)CurrentMessageOffset_RBV` sets and reads the current message offset. Note that it is only possible to set the offset if `$(P)$(R)StartMessageOffset` is set to **Manual**.
* `$(P)$(R)KafkaGroup` and `$(P)$(R)KafkaGroup_RBV` are used to set the Kafka consumer group name/id. The group name is used if several consumers should share consumption from one topic and to store the current message offset on the Kafka broker.

## To-do
This driver is somewhat production ready. However, there are some improvements that could increase its usefulness:

* **Improvements to error handling** The plugin has some error handling code. It could be significantly expanded however. This includes the error reporting capability of the plugin.
* **More PVs** These are required for more fine grained control of the Kafka producer as well as for improvement in error handling.
* **Performance tests** It is likely that performance of the plugin could be improved. To determine if this is the case, performance tests and profiling of the code is required.
* **Modify db-template** The existing PVs could potentially be modified in order to improve its usefulness.
* **Kafka consumer parameters** Some Kafka parameters can be set but being able to set more of them is probably useful. Kafka consumer lag is probably the most useful of these statistics to make available.
* **More extensive unit tests** It is possible to do more extensive unit testing.
* **Bug related to setting PVs** When testing the driver some bug related to the setting of PVs was encountered. A problem probably related to this one was that the CPU usage was excessive. This should be fixed.
* **Problems related to changing offset** Changing the used offset is currently problematic. This should be fixed.
