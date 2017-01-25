//
//  KafkaProducer.cpp
//  KafkaPlugin
//
//  Created by Jonas Nilsson on 2017-01-11.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#include <chrono>
#include "KafkaProducer.h"

namespace KafkaInterface {
    
    int KafkaProducer::GetNumberOfPVs() {
        return PV::count;
    }
    
    KafkaProducer::KafkaProducer(std::string topic, std::string broker, int queueSize) : errorState(false), doFlush(true), topic(nullptr), producer(nullptr), conf(nullptr), tconf(nullptr), flushTimeout(500), maxMessageSize(1000000), topicName(topic), brokerAddrStr(broker), runThread(false), paramCallback(nullptr), msgQueueSize(queueSize) {
        InitRdKafka();
        MakeConnection();
    }
    
    KafkaProducer::KafkaProducer(int queueSize) : errorState(false), doFlush(true), topic(nullptr), producer(nullptr), conf(nullptr), tconf(nullptr), flushTimeout(500), maxMessageSize(1000000), runThread(false), paramCallback(nullptr), msgQueueSize(queueSize) {
        InitRdKafka();
    }
    
    KafkaProducer::~KafkaProducer() {
        if (runThread) {
            runThread = false;
            statusThread.join();
        }
        ShutDownTopic();
        ShutDownProducer();
        delete tconf;
        tconf = nullptr;
        delete conf;
        conf = nullptr;
    }
    
    bool KafkaProducer::StartThread() {
        if (not runThread) {
            if (errorState) {
                SetConStat(KafkaProducer::ConStat::ERROR, "Unable to init kafka sub-system.");
                return false;
            }
            SetConStat(KafkaProducer::ConStat::DISCONNECTED, "Starting status thread.");
            runThread = true;
            statusThread = std::thread(&KafkaProducer::ThreadFunction, this);
            return true;
        }
        return false;
    }
    
    void KafkaProducer::ThreadFunction() {
        std::chrono::milliseconds sleepTime(KafkaProducer::sleepTime);
        while (runThread) {
            std::this_thread::sleep_for(sleepTime);
            {
                std::lock_guard<std::mutex> lock(brokerMutex);
                if (producer != nullptr and topic != nullptr) {
                    producer->poll(0);
                }
            }
        }
    }
    
    bool KafkaProducer::SetMaxMessageSize(size_t msgSize) {
        if (errorState or 0 == msgSize) {
            return false;
        }
        RdKafka::Conf::ConfResult configResult1, configResult2;
        configResult1 = conf->set("message.max.bytes", std::to_string(msgSize), errstr);
        configResult2 = conf->set("message.copy.max.bytes", std::to_string(msgSize), errstr);
        if (RdKafka::Conf::CONF_OK != configResult1 or RdKafka::Conf::CONF_OK != configResult2) {
            SetConStat(KafkaProducer::ConStat::ERROR, "Unable to set max message size.");
            return false;
        }
        maxMessageSize = msgSize;
        setParam(paramCallback, paramsList[PV::max_msg_size], int(msgSize));
        ShutDownTopic();
        ShutDownProducer();
        MakeConnection();
        return true;
    }
    
    size_t KafkaProducer::GetMaxMessageSize() {
        return maxMessageSize;
    }
    
    
    bool KafkaProducer::SetMessageQueueLength(int queue) {
        if (errorState or 0 >= queue) {
            return false;
        }
        RdKafka::Conf::ConfResult configResult;
        configResult = conf->set("queue.buffering.max.messages", std::to_string(queue), errstr);
        if (RdKafka::Conf::CONF_OK != configResult) {
            SetConStat(KafkaProducer::ConStat::ERROR, "Unable to set message queue length.");
            return false;
        }
        msgQueueSize = queue;
        ShutDownTopic();
        ShutDownProducer();
        MakeConnection();
        return true;
    }
    
    int KafkaProducer::GetMessageQueueLength() {
        return msgQueueSize;
    }
    
    bool KafkaProducer::SendKafkaPacket(unsigned char *buffer, size_t buffer_size) {
        if (errorState or 0 == buffer_size) {
            return false;
        }
        if (buffer_size> maxMessageSize) {
            bool success = SetMaxMessageSize(buffer_size);
            if (not success) {
                errorState = true;
                return false;
            }
        }
        std::lock_guard<std::mutex> lock(brokerMutex);
        if (nullptr == producer or nullptr == topic) {
            return false;
        }
        RdKafka::ErrorCode resp = producer->produce(
                                                    topic, -1, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                                                    buffer, buffer_size, NULL, NULL);
        
        if (RdKafka::ERR_NO_ERROR != resp) {
            SetConStat(KafkaProducer::ConStat::ERROR,
                       "Producer failed with error code: " + std::to_string(resp));
            return false;
        }
        return true;
    }
    
    void KafkaProducer::event_cb(RdKafka::Event &event) {
        //@todo This member function really needs some expanded capability
        switch (event.type()) {
            case RdKafka::Event::EVENT_ERROR:
                if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN) {
                    SetConStat(KafkaProducer::ConStat::DISCONNECTED, "Brokers down. Attempting to reconnect.");
                } else {
                    SetConStat(KafkaProducer::ConStat::DISCONNECTED,
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
    
    void KafkaProducer::SetConStat(KafkaProducer::ConStat stat, std::string msg) {
        //std::cout << int(stat) << " : " << msg << std::endl;
        setParam(paramCallback, paramsList.at(PV::con_status), int(stat));
        setParam(paramCallback, paramsList.at(PV::con_msg), msg);
    }
    
    void KafkaProducer::ParseStatusString(std::string msg) {
        //@todo We should probably extract some more stats from the JSON message
        bool parseSuccess = reader.parse(msg, root);
        if (not parseSuccess) {
            SetConStat(KafkaProducer::ConStat::ERROR, "Status msg.: Unable to parse.");
            return;
        }
        brokers = root["brokers"];
        if (brokers.isNull() or brokers.size() == 0) {
            SetConStat(KafkaProducer::ConStat::ERROR, "Status msg.: No brokers.");
        } else {
            KafkaProducer::ConStat tempStat = KafkaProducer::ConStat::DISCONNECTED;
            std::string statString = "Brokers down. Attempting reconnection.";
            for (auto b : brokers) {
                if ("UP" == b["state"].asString()) {
                    tempStat = KafkaProducer::ConStat::CONNECTED;
                    statString = "No errors.";
                    break;
                }
            }
            SetConStat(tempStat, statString);
        }
        int unsentMessages = root["msg_cnt"].asInt();
        setParam(paramCallback, paramsList.at(PV::msgs_in_queue), unsentMessages);
    }
    
    void KafkaProducer::AttemptFlushAtReconnect(bool flush, int flushTime) {
        doFlush = flush;
        KafkaProducer::flushTimeout = flushTime;
    }
    
    void KafkaProducer::InitRdKafka() {
        conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
        
        if (NULL == conf) {
            errorState = true;
            SetConStat(KafkaProducer::ConStat::ERROR, "Can not create global conf object.");
            return;
        }
        
        if (NULL == tconf) {
            errorState = true;
            SetConStat(KafkaProducer::ConStat::ERROR, "Can not create topic conf object.");
            return;
        }
        
        RdKafka::Conf::ConfResult configResult;
        configResult = conf->set("event_cb", this, errstr);
        if (RdKafka::Conf::CONF_OK != configResult) {
            errorState = true;
            SetConStat(KafkaProducer::ConStat::ERROR, "Can not set event callback.");
            return;
        }
        
        configResult = conf->set("statistics.interval.ms", std::to_string(kafka_stats_interval), errstr);
        if (RdKafka::Conf::CONF_OK != configResult) {
            SetConStat(KafkaProducer::ConStat::ERROR, "Unable to set statistics interval.");
        }
        
        configResult = conf->set("queue.buffering.max.messages", std::to_string(msgQueueSize), errstr);
        if (RdKafka::Conf::CONF_OK != configResult) {
            SetConStat(KafkaProducer::ConStat::ERROR, "Unable to set queue length.");
        }
    }
    
    bool KafkaProducer::SetStatsTimeMS(int time) {
        if (errorState or time <= 0) {
            return false;
        }
        RdKafka::Conf::ConfResult configResult;
        configResult = conf->set("statistics.interval.ms", std::to_string(time), errstr);
        if (RdKafka::Conf::CONF_OK != configResult) {
            SetConStat(KafkaProducer::ConStat::ERROR, "Unable to set statistics interval.");
            return false;
        }
        kafka_stats_interval = time;
        //setParam(paramCallback, paramsList[PV::stats_time], time);
        ShutDownTopic();
        ShutDownProducer();
        MakeConnection();
        return true;
    }
    
    int KafkaProducer::GetStatsTimeMS() {
        return kafka_stats_interval;
    }
    
    bool KafkaProducer::SetTopic(std::string topicName) {
        if (errorState or 0 == topicName.size()) {
            return false;
        }
        KafkaProducer::topicName = topicName;
        brokerMutex.lock();
        if (nullptr != topic and nullptr != producer) {
            brokerMutex.unlock();
            ShutDownTopic();
        } else {
            brokerMutex.unlock();
        }
        std::lock_guard<std::mutex> lock(brokerMutex);
        if (nullptr != producer) {
            topic = RdKafka::Topic::create(producer, topicName, tconf, errstr);
            if (nullptr == topic) {
                SetConStat(KafkaProducer::ConStat::ERROR, "Unable to create topic.");
                return false;
            } else {
                SetConStat(KafkaProducer::ConStat::CONNECTING, "Connecting to topic.");
                return true;
            }
        }
        return true;
    }
    
    std::string KafkaProducer::GetTopic() {
        return topicName;
    }
    
    bool KafkaProducer::SetBrokerAddr(std::string brokerAddr) {
        if (errorState or brokerAddr.size() == 0) {
            return false;
        }
        RdKafka::Conf::ConfResult cRes;
        cRes = conf->set("metadata.broker.list", brokerAddr, errstr);
        if (RdKafka::Conf::CONF_OK != cRes) {
            SetConStat(KafkaProducer::ConStat::ERROR, "Can not set new broker.");
            return false;
        }
        brokerAddrStr = brokerAddr;
        brokerMutex.lock();
        if (nullptr != topic) {
            brokerMutex.unlock();
            ShutDownTopic();
            ShutDownProducer();
        } else {
            brokerMutex.unlock();
        }
        MakeConnection();
        return true;
    }
    
    std::string KafkaProducer::GetBrokerAddr() {
        return brokerAddrStr;
    }
    
    bool KafkaProducer::MakeConnection() {
        std::lock_guard<std::mutex> lock(brokerMutex);
        if (nullptr == producer and nullptr == topic) {
            if (brokerAddrStr.size() > 0) {
                producer = RdKafka::Producer::create(conf, errstr);
                if (NULL == producer) {
                    SetConStat(KafkaProducer::ConStat::ERROR, "Unable to create producer.");
                    return false;
                }
                if (topicName.size() != 0) {
                    topic = RdKafka::Topic::create(producer, topicName, tconf, errstr);
                    if (nullptr == topic) {
                        SetConStat(KafkaProducer::ConStat::ERROR, "Unable to create topic.");
                        return false;
                    }
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (nullptr != producer and nullptr == topic) {
            if (topicName.size() != 0) {
                topic = RdKafka::Topic::create(producer, topicName, tconf, errstr);
                if (nullptr == topic) {
                    SetConStat(KafkaProducer::ConStat::ERROR, "Unable to create topic.");
                    return false;
                }
                return true;
            } else {
                return false;
            }
        } else if (nullptr != producer and nullptr != topic) {
            return true;
        }
        assert(false);
        return true;
    }
    
    void KafkaProducer::ShutDownProducer() {
        brokerMutex.lock();
        if (nullptr != topic) {
            brokerMutex.unlock();
            ShutDownTopic();
            brokerMutex.lock();
        }
        if (nullptr != producer) {
            delete producer;
            producer = nullptr;
        }
        brokerMutex.unlock();
    }
    
    void KafkaProducer::ShutDownTopic() {
        std::lock_guard<std::mutex> lock(brokerMutex);
        if (nullptr != topic) {
            if (doFlush) {
                int res = producer->flush(flushTimeout);
                if (RdKafka::ERR__TIMED_OUT == res) {
                    SetConStat(KafkaProducer::ConStat::DISCONNECTED, "Timed out when waiting for msg flush.");
                } else if (RdKafka::ERR_NO_ERROR == res) {
                    //Do nothing on no error
                } else {
                    SetConStat(KafkaProducer::ConStat::DISCONNECTED, "Unknown error when waiting for msg flush.");
                }
            }
            delete topic;
            topic = nullptr;
        }
    }
    
    std::vector<PV_param> &KafkaProducer::GetParams() {
        return paramsList;
    }
    
    void KafkaProducer::RegisterParamCallbackClass(NDPluginDriver *ptr) {
        paramCallback = ptr;
    }
}
