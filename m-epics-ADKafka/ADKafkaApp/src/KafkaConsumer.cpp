//
//  KafkaConsumer.cpp
//  KafkaPlugin
//
//  Created by Jonas Nilsson on 2017-01-16.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#include "KafkaConsumer.h"
namespace KafkaInterface {
    
    KafkaMessage::KafkaMessage(RdKafka::Message *msg) : msg(msg) {
        
    }
    
    KafkaMessage::~KafkaMessage() {
        delete msg;
    }
    
    void* KafkaMessage::GetDataPtr() {
        return msg->payload();
    }
    
    size_t KafkaMessage::size() {
        return msg->len();
    }
    
    KafkaConsumer::KafkaConsumer(std::string topic, std::string broker, std::string groupId) : topicOffset(RdKafka::Topic::OFFSET_END), consumer(nullptr), paramCallback(nullptr), conf(nullptr) {
        InitRdKafka(groupId);
        SetBrokerAddr(broker);
        SetTopic(topic);
    }
    
    KafkaConsumer::KafkaConsumer(std::string groupId) : topicOffset(RdKafka::Topic::OFFSET_END), consumer(nullptr), paramCallback(nullptr), conf(nullptr) {
        InitRdKafka(groupId);
    }
    
    KafkaConsumer::~KafkaConsumer() {
        if (nullptr != consumer) {
            consumer->unassign();
            consumer->close();
            delete consumer;
            consumer = nullptr;
        }
        delete conf;
        conf = nullptr;
    }
    
    void KafkaConsumer::InitRdKafka(std::string groupId) {
        conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        
        if (NULL == conf) {
            errorState = true;
            SetConStat(KafkaConsumer::ConStat::ERROR, "Can not create global conf object.");
            return;
        }
        
        RdKafka::Conf::ConfResult configResult;
        configResult = conf->set("event_cb", this, errstr);
        if (RdKafka::Conf::CONF_OK != configResult) {
            errorState = true;
            SetConStat(KafkaConsumer::ConStat::ERROR, "Can not set event callback.");
            return;
        }
        
        configResult = conf->set("statistics.interval.ms", std::to_string(kafka_stats_interval), errstr);
        if (RdKafka::Conf::CONF_OK != configResult) {
            SetConStat(KafkaConsumer::ConStat::ERROR, "Unable to set statistics interval.");
        }
        
        if (groupId.size() == 0) {
            SetConStat(KafkaConsumer::ConStat::ERROR, "Unable to set group id.");
            errorState = true;
            return;
        }
        configResult = conf->set("group.id", groupId, errstr);
        if (RdKafka::Conf::CONF_OK != configResult) {
            SetConStat(KafkaConsumer::ConStat::ERROR, "Unable to set group id.");
            errorState = true;
            return;
        }
    }
    
    std::map<std::string, PV_param> KafkaConsumer::GetParams() {
        return paramsList;
    }
    
    void KafkaConsumer::SetOffset(std::int64_t offset) {
        topicOffset = offset;
        UpdateTopic();
    }
    
    KafkaMessage* KafkaConsumer::WaitForPkg(int timeout) {
        if (nullptr != consumer and topicName.size() > 0) {
            RdKafka::Message *msg = consumer->consume(timeout);
            if (msg->err() == RdKafka::ERR_NO_ERROR) {
                topicOffset = msg->offset();
                return new KafkaMessage(msg);
            } else {
                delete msg;
                return nullptr;
            }
        } else {
            return nullptr;
        }
        return nullptr;
    }
    
    void KafkaConsumer::event_cb(RdKafka::Event &event) {
        //@todo This member function really needs some expanded capability
        switch (event.type()) {
            case RdKafka::Event::EVENT_ERROR:
                if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN) {
                    SetConStat(KafkaConsumer::ConStat::DISCONNECTED, "Brokers down. Attempting to reconnect.");
                } else {
                    SetConStat(KafkaConsumer::ConStat::DISCONNECTED,
                               "Event error received: " + std::to_string(event.err()));
                }
                break;
            case RdKafka::Event::EVENT_LOG:
                //@todo Add message/log or something
                break;
            case RdKafka::Event::EVENT_THROTTLE:
                //@todo Add message/log or something
                break;
            case RdKafka::Event::EVENT_STATS:
                ParseStatusString(event.str());
                break;
            default:
                if ((event.type() == RdKafka::Event::EVENT_LOG) and
                    (event.severity() == RdKafka::Event::EVENT_SEVERITY_ERROR)) {
                    //@todo Add message/log or something
                    
                } else {
                    //@todo Add message/log or something
                }
        }
    }
    
    void KafkaConsumer::ParseStatusString(std::string msg) {
        //@todo We should probably extract some more stats from the JSON message
        bool parseSuccess = reader.parse(msg, root);
        if (not parseSuccess) {
            SetConStat(KafkaConsumer::ConStat::ERROR, "Status msg.: Unable to parse.");
            return;
        }
        brokers = root["brokers"];
        if (brokers.isNull() or brokers.size() == 0) {
            SetConStat(KafkaConsumer::ConStat::ERROR, "Status msg.: No brokers.");
        } else {
            KafkaConsumer::ConStat tempStat = KafkaConsumer::ConStat::DISCONNECTED;
            std::string statString = "Brokers down. Attempting reconnection.";
            for (auto b : brokers) {
                if ("UP" == b["state"].asString()) {
                    tempStat = KafkaConsumer::ConStat::CONNECTED;
                    statString = "No errors.";
                    break;
                }
            }
            SetConStat(tempStat, statString);
        }
        int unsentMessages = root["msg_cnt"].asInt();
        setParam(paramCallback, paramsList.at("queued"), unsentMessages);
    }
    
    std::int64_t KafkaConsumer::GetCurrentOffset() {
        return topicOffset;
    }
    
    bool KafkaConsumer::UpdateTopic() {
        if (nullptr != consumer and topicName.size() > 0) {
            consumer->unassign();
            std::vector<RdKafka::TopicPartition*> topics;
            topics.push_back(RdKafka::TopicPartition::create(topicName, 0, topicOffset));
            consumer->assign(topics);
        }
        return true;
    }
    
    bool KafkaConsumer::MakeConnection() {
        if (consumer != nullptr) {
            consumer->unassign();
            consumer->close();
            delete consumer;
            consumer = nullptr;
        }
        if (brokerAddrStr.size() > 0) {
            consumer = RdKafka::KafkaConsumer::create(conf, errstr);
            if (nullptr == consumer) {
                SetConStat(KafkaConsumer::ConStat::ERROR, "Unable to create consumer.");
                return false;
            }
            UpdateTopic();
        }
        return true;
    }
    
    bool KafkaConsumer::SetTopic(std::string topicName) {
        if (errorState or 0 == topicName.size()) {
            return false;
        }
        KafkaConsumer::topicName = topicName;
        UpdateTopic();
        return true;
    }
    
    bool KafkaConsumer::SetBrokerAddr(std::string brokerAddr) {
        if (errorState or brokerAddr.size() == 0) {
            return false;
        }
        RdKafka::Conf::ConfResult cRes;
        cRes = conf->set("metadata.broker.list", brokerAddr, errstr);
        if (RdKafka::Conf::CONF_OK != cRes) {
            SetConStat(KafkaConsumer::ConStat::ERROR, "Can not set new broker.");
            return false;
        }
        brokerAddrStr = brokerAddr;
        MakeConnection();
        return true;
    }
    
    bool KafkaConsumer::SetGroupId(std::string groupId) {
        if (errorState or groupId.size() == 0) {
            return false;
        }
        RdKafka::Conf::ConfResult cRes;
        cRes = conf->set("group.id", groupId, errstr);
        if (RdKafka::Conf::CONF_OK != cRes) {
            SetConStat(KafkaConsumer::ConStat::ERROR, "Can not set new group id.");
            return false;
        }
        MakeConnection();
        return true;
    }
    
    void KafkaConsumer::SetConStat(ConStat stat, std::string msg) {
        //std::cout << int(stat) << " : " << msg << std::endl;
        setParam(paramCallback, paramsList.at("status"), int(stat));
        setParam(paramCallback, paramsList.at("message"), msg);
    }
    
    void KafkaConsumer::RegisterParamCallbackClass(NDPluginDriver *ptr) {
        paramCallback = ptr;
    }
    
    bool KafkaConsumer::SetStatsTime(int time) {
        if (errorState or time <= 0) {
            return false;
        }
        RdKafka::Conf::ConfResult configResult;
        configResult = conf->set("statistics.interval.ms", std::to_string(kafka_stats_interval), errstr);
        if (RdKafka::Conf::CONF_OK != configResult) {
            SetConStat(KafkaConsumer::ConStat::ERROR, "Unable to set statistics interval.");
            return false;
        }
        MakeConnection();
        return true;
    }
}
