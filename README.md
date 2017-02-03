# Area detector kafka interface
This repository contains two seperate projects which facilitates the transmission of data between an EPICS IOC and Kafka brokers. The two projects are:

* An EPICS areaDetector driver which acts as a Kafka consumer and makes NDArray data recived from the broker available to the IOC and over CA/PV-access.
* An EPICS areaDetector plugin which connects to an areaDetector and serializes NDArray data it receives and sends it to a Kafka broker.

More information about the two projects can be found in their respesctive directories and README-files.

The repository also contains a directory with code for unit tests of the two projects. Do note that the build system of the unit tests might require some modification to work on your system.

