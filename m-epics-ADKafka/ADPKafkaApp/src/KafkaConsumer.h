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
#include <map>
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
    
    virtual std::map<std::string, PV_param> GetParams();
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
    
    std::map<std::string,PV_param> paramsList = {
        {"stats_tm", PV_param("KAFKA_STATS_INT", asynParamInt32)},
        {"msg_size", PV_param("KAFKA_MAX_MSG_SIZE", asynParamInt32)},
        {"status", PV_param("KAFKA_CONNECTION_STATUS", asynParamInt32)},
        {"message", PV_param("KAFKA_CONNECTION_MESSAGE", asynParamOctet)},
        {"offset", PV_param("KAFKA_MESSAGE_OFFSET", asynParamInt32)},
        {"queued", PV_param("KAFKA_UNPROCCESSED_MESSAGES", asynParamInt32)},
    };
};
}
