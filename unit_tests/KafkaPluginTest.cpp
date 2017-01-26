//
//  KafkaPluginTest.cpp
//  KafkaInterface
//
//  Created by Jonas Nilsson on 2017-01-25.
//
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <string>
#include <tuple>
#include <thread>
#include <chrono>
#include "KafkaPlugin.h"

using ::testing::Test;
using ::testing::_;
using ::testing::Exactly;
using ::testing::Mock;
using ::testing::Eq;
using ::testing::AtLeast;

std::string usedBrokerAddr = "some_broker";
std::string usedTopic = "some_topic";

class KafkaPluginStandIn : public KafkaPlugin {
public:
    KafkaPluginStandIn(std::string portName) : KafkaPlugin(portName.c_str(), 10, 1, "some_arr_port", 1, 0, 1, 1, usedBrokerAddr.c_str(), usedTopic.c_str()) {};
    using KafkaPlugin::prod;
    using KafkaPlugin::paramsList;
    using KafkaPlugin::PV;
    MOCK_METHOD2(setStringParam, asynStatus(int, const char*));
    MOCK_METHOD2(setIntegerParam, asynStatus(int, int));
};

class KafkaPluginEnv : public Test {
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

TEST_F(KafkaPluginEnv, InitParamsIndexTest) {
    KafkaPluginStandIn plugin("port_nr_2144");
    for (auto &p : plugin.paramsList) {
        ASSERT_NE(*p.index, 0);
    }
    
    for (auto &p : plugin.prod.GetParams()) {
        ASSERT_NE(*p.index, 0);
    }
}

TEST_F(KafkaPluginEnv, InitIsErrorStateTest) {
    KafkaPluginStandIn plugin("port_nr_222");
    ASSERT_TRUE(plugin.prod.SetStatsTimeMS(10000));
}

TEST_F(KafkaPluginEnv, ParamCallbackIsSetTest) {
    KafkaPluginStandIn plugin("port_nr_21o");
    int usedValue = 5000;
    EXPECT_CALL(plugin, setIntegerParam(_, Eq(usedValue))).Times(Exactly(1));
    ASSERT_TRUE(plugin.prod.SetMaxMessageSize(usedValue));
}

TEST_F(KafkaPluginEnv, ProducerThreadIsRunningTest) {
    std::chrono::milliseconds sleepTime(1000);
    KafkaPluginStandIn plugin("port_nr_21");
    EXPECT_CALL(plugin, setIntegerParam(_, _)).Times(AtLeast(1));
    EXPECT_CALL(plugin, setIntegerParam(_, Eq(0))).Times(AtLeast(1));
    std::this_thread::sleep_for(sleepTime);
}

TEST_F(KafkaPluginEnv, InitBrokerStringsTest) {
    KafkaPluginStandIn plugin("port_nr_2tr");
    ASSERT_EQ(usedBrokerAddr, plugin.prod.GetBrokerAddr());
    ASSERT_EQ(usedTopic, plugin.prod.GetTopic());
    
    const int bufferSize = 50;
    char buffer[bufferSize];
    plugin.getStringParam(*plugin.paramsList[KafkaPluginStandIn::PV::kafka_addr].index, bufferSize, buffer);
    ASSERT_EQ(std::string(buffer), usedBrokerAddr);
    
    plugin.getStringParam(*plugin.paramsList[KafkaPluginStandIn::PV::kafka_topic].index, bufferSize, buffer);
    ASSERT_EQ(std::string(buffer), usedTopic);
}

TEST_F(KafkaPluginEnv, ProcessCallbacksCallTest) {
    
}


