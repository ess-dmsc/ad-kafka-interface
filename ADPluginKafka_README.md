# ADPluginKafka
An EPICS areaDetector plugin which sends areaDetector data serialised using flatbuffers to a Kafka broker. The plugin is in a state which should make it useful (ignoring unknown bugs). Several suggestions on improvements are listed last in this document however.

## Requirements
This plugin has been developed primarily for use with the **ESS EPICS Environment** (EEE) used at ESS in Lund. Make files for building the module when using a regular EPICS installation are included but have received very little testing.

For communicating with the Kafka broker, the C++ version of `librdkafka` is used. The source code for this library can be downloaded from [https://github.com/edenhill/librdkafka](https://github.com/edenhill/librdkafka). At least version 0.9.4 of `librdkafka` is required for all the features to work though earlier versions will also compile.

To simplify data handling, the plugin uses flatbuffers ([https://github.com/google/flatbuffers](https://github.com/google/flatbuffers)) for data serialisation. `librdkafka` produces statistics messages in JSON and these are parsed using `jsoncpp` ([https://github.com/open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)).
In order to run the demo of the plugin in the `startup` (or `iocs`) directory, the module `ADSimDetector` is required as it contains features for running a simulated areaDetector.

## Compiling and running the example
There are currently two sets of make files provided with the repository. One for use with the ESS EPICS Environment and one for use with a standard EPICS installation.

### ESS EPICS Environment
If the ESS EPICS Environment is available, the *EEEmakefile* should be used as an input to gnumake:

* `cd` to the `m-epics-ADPluginKafka` directory.
* Run gnumake (`make -f EEEmakefile`).
* Install using gnumake (`sudo -E make -f EEEmakefile install`).

Note that the EEE make file assumes that librdkafka is installed in the standard location (i.e. `/usr/local/`). If this is not the case, the file `EEEmakefile` will have to be modified so that the variables `LIBRDKAFKA_LIB_PATH` and `LIBRDKAFKA_INC_PATH`  point to the locations of the library and header files.

#### Running an example IOC using the ESS EPICS Environment
A simple example illustrating how the plugin works is provided in the `m-epics-ADPluginKafka/startup` directory. When the plugin is installed:

* `cd` to `m-epics-ADPluginKafka/startup`.
* Run the startup script: `iocsh -r ADPluginKafka,1.0.0-BETA -c "requireSnippet(ADPluginKafka_demo.cmd)"`.

### Compiling and running the driver in a standard EPICS installation
The steps shown here worked on the development machine but has been tested nowhere else.

1. Copy the `m-epics-ADPluginKafka` directory to your `$(EPICS_MODULES_PATH)/areaDetector` directory.
2. Set the variable `SIMDET` in the file `m-epics-ADPluginKafka/iocs/ADPluginKafkaIOC/configure/RELEASE` to point to the location of `ADSimDetector` in your EPICS installation.
3. Modify the files `Makefile` and `start_epics` in the `m-epics-ADPluginKafka/iocs/ADPluginKafkaIOC/iocBoot/iocADPluginKafka` directory to reflect the current platform.
4. Change (`cd`) to the `m-epics-ADPluginKafka` directory and compile by running `make`.
5. Modify the `m-epics-ADPluginKafka/iocs/ADPluginKafkaIOC/iocBoot/iocADPluginKafka/st.cmd` file to use the address of your Kafka broker.
6. Run `sh start_epics` from that directory.


## Process variables (PV:s)
This plugin provides a few extra process variables (PV) besides the ones provided through inheritance from `NDPluginDriver`. The plugin also modifies one process variable inherited from `NDPluginDriver` directly. All the relevant PVs are listed below.

* `$(P)$(R)KafkaBrokerAddress` and `$(P)$(R)KafkaBrokerAddress_RBV` are used to set the address of one or more Kafka broker.The address should include the port and have the following form:`address:port`. When using several addresses they should be separated by a comma. Note that the text string is limited to 40 characters.
* `$(P)$(R)KafkaTopic` and `$(P)$(R)KafkaTopic_RBV` are used to set and retrieve the current topic. Limited to 40 characters.
* `$(P)$(R)ConnectionStatus_RBV` holds an integer corresponding to the current connection status. Se `ADPluginKafka.template` for possible values.
* `$(P)$(R)ConnectionMessage_RBV` is a PV that has a text message of at most 40 characters that gives information about the current connection status.
* `$(P)$(R)KafkaMaxQueueSize` and `$(P)$(R)KafkaMaxQueueSize_RBV` modifies and reads the number of messages allowed in the Kafka output buffer. Never set to a value < 1.
* `$(P)$(R)UnsentPackets_RBV` keeps track of the number of messages not yet transmitted to the Kafka broker. The minimum time between updates of this value is set by the next PV.
* `$(P)$(R)KafkaMaxMessageSize_RBV` is used to read the maximum message size allowed by librdkafka. This value should be updated automatically as message sizes exceeds their old values. The absolute maximum size is approx. 953 MB.
* `$(P)$(R)KafkaStatsIntervalTime` and `$(P)$(R)KafkaStatsIntervalTime_RBV` are used to set and read the time between Kafka broker connection stats. This value is given in milliseconds (ms). Setting a very short update time is not advised.
* `$(P)$(R)DroppedArrays_RBV` is increased if the Kafka producer messages queue is full (i.e `$(P)$(R)UnsentPackets_RBV` is equal to `$(P)$(R)KafkaMaxQueueSize_RBV`.

## To-do
The plugin is somewhat production ready but improvements would be useful. Some of these (in no particular order) are:

* **Improvements to error handling** There are several types of connection, buffer full and data transmission error that this plugin will not handle gracefully.
* **More PV:s** These are required for more fine grained control of the Kafka producer as well as for improvement in error handling.
* **Performance tests** It is likely that performance of the plugin could be improved. To determine if this is the case, performance tests and profiling of the code is required.
* **Modify db-template** The existing PV:s could be modified in order to be slightly more useful.
* **Kafka producer parameters** Kafka producer settings such as memory buffers, timeouts and so on needs to be adjusted for optimal performance.
* **More extensive unit tests** It is possible to do more extensive unit testing.

