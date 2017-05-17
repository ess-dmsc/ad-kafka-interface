/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaDriverTest.cpp
 *  @brief This file holds KafkaDriver unit tests..
 */

#include "KafkaDriver.h"
#include "PortName.h"
#include <chrono>
#include <ciso646>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

using ::testing::Test;
using ::testing::_;
using ::testing::Exactly;
using ::testing::Mock;
using ::testing::Eq;
using ::testing::AtLeast;
using ::testing::StrEq;
using ::testing::Ne;
using ::testing::NiceMock;

const std::string usedBrokerAddr = "some_broker";
const std::string usedTopic = "some_topic";

/// @brief Simple stand-in class used for unit tests.
class KafkaDriverStandIn : public KafkaDriver {
public:
  KafkaDriverStandIn()
      : KafkaDriver(PortName().c_str(), 10, 0, 0, 0, usedBrokerAddr.c_str(),
                    usedTopic.c_str()){};
  using KafkaDriver::consumer;
  using KafkaDriver::paramsList;
  using KafkaDriver::PV;
  using KafkaDriver::startEventId_;
  using KafkaDriver::stopEventId_;
  using asynPortDriver::pasynUserSelf;
  using ADDriver::ADStatusMessage;
  MOCK_METHOD2(setStringParam, asynStatus(int, const char *));
  MOCK_METHOD2(setIntegerParam, asynStatus(int, int));
};

/// @brief A testing fixture used for setting up unit tests.
class KafkaDriverEnv : public Test {
public:
  static void SetUpTestCase(){

  };

  static void TearDownTestCase(){

  };

  virtual void SetUp(){};

  virtual void TearDown(){

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
  // Ugly hack to make sure that the thread actually starts
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

TEST_F(KafkaDriverEnv, InitIsErrorStateTest) {
  KafkaDriverStandIn drvr;
  ASSERT_TRUE(drvr.consumer.SetStatsTimeIntervalMS(10000));
  // Ugly hack to make sure that the thread actually starts
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

TEST_F(KafkaDriverEnv, ParameterCountTest) {
  KafkaDriverStandIn drvr;
  ASSERT_EQ(drvr.paramsList.size(), KafkaDriverStandIn::PV::count);
  // Ugly hack to make sure that the thread actually starts
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

TEST_F(KafkaDriverEnv, ParamCallbackIsSetTest) {
  KafkaDriverStandIn drvr;
  int usedValue = 5000;
  EXPECT_CALL(drvr, setIntegerParam(_, Eq(usedValue))).Times(Exactly(1));
  ASSERT_TRUE(drvr.consumer.SetOffset(usedValue));
  // Ugly hack to make sure that the thread actually starts
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

TEST_F(KafkaDriverEnv, InitBrokerStringsTest) {
  KafkaDriverStandIn drvr;
  ASSERT_EQ(usedBrokerAddr, drvr.consumer.GetBrokerAddr());
  ASSERT_EQ(usedTopic, drvr.consumer.GetTopic());

  const int bufferSize = 50;
  char buffer[bufferSize];
  drvr.getStringParam(
      *drvr.paramsList[KafkaDriverStandIn::PV::kafka_addr].index, bufferSize,
      buffer);
  ASSERT_EQ(std::string(buffer), usedBrokerAddr);

  drvr.getStringParam(
      *drvr.paramsList[KafkaDriverStandIn::PV::kafka_topic].index, bufferSize,
      buffer);
  ASSERT_EQ(std::string(buffer), usedTopic);

  drvr.getStringParam(
      *drvr.paramsList[KafkaDriverStandIn::PV::kafka_group].index, bufferSize,
      buffer);
  ASSERT_EQ(std::string(buffer), drvr.consumer.GetGroupId());
  // Ugly hack to make sure that the thread actually starts
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

TEST_F(KafkaDriverEnv, InitStatsTimeTest) {
  KafkaDriverStandIn drvr;
  int temp;
  drvr.getIntegerParam(
      *drvr.paramsList[KafkaDriverStandIn::PV::stats_time].index, &temp);
  ASSERT_EQ(drvr.consumer.GetStatsTimeMS(), temp);
  // Ugly hack to make sure that the thread actually starts
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

TEST_F(KafkaDriverEnv, ThreadRunningTest) {
  NiceMock<KafkaDriverStandIn> drvr;
  EXPECT_CALL(drvr, setStringParam(Ne(drvr.ADStatusMessage), _))
      .Times(AtLeast(0));
  EXPECT_CALL(drvr, setStringParam(Eq(drvr.ADStatusMessage), _))
      .Times(Exactly(1));
  epicsEventSignal(drvr.startEventId_);
  epicsEventSignal(drvr.stopEventId_);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST_F(KafkaDriverEnv, ConnectionStatusUpdateTest) {
  NiceMock<KafkaDriverStandIn> drvr;
  int msgIndex = -1;
  for (auto p : drvr.consumer.GetParams()) {
    if ("KAFKA_CONNECTION_MESSAGE" == p.desc) {
      msgIndex = *p.index;
      break;
    }
  }
  EXPECT_CALL(drvr, setIntegerParam(Ne(msgIndex), _))
      .Times(testing::AtLeast(0));
  EXPECT_CALL(drvr, setStringParam(Eq(msgIndex), _)).Times(AtLeast(1));
  EXPECT_CALL(drvr,
              setStringParam(Eq(msgIndex),
                             StrEq("Brokers down. Attempting reconnection.")))
      .Times(AtLeast(1));
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

TEST_F(KafkaDriverEnv, SetStatsTimeTest) {
  KafkaDriverStandIn drvr;
  int usedPVIndex = *drvr.paramsList[KafkaDriverStandIn::PV::stats_time].index;
  int newStatsTime = 1000;

  auto tempUser = pasynManager->createAsynUser(nullptr, nullptr);
  tempUser->reason = usedPVIndex;

  EXPECT_CALL(drvr, setIntegerParam(Eq(usedPVIndex), Eq(newStatsTime)))
      .Times(Exactly(1));

  drvr.writeInt32(tempUser, newStatsTime);

  pasynManager->freeAsynUser(tempUser);
}
