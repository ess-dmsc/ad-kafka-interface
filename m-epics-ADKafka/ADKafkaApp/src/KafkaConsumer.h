//
//  KafkaConsumer.hpp
//  KafkaPlugin
//
//  Created by Jonas Nilsson on 2017-01-16.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#pragma once

#include <librdkafka/rdkafkacpp.h>
#include <string>
#include <vector>
#include "ParamUtility.h"
#include "json.h"

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
    KafkaConsumer(std::string topic, std::string broker, std::string groupId = "KF");
    KafkaConsumer(std::string groupId = "KF");
    ~KafkaConsumer();
    
    virtual void RegisterParamCallbackClass(NDPluginDriver *ptr);
    
    virtual bool SetTopic(std::string topicName);
    
    virtual bool SetBrokerAddr(std::string brokerAddr);
    
    virtual KafkaMessage* WaitForPkg(int timeout);
    
    virtual std::int64_t GetCurrentOffset();
    
    virtual void SetOffset(std::int64_t offset);
    
    virtual bool SetGroupId(std::string groupId);
    
    virtual bool SetStatsTime(int time);
    
    virtual std::vector<PV_param> GetParams();
    
    static int GetNumberOfPVs();
protected:
    bool errorState = false;
    
    size_t bufferSize = 100000000;
    
    std::int64_t topicOffset;
    
    enum class ConStat { CONNECTED = 0, CONNECTING = 1, DISCONNECTED = 2, ERROR = 3, };
    
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
    
    //Some configuration values
    const int kafka_stats_interval = 500; //In ms
    const int sleepTime = 50; //Milliseconds sleeping between poll()-calls
    
    void event_cb(RdKafka::Event &event);
    
    NDPluginDriver *paramCallback;
    
    std::string topicName;
    std::string brokerAddrStr;
    
    //Variables used by the Kafka producer.
    std::string errstr;
    RdKafka::Conf *conf = nullptr;
    RdKafka::KafkaConsumer *consumer = nullptr;
    
    //Variables used by the JSON parser
    Json::Value root, brokers;
    Json::Reader reader;
    
    enum PV {
        stats_time,
        max_msg_size,
        con_status,
        con_msg,
        msg_offset,
        msgs_in_queue,
        count,
    };
    
    std::vector<PV_param> paramsList = {
        PV_param("KAFKA_STATS_TIME", asynParamInt32), //stats_time
        PV_param("KAFKA_MAX_MSG_SIZE", asynParamInt32), //max_msg_size
        PV_param("KAFKA_CONNECTION_STATUS", asynParamInt32), //con_status
        PV_param("KAFKA_CONNECTION_MESSAGE", asynParamOctet), //con_msg
        PV_param("KAFKA_MESSAGE_OFFSET", asynParamInt32), //msg_offset
        PV_param("KAFKA_UNPROCCESSED_MESSAGES", asynParamInt32), //msgs_in_queue
    };
};
}
