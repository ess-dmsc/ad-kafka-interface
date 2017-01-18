//
//  KafkaProducer.hpp
//  KafkaPlugin
//
//  Created by Jonas Nilsson on 2017-01-11.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <librdkafka/rdkafkacpp.h>
#include <iostream>
#include "ParamUtility.h"
#include "NDPluginDriver.h"
#include "json.h"

namespace KafkaInterface {
    
    class KafkaProducer : public RdKafka::EventCb {
        //@todo This class copies the data that is to be sent, make it so that it does not have to.
    public:
        KafkaProducer(std::string topic, std::string broker);
        
        KafkaProducer();
        
        ~KafkaProducer();
        
        virtual std::map<std::string, PV_param> GetParams();
        
        virtual void RegisterParamCallbackClass(NDPluginDriver *ptr);
        
        virtual bool SetTopic(std::string topicName);
        
        virtual bool SetBrokerAddr(std::string brokerAddr);
        
        virtual bool SetMaxMessageSize(size_t msgSize);
        
        virtual bool SetMessageQueueLength(int queue);
        
        virtual bool SetStatsTime(int time);
        
        virtual void AttemptFlushAtReconnect(bool flush, int flushTime);
        
        /** @brief Starts the thread that keeps track of the status of the Kafka connection.
         * @note Call this thread only after the PV parameters have been registered with the
         * EPICS subsystem as the indexes are not protected against simultaneous access from
         * different threads.
         */
        virtual bool StartThread();
        
        /** @brief Sends the binary data stored in the buffer to the Kafka broker.
         * \todo Complete documentation.
         */
        virtual bool SendKafkaPacket(unsigned char *buffer, size_t buffer_size);
    protected:
        size_t maxMessageSize; // In bytes
        int msgQueueSize;
        
        bool doFlush;       //Should we wait for existing messages to be sent before we close the connection?
        int flushTimeout;   //For how long should we wait.
        bool errorState;    //Are we unable to init librdkafka?
        
        virtual void ShutDownTopic();
        
        virtual void ShutDownProducer();
        
        /** @brief Callback member function used by the status and error handling system of librdkafka.
         * This member function is registered as a callback function with librdkafka for error and
         * status messages. Status messages are received as JSON strings which are decoded using
         * jsoncpp. Other events are currently barely handled.
         * @param[in] event RdKafka::Event instance that holds information on statistics, errors or
         * other events.
         */
        virtual void event_cb(RdKafka::Event &event);
        
        /** @brief Thread member function. Should only be called by KafkaProducer::StartThread().
         */
        virtual void ThreadFunction();
        
        //Kafka connection status enum
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
        
        /** @brief Parses JSON status string from Kafka system and updates PV:s.
         * Uses jsoncpp to parse the status string and extracts current connection status of available
         * brokers as well as the number of messages not yet transmitted to the Kafka broker. This
         * information is then used to update the relevant PV:s.
         * @param[in] msg JSON status message obtained from the Kafka producer system.
         */
        virtual void ParseStatusString(std::string msg);
        
        //Some configuration values
        const int kafka_stats_interval = 500; //In ms
        const int sleepTime = 50; //Milliseconds sleeping between poll()-calls
        
        mutable std::mutex brokerMutex;
        
        /** @brief Attempts to init the Kafka producer system of librdkafka.
         * Failure to init the Kafka system results in a error message written to the relevant PV and
         * an attempt in cleaning up (deleting) all the variables related to the Kafka producer.
         * Note that this function does not actually connect to a Kafka broker but instead starts the
         * Kafka producer system that will attempt to connect to a broker.
         */
        virtual void InitRdKafka();
        
        virtual bool MakeConnection();
        
        //Variables used by the Kafka producer.
        std::string errstr;
        RdKafka::Conf *conf = nullptr;
        RdKafka::Conf *tconf = nullptr;
        RdKafka::Topic *topic = nullptr;
        RdKafka::Producer *producer = nullptr;
        
        std::string topicName;
        std::string brokerAddrStr;
        
        //Variables used by the JSON parser
        Json::Value root, brokers;
        Json::Reader reader;
        
        std::thread statusThread;
        
        NDPluginDriver *paramCallback;
        
        std::atomic_bool runThread;
        
        std::map<std::string,PV_param> paramsList = {
            {"stats_tm", PV_param("KAFKA_STATS_INT", asynParamInt32)},
            {"msg_size", PV_param("KAFKA_MAX_MSG_SIZE", asynParamInt32)},
            {"status", PV_param("KAFKA_CONNECTION_STATUS", asynParamInt32)},
            {"message", PV_param("KAFKA_CONNECTION_MESSAGE", asynParamOctet)},
            {"queued", PV_param("KAFKA_UNSENT_PACKETS", asynParamInt32)},
        };
    };
}
