/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaPluginTest.cpp
 *  @brief Some limited tests of actual plugin class.
 */

#include "GenerateNDArray.h"
#include "KafkaPlugin.h"
#include "PortName.h"
#include <chrono>
#include <ciso646>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

using ::testing::Test;
using ::testing::_;
using ::testing::Exactly;
using ::testing::Mock;
using ::testing::Eq;
using ::testing::AtLeast;

const std::string usedBrokerAddr = "some_broker";
const std::string usedTopic = "some_topic";

/// @brief Simple stand-in class used for unit tests.
class KafkaPluginStandIn : public KafkaPlugin {
public:
  KafkaPluginStandIn()
      : KafkaPlugin(PortName().c_str(), 10, 1, "some_arr_port", 1, 0, 1, 1,
                    usedBrokerAddr.c_str(), usedTopic.c_str()){};
  using KafkaPlugin::producer;
  using KafkaPlugin::paramsList;
  using KafkaPlugin::PV;
  using asynPortDriver::pasynUserSelf;
  MOCK_METHOD2(setStringParam, asynStatus(int, const char *));
  MOCK_METHOD2(setIntegerParam, asynStatus(int, int));
};

/// @brief A testing fixture used for setting up unit tests.
class KafkaPluginEnv : public Test {
public:
  static void SetUpTestCase(){

  };

  static void TearDownTestCase(){

  };

  virtual void SetUp(){};

  virtual void TearDown(){

  };
};

TEST_F(KafkaPluginEnv, InitParamsIndexTest) {
  KafkaPluginStandIn plugin;
  for (auto &p : plugin.paramsList) {
    ASSERT_NE(*p.index, 0);
  }

  for (auto &p : plugin.producer.GetParams()) {
    ASSERT_NE(*p.index, 0);
  }
}

TEST_F(KafkaPluginEnv, ParameterCountTest) {
  KafkaPluginStandIn plug;
  ASSERT_EQ(plug.paramsList.size(), KafkaPluginStandIn::PV::count);
}

TEST_F(KafkaPluginEnv, InitIsErrorStateTest) {
  KafkaPluginStandIn plugin;
  ASSERT_TRUE(plugin.producer.SetStatsTimeMS(10000));
}

TEST_F(KafkaPluginEnv, ParamCallbackIsSetTest) {
  KafkaPluginStandIn plugin;
  int usedValue = 5000;
  EXPECT_CALL(plugin, setIntegerParam(_, Eq(usedValue))).Times(Exactly(1));
  ASSERT_TRUE(plugin.producer.SetMaxMessageSize(usedValue));
}

TEST_F(KafkaPluginEnv, ProducerThreadIsRunningTest) {
  std::chrono::milliseconds sleepTime(1000);
  KafkaPluginStandIn plugin;
  EXPECT_CALL(plugin, setIntegerParam(_, _)).Times(AtLeast(1));
  EXPECT_CALL(plugin, setIntegerParam(_, Eq(0))).Times(AtLeast(1));
  std::this_thread::sleep_for(sleepTime);
}

TEST_F(KafkaPluginEnv, InitBrokerStringsTest) {
  KafkaPluginStandIn plugin;
  ASSERT_EQ(usedBrokerAddr, plugin.producer.GetBrokerAddr());
  ASSERT_EQ(usedTopic, plugin.producer.GetTopic());

  const int bufferSize = 50;
  char buffer[bufferSize];
  plugin.getStringParam(
      *plugin.paramsList[KafkaPluginStandIn::PV::kafka_addr].index, bufferSize,
      buffer);
  ASSERT_EQ(std::string(buffer), usedBrokerAddr);

  plugin.getStringParam(
      *plugin.paramsList[KafkaPluginStandIn::PV::kafka_topic].index, bufferSize,
      buffer);
  ASSERT_EQ(std::string(buffer), usedTopic);
}

TEST_F(KafkaPluginEnv, ProcessCallbacksCallTest) {
  NDArrayGenerator arrGen;
  NDArray *arr = arrGen.GenerateNDArray(5, 10, 3, NDDataType_t::NDUInt8);
  KafkaPluginStandIn plugin;
  auto tempUser = pasynManager->createAsynUser(nullptr, nullptr);
  plugin.driverCallback(tempUser, reinterpret_cast<void *>(arr));
  int queueIndex = -1;
  for (auto &p : plugin.producer.GetParams()) {
    if ("KAFKA_UNSENT_PACKETS" == p.desc) {
      queueIndex = *p.index;
    }
  }
  ASSERT_NE(queueIndex, -1);
  EXPECT_CALL(plugin, setIntegerParam(_, _)).Times(AtLeast(1));
  EXPECT_CALL(plugin, setIntegerParam(Eq(queueIndex), Eq(1))).Times(AtLeast(1));
  std::chrono::milliseconds sleepTime(1000);
  std::this_thread::sleep_for(sleepTime);
}

TEST_F(KafkaPluginEnv, KafkaQueueFullTest) {
  std::chrono::milliseconds sleepTime(1000);
  KafkaPluginStandIn plugin;
  int kafkaMaxQueueSize = 5;
  plugin.producer.SetMessageQueueLength(kafkaMaxQueueSize);
  NDArrayGenerator arrGen;
  for (int i = 0; i < kafkaMaxQueueSize; i++) {
    NDArray *ptr = arrGen.GenerateNDArray(5, 10, 3, NDDataType_t::NDUInt8);
    auto tempUser = pasynManager->createAsynUser(nullptr, nullptr);
    plugin.driverCallback(tempUser, reinterpret_cast<void *>(ptr));
    ptr->release();
  }
  int queueIndex = -1;
  for (auto &p : plugin.producer.GetParams()) {
    if ("KAFKA_UNSENT_PACKETS" == p.desc) {
      queueIndex = *p.index;
    }
  }
  ASSERT_NE(queueIndex, -1);
  EXPECT_CALL(plugin, setIntegerParam(_, _)).Times(AtLeast(1));
  EXPECT_CALL(plugin, setIntegerParam(Eq(queueIndex), Eq(kafkaMaxQueueSize)))
      .Times(AtLeast(1));
  std::this_thread::sleep_for(sleepTime);
  testing::Mock::VerifyAndClear(&plugin);
  NDArray *ptr = arrGen.GenerateNDArray(5, 10, 3, NDDataType_t::NDUInt8);
  auto tempUser = pasynManager->createAsynUser(nullptr, nullptr);
  plugin.driverCallback(tempUser, reinterpret_cast<void *>(ptr));
  ptr->release();

  EXPECT_CALL(plugin, setIntegerParam(testing::Ne(queueIndex), _))
      .Times(AtLeast(1));
  EXPECT_CALL(plugin,
              setIntegerParam(Eq(queueIndex), testing::Ne(kafkaMaxQueueSize)))
      .Times(testing::Exactly(0));
  EXPECT_CALL(plugin, setIntegerParam(Eq(queueIndex), Eq(kafkaMaxQueueSize)))
      .Times(AtLeast(1));
  std::this_thread::sleep_for(sleepTime);
}
