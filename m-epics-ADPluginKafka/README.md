# m-epics-KafkaPlugin
An EPICS areaDetector plugin which send areaDetector data serialized using flatbuffers to a Kafka broker. The current version is working but needs modifications to handle multi-threading

## Requirements
This project is not compatible with the latest version of the EPICS areaDetector module as it does not implement `pPlugin->start()` on configuration. This can easily be changed however. The `GNUmakefile` used to build this plugin is only compatible with the **ESS EPICS Environment** (EEE) used at ESS in Lund. It should be relatively easy to add make files for compiling the plugin with a generic EPICS installation.

For communicating with the Kafka broker, the C++ version of `librdkafka` is used. The source code for this library can be downloaded from [https://github.com/edenhill/librdkafka](https://github.com/edenhill/librdkafka). To simplify data handling, the plugin uses flatbuffers ([https://github.com/google/flatbuffers](https://github.com/google/flatbuffers)) for data serialization. `librdkafka` produces statistics messages in JSON and these are parsed using `jsoncpp` ([https://github.com/open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)).
In order to run the simple demo of the plugin in the `startup` folder, the module `adexample` is required as it contains features for running a simulated areaDetector.

## Compiling and running the example
There are currently two sets of make files provided with the repository. If the ESS EPICS Environment is available, the *EEEmakefile* should be used as an input to gnumake:

* `cd` to the `m-epics-KafkaPlugin` directory.
* Run gnumake (`make -f EEEmakefile`).
* Install using gnumake (`sudo -E make -f EEEmakefile install`).

If a standard EPICS installation is used, modify the appropriate files in the `configure` directory and then compile the plugin using make without any arguments. Note that this has not been tested.

A simple example illustrating how the plugin works is provided in the `KafkaPluginApp/startup` directory. When the plugin is installed:

* `cd` to `m-epics-kafkaplugin/startup`.
* Run the startup script: `iocsh -r kafkaplugin,1.0-ALPHA -c "requireSnippet(KafkaProducerDemo.cmd)"`.

The example now also makes the PV:s available thorugh pvAccess (EPICS v4). It is possible that pvAccess is blocked on your developement machine, in which case the following commands can be used to remove all firewall rules:

    sudo iptables -F INPUT
    sudo iptables -F FORWARD

## Process variables (PV:s)
This plugin provides a few extra process variables besides the ones provided through inheritance from `NDPluginDriver`. These are:

* `$(P)$(R)KafkaBrokers` used to set the address of one or more Kafka broker. The adress should include the port and have the following form:`address:port`. Several addresses can be separated with a blank space. Note that the text string is limited to 40 characters.
* `$(P)$(R)KafkaBrokers_RBV` is the read-back value PV of the Kafka brokers string.
* `$(P)$(R)KafkaBrokerTopic` is used to set the topic. Limited to 40 characters.
* `$(P)$(R)KafkaBrokerTopic_RBV` corresponding read-back value.
* `$(P)$(R)ConnectionStatus_RBV` holds an integer corresponding to the current connection status. Se `KafkaPLugin.template` for possible values.
* `$(P)$(R)ConnectionMessage_RBV` is a PV that has a text message of at most 40 characters that gives information about the current conncetion status.
* `$(P)$(R)UnsentPackets_RBV` keeps track of the number of messages not yet transmitted to the Kafka broker. Updated at most every 2 seconds.

##To-do
This plugin is not yet production ready and several improvements are required. Some of these (in no particular order) are:

* **Improvements to error handling** There are several types of connection, buffer full and data transmission error that this plugin will not handle gracefully.
* **More PV:s** These are required for more fine grained control of the Kafka producer as well as for improvement in error handling.
* **Multi-threading** Due to problems with EEE, multi threading is currently not working.
* **Unit tests** As this software is potentially going to be used in a production critical system, unit tests needs to exist.
* **Performance tests** It is likely that performance of the plugin could be improved. To determine if this is the case, performance tests and profiling of the code is required.
* **Modify db-template** The existing PV:s needs to be modified in order to be slightly more usefull.
* **Update stats on read** Update connection status and statistics when reading the relevant PV:s.
* **Check data handling** The use of _processCallbacks()_ is somewhat convoluted and it should be checked that all calls are correct.
* **Kafka producer parameters** Kafka producer settings such as memory buffers, timeouts and so on needs to be adjusted for optimal (and any) performance.

