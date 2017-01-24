//
//  KafkaProducerTest.cpp
//  KafkaPlugin
//
//  Created by Jonas Nilsson on 2017-01-11.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "KafkaProducer.h"

namespace KafkaInterface {
    
    class KafkaProducerStandIn : public KafkaProducer {
    public:
        KafkaProducerStandIn() : KafkaProducer() {};
        KafkaProducerStandIn(std::string addr, std::string topic): KafkaProducer(addr, topic) {};
        using KafkaProducer::errorState;
        using KafkaProducer::ConStat;
        using KafkaProducer::kafka_stats_interval;
        using KafkaProducer::PV;
        void SetConStatParent(KafkaProducerStandIn::ConStat stat, std::string msg) {KafkaProducer::SetConStat(stat, msg);};
        bool MakeConnectionParent() {return KafkaProducer::MakeConnection();};
        MOCK_METHOD0(MakeConnection, bool(void));
        MOCK_METHOD2(SetConStat, void(KafkaProducerStandIn::ConStat, std::string));
    private:
        
    };
    
    class NDPluginDriverStandIn : public NDPluginDriver {
    public:
        NDPluginDriverStandIn(const char *portName, int queueSize, int blockingCallbacks, const char *NDArrayPort, int NDArrayAddr, int maxAddr, int numParams, int maxBuffers, size_t maxMemory, int interfaceMask, int interruptMask, int asynFlags, int autoConnect, int priority, int stackSize) : NDPluginDriver(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr, maxAddr, numParams, maxBuffers, maxMemory, interfaceMask, interruptMask, asynFlags, autoConnect, priority, stackSize) {};
        MOCK_METHOD2(setStringParam, asynStatus(int, const char*));
        MOCK_METHOD2(setIntegerParam, asynStatus(int, int));
        MOCK_METHOD3(createParam, asynStatus(const char*, asynParamType, int*));
    };
    
    class KafkaProducerEnv : public ::testing::Test {
    public:
        static void SetUpTestCase() {
            std::string portName("someNameFirst");
            int queueSize = 10;
            int blockingCallbacks = 0;
            std::string NDArrayPort("NDArrayPortName");
            int NDArrayAddr = 42;
            int numberOfParams = 0;
            size_t maxMemory = 10;
            int mask1 = asynInt8ArrayMask | asynInt16ArrayMask | asynInt32ArrayMask |
            asynFloat32ArrayMask | asynFloat64ArrayMask;
            int mask2 = asynInt8ArrayMask | asynInt16ArrayMask | asynInt32ArrayMask |
            asynFloat32ArrayMask | asynFloat64ArrayMask;
            int priority = 0;
            int stackSize = 5;
            plugin = new NDPluginDriverStandIn(portName.c_str(), queueSize, blockingCallbacks, NDArrayPort.c_str(), NDArrayAddr, 1,
                                               numberOfParams, 2, maxMemory, mask1, mask2, 0, 1, priority, stackSize);
        };
        
        static void TearDownTestCase() {
            delete plugin;
            plugin = nullptr;
        };
        
        virtual void SetUp() {
            
        };
        
        virtual void TearDown() {
            
        };
        
        static NDPluginDriverStandIn *plugin;
    };
    
    NDPluginDriverStandIn *KafkaProducerEnv::plugin = nullptr;
    
    using namespace testing;
    using ::testing::Mock;
    
    TEST_F(KafkaProducerEnv, InitTest) {
        KafkaProducer prod;
        ASSERT_TRUE(prod.SetMaxMessageSize(10));
        ASSERT_FALSE(prod.SetMaxMessageSize(0));
        ASSERT_TRUE(prod.SetStatsTime(10));
        ASSERT_FALSE(prod.SetStatsTime(0));
        unsigned char tempStr[] = "some";
        ASSERT_FALSE(prod.SendKafkaPacket(tempStr, 4));
        ASSERT_FALSE(prod.SendKafkaPacket(tempStr, 0));
    }
    
    TEST_F(KafkaProducerEnv, SetTopicAndConnectionTest1) {
        KafkaProducerStandIn prod;
        EXPECT_CALL(prod, MakeConnection()).Times(AtLeast(1));
        ASSERT_TRUE(prod.SetTopic("any_name"));
        ASSERT_TRUE(prod.SetBrokerAddr("any_name"));
    }
    
    TEST_F(KafkaProducerEnv, InitWithAddrAndTopic) {
        KafkaProducer prod("some_addr", "some_topic");
        ASSERT_TRUE(prod.SetMaxMessageSize(10));
        ASSERT_FALSE(prod.SetMaxMessageSize(0));
        ASSERT_TRUE(prod.SetStatsTime(10));
        ASSERT_FALSE(prod.SetStatsTime(0));
        unsigned char tempStr[] = "some";
        ASSERT_TRUE(prod.SendKafkaPacket(tempStr, 4));
        ASSERT_FALSE(prod.SendKafkaPacket(tempStr, 0));
    }
    
    TEST_F(KafkaProducerEnv, ErrorStateTest) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        prod.errorState = true;
        ASSERT_FALSE(prod.SetTopic("any_name"));
        ASSERT_FALSE(prod.SetTopic(""));
        ASSERT_FALSE(prod.SetBrokerAddr("any_name"));
        ASSERT_FALSE(prod.SetBrokerAddr(""));
        ASSERT_FALSE(prod.SetMaxMessageSize(10));
        ASSERT_FALSE(prod.SetMaxMessageSize(0));
        ASSERT_FALSE(prod.SetStatsTime(10));
        ASSERT_FALSE(prod.SetStatsTime(0));
        unsigned char tempStr[] = "some";
        ASSERT_FALSE(prod.SendKafkaPacket(tempStr, 4));
        ASSERT_FALSE(prod.SendKafkaPacket(tempStr, 0));
    }
    
    TEST_F(KafkaProducerEnv, StartThreadSuccessMessage) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        EXPECT_CALL(prod, SetConStat(Ne(KafkaProducerStandIn::ConStat::ERROR), _)).Times(Exactly(1));
        ASSERT_TRUE(prod.StartThread());
    }
    
    TEST_F(KafkaProducerEnv, StartThreadFailureMessage) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        prod.errorState = true;
        EXPECT_CALL(prod, SetConStat(Eq(KafkaProducerStandIn::ConStat::ERROR), _)).Times(Exactly(1));
        prod.StartThread();
        
    }
    
    TEST_F(KafkaProducerEnv, ParamCallFailure) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        prod.errorState = true;
        ON_CALL(prod, SetConStat(_, _)).WillByDefault(Invoke(&prod, &KafkaProducerStandIn::SetConStatParent));
        prod.RegisterParamCallbackClass(plugin);
        EXPECT_CALL(*plugin, setIntegerParam(_,_)).Times(Exactly(0));
        EXPECT_CALL(*plugin, setStringParam(_,_)).Times(Exactly(0));
        prod.StartThread();
        Mock::VerifyAndClear(plugin);
    }
    
    TEST_F(KafkaProducerEnv, ParamCallSuccess) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        prod.errorState = true;
        std::vector<PV_param> params = prod.GetParams();
        int ctr = 1;
        for (auto p : params) {
            *p.index = ctr;
            ctr++;
        }
        ON_CALL(prod, SetConStat(_, _)).WillByDefault(Invoke(&prod, &KafkaProducerStandIn::SetConStatParent));
        prod.RegisterParamCallbackClass(plugin);
        EXPECT_CALL(*plugin, setIntegerParam(_,_)).Times(AtLeast(1));
        EXPECT_CALL(*plugin, setStringParam(_,_)).Times(AtLeast(1));
        prod.StartThread();
        Mock::VerifyAndClear(plugin);
    }
    
    TEST_F(KafkaProducerEnv, MaxMessagesInQueue) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        ON_CALL(prod, MakeConnection()).WillByDefault(Invoke(&prod, &KafkaProducerStandIn::MakeConnectionParent));
        std::vector<PV_param> params = prod.GetParams();
        int ctr = 1;
        for (auto const &p : params) {
            *p.index.get() = ctr;
            ctr++;
        }
        int sendMsgs = 10;
        int msg_queued = *params[KafkaProducerStandIn::PV::msgs_in_queue].index.get();
        ON_CALL(prod, SetConStat(_, _)).WillByDefault(Invoke(&prod, &KafkaProducerStandIn::SetConStatParent));
        prod.RegisterParamCallbackClass(plugin);
        prod.SetMessageQueueLength(sendMsgs);
        EXPECT_CALL(*plugin, setIntegerParam(_,_)).Times(AtLeast(0));
        EXPECT_CALL(*plugin, setIntegerParam(Eq(msg_queued),Eq(sendMsgs))).Times(AtLeast(1));
        prod.StartThread();
        std::string msg("Some message");
        for (int i = 0; i < sendMsgs; i++) {
            ASSERT_TRUE(prod.SendKafkaPacket((unsigned char*)msg.c_str(), msg.size()));
        }
        
        std::chrono::milliseconds sleepTime(int(prod.kafka_stats_interval * 1.5));
        std::this_thread::sleep_for(sleepTime);
        Mock::VerifyAndClear(plugin);
    }
    
    TEST_F(KafkaProducerEnv, TooManyMessagesInQueue) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        ON_CALL(prod, MakeConnection()).WillByDefault(Invoke(&prod, &KafkaProducerStandIn::MakeConnectionParent));
        std::vector<PV_param> params = prod.GetParams();
        int ctr = 1;
        for (auto const &p : params) {
            *p.index.get() = ctr;
            ctr++;
        }
        int sendMsgs = 11;
        int msg_queued = *params[KafkaProducerStandIn::PV::msgs_in_queue].index.get();
        ON_CALL(prod, SetConStat(_, _)).WillByDefault(Invoke(&prod, &KafkaProducerStandIn::SetConStatParent));
        prod.RegisterParamCallbackClass(plugin);
        prod.SetMessageQueueLength(sendMsgs);
        EXPECT_CALL(*plugin, setIntegerParam(_,_)).Times(AtLeast(0));
        EXPECT_CALL(*plugin, setIntegerParam(Eq(msg_queued),Eq(sendMsgs))).Times(AtLeast(1));
        prod.StartThread();
        std::string msg("Some message");
        for (int i = 0; i < sendMsgs; i++) {
            ASSERT_TRUE(prod.SendKafkaPacket((unsigned char*)msg.c_str(), msg.size()));
        }
        ASSERT_FALSE(prod.SendKafkaPacket((unsigned char*)msg.c_str(), msg.size()));
        std::chrono::milliseconds sleepTime(int(prod.kafka_stats_interval * 1.5));
        std::this_thread::sleep_for(sleepTime);
        Mock::VerifyAndClear(plugin);
    }
    
    TEST_F(KafkaProducerEnv, TopicChangeReconnect) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        EXPECT_CALL(prod, MakeConnection()).Times(Exactly(0));
        prod.SetTopic("New topic");
    }
    
    TEST_F(KafkaProducerEnv, AddressChangeReconnect) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        EXPECT_CALL(prod, MakeConnection()).Times(Exactly(1));
        prod.SetBrokerAddr("new_addr");
    }
    
    TEST_F(KafkaProducerEnv, MessageSizeChangeReconnect) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        EXPECT_CALL(prod, MakeConnection()).Times(Exactly(1));
        prod.SetMaxMessageSize(1000);
    }
    
    TEST_F(KafkaProducerEnv, MessageQueueLengthChangeReconnect) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        EXPECT_CALL(prod, MakeConnection()).Times(Exactly(1));
        prod.SetMessageQueueLength(15);
    }
    
    TEST_F(KafkaProducerEnv, SetStatsTimeReconnect) {
        KafkaProducerStandIn prod("some_addr", "some_topic");
        EXPECT_CALL(prod, MakeConnection()).Times(Exactly(1));
        prod.SetStatsTime(100);
    }
}
