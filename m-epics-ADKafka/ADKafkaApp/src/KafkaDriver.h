/** Copyright (C) 2016 European Spallation Source */

/** @file  KafkaPlugin.h
 *  @brief Header file for the C++ implementation of an EPICS areaDetector Kafka-plugin.
 */

#pragma once

#include <epicsEvent.h>
#include <ADDriver.h>
#include <string>
#include <map>

#include "ParamUtility.h"
#include "KafkaConsumer.h"

using KafkaInterface::KafkaConsumer;

class epicsShareClass KafkaDriver : public ADDriver {
  public:
    KafkaDriver(const char *portName, int maxBuffers, size_t maxMemory, int priority, int stackSize, const char *brokerAddress, const char *brokerTopic);
    
    ~KafkaDriver();
    
    /** @brief Used to set the string parameters of the Kafka producer.
     * If a configuration string is updated, the Kafka prdoucer will be immediatly destroyed and
     * re-created using the new parameter. This means that unsent data will be lost.
     * Based on code used in one of the standard plugins. Parameter description is taken from the
     * documentation of NDPluginDriver::writeOctet().
     * @param[in] pasynUser pasynUser structure that encodes the reason and address.
     * @param[in] value Address of the string to write.
     * @param[in] nChars Number of characters to write.
     * @param[out] nActual Number of characters actually written.
     */
    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);
    
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    
    virtual void consumeTask();

  protected:
    int MIN_PARAM_INDEX;
    KafkaConsumer consumer;
    
    epicsEventId startEventId_;
    epicsEventId stopEventId_;
    epicsEventId threadExitEventId_;
    
    enum PV {
        kafka_addr,
        kafka_topic,
        kafka_group,
        stats_time,
        set_offset,
        count,
    };
    
    enum OffsetSetting {
        Beginning = 0,
        Stored = 1,
        Manual = 2,
        End = 3,
    };
    
    OffsetSetting usedOffsetSetting;
    
    std::vector<PV_param> paramsList = {
        PV_param("KAFKA_BROKER_ADDRESS", asynParamOctet), //kafka_addr
        PV_param("KAFKA_TOPIC", asynParamOctet), //kafka_topic
        PV_param("KAFKA_GROUP", asynParamOctet), //kafka_group
        PV_param("KAFKA_STATS_INT_MS", asynParamInt32), //stats_time
        PV_param("KAFKA_SET_OFFSET", asynParamInt32), //set_offset
    };
    bool keepThreadAlive;
};
