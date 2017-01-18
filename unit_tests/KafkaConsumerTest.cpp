//
//  KafkaConsumerTest.cpp
//  KafkaPlugin
//
//  Created by Jonas Nilsson on 2017-01-16.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include "KafkaConsumer.h"

class KafkaConsumerStandIn : public KafkaInterface::KafkaConsumer {
public:
    KafkaConsumerStandIn() : KafkaConsumer() {};
    KafkaConsumerStandIn(std::string addr, std::string topic): KafkaInterface::KafkaConsumer(addr, topic) {};
    using KafkaInterface::KafkaConsumer::errorState;
    using KafkaInterface::KafkaConsumer::ConStat;
    using KafkaInterface::KafkaConsumer::kafka_stats_interval;
    using KafkaInterface::KafkaConsumer::consumer;
    using KafkaInterface::KafkaConsumer::paramCallback;
    void SetConStatParent(KafkaConsumerStandIn::ConStat stat, std::string msg) {KafkaInterface::KafkaConsumer::SetConStat(stat, msg);};
    bool MakeConnectionParent() {return KafkaInterface::KafkaConsumer::MakeConnection();};
    MOCK_METHOD1(ParseStatusString, void(std::string));
    MOCK_METHOD0(MakeConnection, bool(void));
    MOCK_METHOD0(UpdateTopic, bool(void));
    MOCK_METHOD2(SetConStat, void(KafkaConsumerStandIn::ConStat, std::string));
};

class KafkaConsumerStandInAlt : public KafkaInterface::KafkaConsumer {
public:
    KafkaConsumerStandInAlt() : KafkaConsumer() {};
    KafkaConsumerStandInAlt(std::string addr, std::string topic): KafkaInterface::KafkaConsumer(addr, topic) {};
    using KafkaInterface::KafkaConsumer::errorState;
    using KafkaInterface::KafkaConsumer::ConStat;
    using KafkaInterface::KafkaConsumer::kafka_stats_interval;
    using KafkaInterface::KafkaConsumer::consumer;
};

class NDPluginDriverStandIn : public NDPluginDriver {
public:
    NDPluginDriverStandIn(const char *portName, int queueSize, int blockingCallbacks, const char *NDArrayPort, int NDArrayAddr, int maxAddr, int numParams, int maxBuffers, size_t maxMemory, int interfaceMask, int interruptMask, int asynFlags, int autoConnect, int priority, int stackSize) : NDPluginDriver(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr, maxAddr, numParams, maxBuffers, maxMemory, interfaceMask, interruptMask, asynFlags, autoConnect, priority, stackSize) {};
    MOCK_METHOD2(setStringParam, asynStatus(int, const char*));
    MOCK_METHOD2(setIntegerParam, asynStatus(int, int));
    MOCK_METHOD3(createParam, asynStatus(const char*, asynParamType, int*));
};

class KafkaConsumerEnv : public ::testing::Test {
public:
    static void SetUpTestCase() {
        std::string portName("someName");
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

NDPluginDriverStandIn *KafkaConsumerEnv::plugin = nullptr;

typedef std::chrono::milliseconds TimeT;
using namespace testing;
using ::testing::Mock;

namespace KafkaInterface {
    TEST_F(KafkaConsumerEnv, ConnectionSuccessTest) {
        KafkaConsumerStandInAlt cons("some_addr", "some_topic");
        ASSERT_NE(cons.consumer, nullptr);
    }
    
    TEST_F(KafkaConsumerEnv, ConnectionFailTest) {
        KafkaConsumerStandInAlt cons;
        ASSERT_EQ(cons.consumer, nullptr);
    }
    
    TEST_F(KafkaConsumerEnv, StatsTest) {
        KafkaConsumerStandIn cons("some_broker", "some_topic");
        EXPECT_CALL(cons, ParseStatusString(_)).Times(AtLeast(1));
        KafkaMessage *msg = cons.WaitForPkg(1000);
        delete msg;
    }
    
    TEST_F(KafkaConsumerEnv, NoWaitTest) {
        KafkaConsumer cons;
        auto start = std::chrono::steady_clock::now();
        KafkaMessage *msg = cons.WaitForPkg(1000);
        auto duration = std::chrono::duration_cast< TimeT>(std::chrono::steady_clock::now() - start);
        ASSERT_LT(duration.count(), 100);
        ASSERT_EQ(msg, nullptr);
        delete msg;
    }
    
    TEST_F(KafkaConsumerEnv, WaitTest) {
        KafkaConsumer cons("some_addr", "some_topic");
        auto start = std::chrono::steady_clock::now();
        KafkaMessage *msg = cons.WaitForPkg(1000);
        auto duration = std::chrono::duration_cast< TimeT>(std::chrono::steady_clock::now() - start);
        ASSERT_GE(duration.count(), 1000);
        ASSERT_EQ(msg, nullptr);
        delete msg;
    }
    
    TEST_F(KafkaConsumerEnv, StatsQueueTest) {
        KafkaConsumer cons("some_addr", "some_topic");
        auto params = cons.GetParams();
        cons.RegisterParamCallbackClass(plugin);
        int ctr = 1;
        for (auto p : params) {
            *p.second.index = ctr;
            ctr++;
        }
        int queueIndex = *params["queued"].index;
        EXPECT_CALL(*plugin, setIntegerParam(_,_)).Times(AtLeast(1));
        EXPECT_CALL(*plugin, setIntegerParam(Eq(queueIndex),_)).Times(AtLeast(1));
        KafkaMessage *msg = cons.WaitForPkg(1000);
        
        delete msg;
        Mock::VerifyAndClear(plugin);
    }
    
    TEST_F(KafkaConsumerEnv, StatsStatusTest) {
        KafkaConsumer cons("some_addr", "some_topic");
        auto params = cons.GetParams();
        cons.RegisterParamCallbackClass(plugin);
        int ctr = 1;
        for (auto p : params) {
            *p.second.index = ctr;
            ctr++;
        }
        int statusIndex = *params["status"].index;
        EXPECT_CALL(*plugin, setIntegerParam(_,_)).Times(AtLeast(1));
        EXPECT_CALL(*plugin, setIntegerParam(Eq(statusIndex),_)).Times(AtLeast(1));
        KafkaMessage *msg = cons.WaitForPkg(1000);
        
        delete msg;
        Mock::VerifyAndClear(plugin);
    }
    
    TEST_F(KafkaConsumerEnv, ErrorStateTest) {
        KafkaConsumerStandInAlt cons;
        cons.errorState = true;
        ASSERT_FALSE(cons.SetTopic("some_topic_2"));
        ASSERT_FALSE(cons.SetBrokerAddr("some_topic_2"));
        ASSERT_FALSE(cons.SetGroupId("some_topic_2"));
        ASSERT_FALSE(cons.SetStatsTime(100));
    }
    
    TEST_F(KafkaConsumerEnv, NoErrorStateTest) {
        KafkaConsumer cons("addr", "tpic");
        ASSERT_TRUE(cons.SetTopic("some_topic_2"));
        ASSERT_TRUE(cons.SetBrokerAddr("some_topic_2"));
        ASSERT_TRUE(cons.SetGroupId("some_topic_2"));
        ASSERT_TRUE(cons.SetStatsTime(100));
    }
    
    TEST_F(KafkaConsumerEnv, NegativeTimeTest) {
        KafkaConsumer cons("addr", "tpic");
        ASSERT_FALSE(cons.SetStatsTime(-1));
    }
    
    TEST_F(KafkaConsumerEnv, ZeroTimeTest) {
        KafkaConsumer cons("addr", "tpic");
        ASSERT_FALSE(cons.SetStatsTime(0));
    }
    
    TEST_F(KafkaConsumerEnv, ZeroLengthGroupIdTest) {
        KafkaConsumer cons("addr", "tpic");
        ASSERT_FALSE(cons.SetGroupId(""));
    }
    
    TEST_F(KafkaConsumerEnv, ZeroLengthBrokerTest) {
        KafkaConsumer cons("addr", "tpic");
        ASSERT_FALSE(cons.SetBrokerAddr(""));
    }
    
    TEST_F(KafkaConsumerEnv, ZeroLengthTopicTest) {
        KafkaConsumer cons("addr", "tpic");
        ASSERT_FALSE(cons.SetTopic(""));
    }
    
    TEST_F(KafkaConsumerEnv, RegCallbackTest) {
        KafkaConsumerStandIn cons;
        cons.RegisterParamCallbackClass(plugin);
        ASSERT_EQ(plugin, cons.paramCallback);
    }
    
    TEST_F(KafkaConsumerEnv, SetTopicTest) {
        KafkaConsumerStandIn cons("addr", "tpic");
        EXPECT_CALL(cons, UpdateTopic()).Times(Exactly(1));
        cons.SetTopic("new_topic");
    }
    
    TEST_F(KafkaConsumerEnv, SetBrokerTest) {
        KafkaConsumerStandIn cons("addr", "tpic");
        EXPECT_CALL(cons, MakeConnection()).Times(Exactly(1));
        cons.SetBrokerAddr("new_broker");
    }
    
    TEST_F(KafkaConsumerEnv, SetOffsetTest) {
        KafkaConsumerStandIn cons("addr", "tpic");
        EXPECT_CALL(cons, UpdateTopic()).Times(Exactly(1));
        cons.SetOffset(0);
    }
    
    TEST_F(KafkaConsumerEnv, SetGroupIdTest) {
        KafkaConsumerStandIn cons("addr", "tpic");
        EXPECT_CALL(cons, MakeConnection()).Times(Exactly(1));
        cons.SetGroupId("some_group");
    }
    
    TEST_F(KafkaConsumerEnv, SetStatsTimeTest) {
        KafkaConsumerStandIn cons("addr", "tpic");
        EXPECT_CALL(cons, MakeConnection()).Times(Exactly(1));
        cons.SetStatsTime(100);
    }
    
    TEST_F(KafkaConsumerEnv, SetConStatTest) {
        KafkaConsumerStandIn cons("addr", "tpic");
        auto params = cons.GetParams();
        int ctr = 1;
        for (auto p : params) {
            *p.second.index = ctr;
            ctr++;
        }
        int messageIndex = *params["message"].index;
        int statusIndex = *params["status"].index;
        cons.RegisterParamCallbackClass(plugin);
        EXPECT_CALL(*plugin, setIntegerParam(Eq(statusIndex), Eq(int(KafkaConsumerStandIn::ConStat::ERROR)))).Times(Exactly(1));
        EXPECT_CALL(*plugin, setStringParam(Eq(messageIndex), _)).Times(Exactly(1));
        cons.SetConStatParent(KafkaConsumerStandIn::ConStat::ERROR, "some message");
        Mock::VerifyAndClear(plugin);
    }
}
