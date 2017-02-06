//
//  KafkaDriverTest.cpp
//  KafkaInterface
//
//  Created by Jonas Nilsson on 2017-02-03.
//
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
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
    using KafkaDriver::startEventId_;
    using KafkaDriver::stopEventId_;
    using asynPortDriver::pasynUserSelf;
    using ADDriver::ADStatusMessage;
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

TEST_F(KafkaDriverEnv, ParamCallbackIsSetTest) {
    KafkaDriverStandIn drvr;
    int usedValue = 5000;
    EXPECT_CALL(drvr, setIntegerParam(_, Eq(usedValue))).Times(Exactly(1));
    ASSERT_TRUE(drvr.consumer.SetOffset(usedValue));
}

TEST_F(KafkaDriverEnv, InitBrokerStringsTest) {
    KafkaDriverStandIn drvr;
    ASSERT_EQ(usedBrokerAddr, drvr.consumer.GetBrokerAddr());
    ASSERT_EQ(usedTopic, drvr.consumer.GetTopic());
    
    const int bufferSize = 50;
    char buffer[bufferSize];
    drvr.getStringParam(*drvr.paramsList[KafkaDriverStandIn::PV::kafka_addr].index, bufferSize, buffer);
    ASSERT_EQ(std::string(buffer), usedBrokerAddr);
    
    drvr.getStringParam(*drvr.paramsList[KafkaDriverStandIn::PV::kafka_topic].index, bufferSize, buffer);
    ASSERT_EQ(std::string(buffer), usedTopic);
}

TEST_F(KafkaDriverEnv, ThreadRunningTest) {
    KafkaDriverStandIn drvr;
    EXPECT_CALL(drvr, setStringParam(Eq(drvr.ADStatusMessage), _)).Times(Exactly(1));
    epicsEventSignal(drvr.startEventId_);
    epicsEventSignal(drvr.stopEventId_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
