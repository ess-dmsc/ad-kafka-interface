/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaConsumer.h
 *  @brief Header file of a Kafka consumer class. Used together with an
 * areaDetector driver.
 */

#pragma once

#include "ParamUtility.h"
#include "json.h"
#include <asynNDArrayDriver.h>
#include <librdkafka/rdkafkacpp.h>
#include <string>
#include <vector>

namespace KafkaInterface {
class KafkaMessage {
  public:
    KafkaMessage(RdKafka::Message *msg);
    ~KafkaMessage();
    void *GetDataPtr();
    size_t size();

  private:
    RdKafka::Message *msg;
};

class KafkaConsumer : public RdKafka::EventCb {
  public:
    KafkaConsumer(std::string broker, std::string topic, std::string groupId = "KF");
    KafkaConsumer(std::string groupId = "KF");
    ~KafkaConsumer();

    virtual void RegisterParamCallbackClass(asynNDArrayDriver *ptr);

    virtual bool SetTopic(std::string topicName);
    virtual std::string GetTopic();

    virtual bool SetBrokerAddr(std::string brokerAddr);
    virtual std::string GetBrokerAddr();

    virtual KafkaMessage *WaitForPkg(int timeout);

    virtual void StartConsumption();
    virtual void StopConsumption();

    virtual std::int64_t GetCurrentOffset();

    virtual bool SetOffset(std::int64_t offset);

    virtual int GetOffsetPVIndex();

    virtual bool SetGroupId(std::string groupId);
    virtual std::string GetGroupId();

    virtual bool SetStatsTimeMS(int time);
    virtual int GetStatsTimeMS();

    //    virtual bool SetMessageQueueLength(int queue);
    //    virtual int GetMessageQueueLength();

    virtual std::vector<PV_param> &GetParams();

    static int GetNumberOfPVs();

  protected:
    bool errorState = false;
    bool consumptionHalted = true;

    size_t bufferSize = 100000000;

    std::int64_t topicOffset;

    PV_param offsetParam;

    enum class ConStat {
        CONNECTED = 0,
        CONNECTING = 1,
        DISCONNECTED = 2,
        ERROR = 3,
    };

    /** @brief Sets the correct status PV:s.
     * Will call KafkaPlugin::DestroyKafkaConnection() if the status id is equal to
     * KafkaPlugin::ERROR.
     * @param[in] stat Takes an integer value representing the current status of the Kafka system.
     * Should be a KafkaPlugin::ConStat enum value.
     * @param[in] msg Text string which represents the current status of the Kafka system. Can not
     * be more than 40 characters.
     */
    virtual void SetConStat(ConStat stat, std::string msg);

    virtual void InitRdKafka(std::string groupId);

    virtual bool MakeConnection();

    virtual bool UpdateTopic();

    virtual void ParseStatusString(std::string msg);

    // Some configuration values
    int kafka_stats_interval = 500; // In ms
    const int sleepTime = 50;       // Milliseconds sleeping between poll()-calls

    void event_cb(RdKafka::Event &event);

    asynNDArrayDriver *paramCallback;

    std::string topicName;
    std::string brokerAddrStr;
    std::string groupName;

    // Variables used by the Kafka producer.
    std::string errstr;
    RdKafka::Conf *conf = nullptr;
    RdKafka::KafkaConsumer *consumer = nullptr;

    // Variables used by the JSON parser
    Json::Value root, brokers;
    Json::Reader reader;

    enum PV {
        max_msg_size,
        con_status,
        con_msg,
        msg_offset,
        msgs_in_queue,
        count,
    };

    std::vector<PV_param> paramsList = {
        PV_param("KAFKA_MAX_MSG_SIZE", asynParamInt32),          // max_msg_size
        PV_param("KAFKA_CONNECTION_STATUS", asynParamInt32),     // con_status
        PV_param("KAFKA_CONNECTION_MESSAGE", asynParamOctet),    // con_msg
        PV_param("KAFKA_CURRENT_OFFSET", asynParamInt32),        // msg_offset
        PV_param("KAFKA_UNPROCCESSED_MESSAGES", asynParamInt32), // msgs_in_queue
    };
};
}
