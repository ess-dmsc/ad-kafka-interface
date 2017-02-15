# ADPluginKafka
An EPICS areaDetector plugin which sends areaDetector data serialized using flatbuffers to a Kafka broker. The plugin is in a state which should make it usefull (ignoring unknown bugs). Several suggestions on improvements are listed last in this document however.

## Requirements
The `GNUmakefile` used to build this plugin is only compatible with the **ESS EPICS Environment** (EEE) used at ESS in Lund. Make files for building the module when using a regular EPICS installtion exist but have not been tested.

For communicating with the Kafka broker, the C++ version of `librdkafka` is used. The source code for this library can be downloaded from [https://github.com/edenhill/librdkafka](https://github.com/edenhill/librdkafka). Note that as of the 2017-02-17 the version of librdkafka cloned from the master branch must be used. To simplify data handling, the plugin uses flatbuffers ([https://github.com/google/flatbuffers](https://github.com/google/flatbuffers)) for data serialization. `librdkafka` produces statistics messages in JSON and these are parsed using `jsoncpp` ([https://github.com/open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)).
In order to run the simple demo of the plugin in the `startup` folder, the module `adexample` is required as it contains features for running a simulated areaDetector.

## Compiling and running the example
There are currently two sets of make files provided with the repository. If the ESS EPICS Environment is available, the *EEEmakefile* should be used as an input to gnumake:

* `cd` to the `m-epics-ADPluginKafka` directory.
* Run gnumake (`make -f EEEmakefile`).
* Install using gnumake (`sudo -E make -f EEEmakefile install`).

If a standard EPICS installation is used, modify the appropriate files in the `configure` directory and then compile the plugin using make without any arguments. Note that this has not been tested.

A simple example illustrating how the plugin works is provided in the `m-epics-ADPluginKafka/startup` directory. When the plugin is installed:

* `cd` to `m-epics-ADPluginKafka/startup`.
* Run the startup script: `iocsh -r ADPluginKafka,1.0-BETA -c "requireSnippet(ADPluginKafka_demo.cmd)"`.

The example now also makes the PV:s available thorugh pvAccess (EPICS v4). It is possible that pvAccess is blocked on your developement machine, in which case the following commands can be used to remove all firewall rules:

    sudo iptables -F INPUT
    sudo iptables -F FORWARD

## Process variables (PV:s)
This plugin provides a few extra process variables (PV) besides the ones provided through inheritance from `NDPluginDriver`. The plugin also modifies one proccess variable inherited from `NDPluginDriver` directly. All the relevant PVs are listed below.

* `$(P)$(R)KafkaBrokerAddress` and `$(P)$(R)KafkaBrokerAddress_RBV` are used to set the address of one or more Kafka broker.The adress should include the port and have the following form:`address:port`. When using several addresses they should be seperated by a comma. Note that the text string is limited to 40 characters.
* `$(P)$(R)KafkaBrokerTopic` and `$(P)$(R)KafkaTopic_RBV` are used to set and retrieve the current topic. Limited to 40 characters.
* `$(P)$(R)ConnectionStatus_RBV` holds an integer corresponding to the current connection status. Se `ADPluginKafka.template` for possible values.
* `$(P)$(R)ConnectionMessage_RBV` is a PV that has a text message of at most 40 characters that gives information about the current conncetion status.
* `$(P)$(R)KafkaMaxQueueSize` and `$(P)$(R)KafkaMaxQueueSize_RBV` modifies and reads the number of packts allowed in the Kafka output buffer. Never set to a value < 1.
* `$(P)$(R)UnsentPackets_RBV` keeps track of the number of messages not yet transmitted to the Kafka broker. The minimum time between updates of this value is set by the next PV.
* `$(P)$(R)KafkaMaxMessageSize_RBV` is used to read the maximum message size allowed by librdkafka. This value should be updated automatically as message sizes exceeds their old values. The abosolute maximum size is approx. 953 MB.
* `$(P)$(R)KafkaStatsIntervalTime` and `$(P)$(R)KafkaStatsIntervalTime_RBV` are used to set and read the time between Kafka broker connection stats. This valueis given in milliseconds (ms). Setting a very short update time is not advised.

##To-do
This plugin is not yet production ready and several improvements are required. Some of these (in no particular order) are:

* **Improvements to error handling** There are several types of connection, buffer full and data transmission error that this plugin will not handle gracefully.
* **More PV:s** These are required for more fine grained control of the Kafka producer as well as for improvement in error handling.
* **Performance tests** It is likely that performance of the plugin could be improved. To determine if this is the case, performance tests and profiling of the code is required.
* **Modify db-template** The existing PV:s needs to be modified in order to be slightly more usefull.
* **Check data handling** The use of _processCallbacks()_ is somewhat convoluted and it should be checked that all calls are correct.
* **Kafka producer parameters** Kafka producer settings such as memory buffers, timeouts and so on needs to be adjusted for optimal performance.

