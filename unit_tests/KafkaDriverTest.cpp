//
//  KafkaDriverTest.cpp
//  KafkaInterface
//
//  Created by Jonas Nilsson on 2017-02-03.
//
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "KafkaDriver.h"
#include "PortName.h"

using ::testing::Test;
using ::testing::_;
using ::testing::Exactly;
using ::testing::Mock;
using ::testing::Eq;
using ::testing::AtLeast;

const std::string usedBrokerAddr = "some_broker";
const std::string usedTopic = "some_topic";

class KafkaDriverStandIn : public KafkaDriver {
public:
    KafkaDriverStandIn() : KafkaDriver(PortName().c_str(), 100, 100, NDDataType_t::NDUInt32, 10, 0, 0, 0, usedBrokerAddr.c_str(), usedTopic.c_str()) {};
    using KafkaDriver::consumer;
    using KafkaDriver::paramsList;
    using KafkaDriver::PV;
    using asynPortDriver::pasynUserSelf;
    MOCK_METHOD2(setStringParam, asynStatus(int, const char*));
    MOCK_METHOD2(setIntegerParam, asynStatus(int, int));
};

class KafkaDriverEnv : public Test {
public:
    static void SetUpTestCase() {
        
    };
    
    static void TearDownTestCase() {
        
    };
    
    virtual void SetUp() {
    };
    
    virtual void TearDown() {
        
    };
};

TEST_F(KafkaDriverEnv, InitParamsIndexTest) {
    KafkaDriverStandIn drvr;
    for (auto &p : drvr.paramsList) {
        ASSERT_NE(*p.index, 0);
    }
    
    for (auto &p : drvr.consumer.GetParams()) {
        ASSERT_NE(*p.index, 0);
    }
}

TEST_F(KafkaDriverEnv, InitIsErrorStateTest) {
    KafkaDriverStandIn drvr;
    ASSERT_TRUE(drvr.consumer.SetStatsTimeMS(10000));
}
//
//TEST_F(KafkaPluginEnv, ParamCallbackIsSetTest) {
//    KafkaPluginStandIn plugin("port_nr_21o");
//    int usedValue = 5000;
//    EXPECT_CALL(plugin, setIntegerParam(_, Eq(usedValue))).Times(Exactly(1));
//    ASSERT_TRUE(plugin.prod.SetMaxMessageSize(usedValue));
//}
//
//TEST_F(KafkaPluginEnv, ProducerThreadIsRunningTest) {
//    std::chrono::milliseconds sleepTime(1000);
//    KafkaPluginStandIn plugin("port_nr_21");
//    EXPECT_CALL(plugin, setIntegerParam(_, _)).Times(AtLeast(1));
//    EXPECT_CALL(plugin, setIntegerParam(_, Eq(0))).Times(AtLeast(1));
//    std::this_thread::sleep_for(sleepTime);
//}
//
//TEST_F(KafkaPluginEnv, InitBrokerStringsTest) {
//    KafkaPluginStandIn plugin("port_nr_2tr");
//    ASSERT_EQ(usedBrokerAddr, plugin.prod.GetBrokerAddr());
//    ASSERT_EQ(usedTopic, plugin.prod.GetTopic());
//    
//    const int bufferSize = 50;
//    char buffer[bufferSize];
//    plugin.getStringParam(*plugin.paramsList[KafkaPluginStandIn::PV::kafka_addr].index, bufferSize, buffer);
//    ASSERT_EQ(std::string(buffer), usedBrokerAddr);
//    
//    plugin.getStringParam(*plugin.paramsList[KafkaPluginStandIn::PV::kafka_topic].index, bufferSize, buffer);
//    ASSERT_EQ(std::string(buffer), usedTopic);
//}
//
//TEST_F(KafkaPluginEnv, ProcessCallbacksCallTest) {
//    NDArrayGenerator arrGen;
//    NDArray *arr = arrGen.GenerateNDArray(5, 10, 3, NDDataType_t::NDUInt8);
//    KafkaPluginStandIn plugin("port_nr_511");
//    plugin.driverCallback(nullptr, (void*)arr);
//    int queueIndex = -1;
//    for (auto &p : plugin.prod.GetParams()) {
//        if ("KAFKA_UNSENT_PACKETS" == p.desc) {
//            queueIndex = *p.index;
//        }
//    }
//    ASSERT_NE(queueIndex, -1);
//    EXPECT_CALL(plugin, setIntegerParam(_, _)).Times(AtLeast(1));
//    EXPECT_CALL(plugin, setIntegerParam(Eq(queueIndex), Eq(1))).Times(AtLeast(1));
//    std::chrono::milliseconds sleepTime(1000);
//    std::this_thread::sleep_for(sleepTime);
//}
//
//TEST_F(KafkaPluginEnv, KafkaQueueFullTest) {
//    std::chrono::milliseconds sleepTime(1000);
//    KafkaPluginStandIn plugin("port_nr_512");
//    int kafkaMaxQueueSize = 5;
//    plugin.prod.SetMessageQueueLength(kafkaMaxQueueSize);
//    NDArrayGenerator arrGen;
//    for (int i = 0; i < kafkaMaxQueueSize; i++) {
//        NDArray *ptr = arrGen.GenerateNDArray(5, 10, 3, NDDataType_t::NDUInt8);
//        plugin.driverCallback(nullptr, (void*)ptr);
//        ptr->release();
//    }
//    int queueIndex = -1;
//    for (auto &p : plugin.prod.GetParams()) {
//        if ("KAFKA_UNSENT_PACKETS" == p.desc) {
//            queueIndex = *p.index;
//        }
//    }
//    ASSERT_NE(queueIndex, -1);
//    EXPECT_CALL(plugin, setIntegerParam(_, _)).Times(AtLeast(1));
//    EXPECT_CALL(plugin, setIntegerParam(Eq(queueIndex), Eq(kafkaMaxQueueSize))).Times(AtLeast(1));
//    std::this_thread::sleep_for(sleepTime);
//    testing::Mock::VerifyAndClear(&plugin);
//    NDArray *ptr = arrGen.GenerateNDArray(5, 10, 3, NDDataType_t::NDUInt8);
//    plugin.driverCallback(nullptr, (void*)ptr);
//    ptr->release();
//    
//    EXPECT_CALL(plugin, setIntegerParam(testing::Ne(queueIndex), _)).Times(AtLeast(1));
//    EXPECT_CALL(plugin, setIntegerParam(Eq(queueIndex), testing::Ne(kafkaMaxQueueSize))).Times(testing::Exactly(0));
//    EXPECT_CALL(plugin, setIntegerParam(Eq(queueIndex), Eq(kafkaMaxQueueSize))).Times(AtLeast(1));
//    std::this_thread::sleep_for(sleepTime);
//}
//
//class KafkaPluginInput : public Test {
//public:
//    static void SetUpTestCase() {
//        plugin = new KafkaPluginStandIn("2324");
//    };
//    
//    static void TearDownTestCase() {
//        delete plugin;
//    };
//    
//    virtual void SetUp() {
//    };
//    
//    virtual void TearDown() {
//        
//    };
//    static int GetIndex(std::string name) {
//        int retIndex = -1;
//        for (auto &p : plugin->paramsList) {
//            if (name == p.desc) {
//                retIndex = *p.index;
//                break;
//            }
//        }
//        
//        return retIndex;
//    }
//    static KafkaPluginStandIn *plugin;
//};
//
//KafkaPluginStandIn *KafkaPluginInput::plugin = nullptr;

//TEST_F(KafkaPluginInput, BrokerAddrTest) {
//    std::string testString = "some_test_broker";
//    asynUser usedUser = *plugin->pasynUserSelf;
//    usedUser.reason = GetIndex("KAFKA_BROKER_ADDRESS");
//    size_t nChars;
//    plugin->writeOctet(&usedUser, testString.c_str(), testString.size(), &nChars);
//    ASSERT_EQ(nChars, testString.size());
//    ASSERT_EQ(testString, plugin->prod.GetBrokerAddr());
//}
//
//TEST_F(KafkaPluginInput, BrokerTopicTest) {
//    std::string testString = "some_test_topic";
//    asynUser usedUser = *plugin->pasynUserSelf;
//    usedUser.reason = GetIndex("KAFKA_TOPIC");
//    size_t nChars;
//    plugin->writeOctet(&usedUser, testString.c_str(), testString.size(), &nChars);
//    ASSERT_EQ(nChars, testString.size());
//    ASSERT_EQ(testString, plugin->prod.GetBrokerAddr());
//}
//
//TEST_F(KafkaPluginInput, StatsTimeTest) {
//    int testValue = 555;
//    asynUser usedUser = *plugin->pasynUserSelf;
//    usedUser.reason = GetIndex("KAFKA_STATS_INT_MS");
//    plugin->writeInt32(&usedUser, testValue);
//    ASSERT_EQ(testValue, plugin->prod.GetStatsTimeMS());
//}
//
//TEST_F(KafkaPluginInput, QueueSizeTest) {
//    int testValue = 22;
//    asynUser usedUser = *plugin->pasynUserSelf;
//    usedUser.reason = GetIndex("KAFKA_QUEUE_SIZE");
//    plugin->writeInt32(&usedUser, testValue);
//    ASSERT_EQ(testValue, plugin->prod.GetMessageQueueLength());
//}
