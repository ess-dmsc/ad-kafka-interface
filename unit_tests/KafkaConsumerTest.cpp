/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaConsumerTest.cpp
 *  @brief Unit tests of the Kafka consumer class.
 */

#include "KafkaConsumer.h"
#include <asynNDArrayDriver.h>
#include <chrono>
#include <ciso646>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

/// @brief Simple stand-in class used for unit tests.
class KafkaConsumerStandIn : public KafkaInterface::KafkaConsumer {
public:
  KafkaConsumerStandIn() : KafkaConsumer("some_group"){};
  KafkaConsumerStandIn(std::string addr, std::string topic)
      : KafkaInterface::KafkaConsumer(addr, topic, "some_group"){};
  using KafkaInterface::KafkaConsumer::errorState;
  using KafkaInterface::KafkaConsumer::ConStat;
  using KafkaInterface::KafkaConsumer::kafka_stats_interval;
  using KafkaInterface::KafkaConsumer::consumer;
  using KafkaInterface::KafkaConsumer::paramCallback;
  using KafkaInterface::KafkaConsumer::PV;
  using KafkaInterface::KafkaConsumer::paramsList;
  void SetConStatParent(KafkaConsumerStandIn::ConStat stat, std::string msg) {
    KafkaInterface::KafkaConsumer::SetConStat(stat, msg);
  };
  bool MakeConnectionParent() {
    return KafkaInterface::KafkaConsumer::MakeConnection();
  };
  MOCK_METHOD1(ParseStatusString, void(std::string const&));
  MOCK_METHOD0(MakeConnection, bool(void));
  MOCK_METHOD0(UpdateTopic, bool(void));
  MOCK_METHOD2(SetConStat, void(KafkaConsumerStandIn::ConStat, std::string));
};

/// @brief Simple stand-in class used for unit tests.
class KafkaConsumerStandInAlt : public KafkaInterface::KafkaConsumer {
public:
  KafkaConsumerStandInAlt() : KafkaConsumer("some_group"){};
  KafkaConsumerStandInAlt(std::string addr, std::string topic)
      : KafkaInterface::KafkaConsumer(addr, topic, "some_group"){};
  using KafkaInterface::KafkaConsumer::errorState;
  using KafkaInterface::KafkaConsumer::ConStat;
  using KafkaInterface::KafkaConsumer::kafka_stats_interval;
  using KafkaInterface::KafkaConsumer::consumer;
};

/// @brief Simple stand-in class used for unit tests.
class asynNDArrayDriverStandIn : public asynNDArrayDriver {
public:
  asynNDArrayDriverStandIn(const char *portName, int maxAddr,
                           int maxBuffers, size_t maxMemory, int interfaceMask,
                           int interruptMask, int asynFlags, int autoConnect,
                           int priority, int stackSize)
      : asynNDArrayDriver(portName, maxAddr, maxBuffers, maxMemory,
                          interfaceMask, interruptMask, asynFlags, autoConnect,
                          priority, stackSize){};
  MOCK_METHOD2(setStringParam, asynStatus(int, const char *));
  MOCK_METHOD2(setIntegerParam, asynStatus(int, int));
  MOCK_METHOD3(createParam, asynStatus(const char *, asynParamType, int *));
};

/// @brief A testing fixture used for setting up unit tests.
class KafkaConsumerEnv : public ::testing::Test {
public:
  static void SetUpTestCase() {
    std::string portName("someNameSecond");
    size_t maxMemory = 10;
    int mask1 = asynInt8ArrayMask | asynInt16ArrayMask | asynInt32ArrayMask |
                asynFloat32ArrayMask | asynFloat64ArrayMask;
    int mask2 = asynInt8ArrayMask | asynInt16ArrayMask | asynInt32ArrayMask |
                asynFloat32ArrayMask | asynFloat64ArrayMask;
    int priority = 0;
    int stackSize = 5;
    asynDrvr = new asynNDArrayDriverStandIn(portName.c_str(), 1,
                                            2, maxMemory, mask1, mask2, 0, 1,
                                            priority, stackSize);
  };

  static void TearDownTestCase() {
    delete asynDrvr;
    asynDrvr = nullptr;
  };

  virtual void SetUp(){

  };

  virtual void TearDown(){

  };

  static asynNDArrayDriverStandIn *asynDrvr;
};

asynNDArrayDriverStandIn *KafkaConsumerEnv::asynDrvr = nullptr;

typedef std::chrono::milliseconds TimeT;
using namespace testing;
using ::testing::Mock;

namespace KafkaInterface {
TEST_F(KafkaConsumerEnv, ConnectionSuccessTest) {
  KafkaConsumerStandInAlt cons("some_addr", "some_topic");
  ASSERT_NE(cons.consumer, nullptr);
}

TEST_F(KafkaConsumerEnv, ParameterCountTest) {
  KafkaConsumerStandIn cons;
  ASSERT_EQ(cons.paramsList.size(), KafkaConsumerStandIn::PV::count);
}

TEST_F(KafkaConsumerEnv, ConnectionFailTest) {
  KafkaConsumerStandInAlt cons;
  ASSERT_EQ(cons.consumer, nullptr);
}

TEST_F(KafkaConsumerEnv, StatsTest) {
  KafkaConsumerStandIn cons("some_broker", "some_topic");
  EXPECT_CALL(cons, ParseStatusString(_)).Times(AtLeast(1));
  auto msg = cons.WaitForPkg(1000);
}

TEST_F(KafkaConsumerEnv, NoWaitTest) {
  KafkaConsumer cons("some_group");
  auto start = std::chrono::steady_clock::now();
  auto msg = cons.WaitForPkg(1000);
  auto duration = std::chrono::duration_cast<TimeT>(
      std::chrono::steady_clock::now() - start);
  ASSERT_LT(duration.count(), 100);
  ASSERT_EQ(msg, nullptr);
}

TEST_F(KafkaConsumerEnv, SetOffsetSuccess1Test) {
  KafkaConsumer cons("some_group");
  cons.RegisterParamCallbackClass(asynDrvr);
  int ctr = 1;
  for (auto p : cons.GetParams()) {
    *p.index = ctr;
    ctr++;
  }
  std::int64_t usedValue = RdKafka::Topic::OFFSET_BEGINNING;
  EXPECT_CALL(*asynDrvr, setIntegerParam(_, Eq(usedValue))).Times(Exactly(1));
  ASSERT_TRUE(cons.SetOffset(usedValue));
  ASSERT_EQ(cons.GetCurrentOffset(), usedValue);
  Mock::VerifyAndClear(asynDrvr);
}

TEST_F(KafkaConsumerEnv, SetOffsetSuccess2Test) {
  KafkaConsumer cons("some_group");
  cons.RegisterParamCallbackClass(asynDrvr);
  int ctr = 1;
  for (auto p : cons.GetParams()) {
    *p.index = ctr;
    ctr++;
  }
  std::int64_t usedValue = RdKafka::Topic::OFFSET_END;
  EXPECT_CALL(*asynDrvr, setIntegerParam(_, Eq(usedValue))).Times(Exactly(1));
  ASSERT_TRUE(cons.SetOffset(usedValue));
  ASSERT_EQ(cons.GetCurrentOffset(), usedValue);
  Mock::VerifyAndClear(asynDrvr);
}

TEST_F(KafkaConsumerEnv, SetOffsetSuccess3Test) {
  KafkaConsumer cons("some_group");
  cons.RegisterParamCallbackClass(asynDrvr);
  int ctr = 1;
  for (auto p : cons.GetParams()) {
    *p.index = ctr;
    ctr++;
  }
  std::int64_t usedValue = RdKafka::Topic::OFFSET_STORED;
  EXPECT_CALL(*asynDrvr, setIntegerParam(_, Eq(usedValue))).Times(Exactly(1));
  ASSERT_TRUE(cons.SetOffset(usedValue));
  ASSERT_EQ(cons.GetCurrentOffset(), usedValue);
  Mock::VerifyAndClear(asynDrvr);
}

TEST_F(KafkaConsumerEnv, SetOffsetFailTest) {
  KafkaConsumer cons("some_group");
  cons.RegisterParamCallbackClass(asynDrvr);
  int ctr = 1;
  for (auto p : cons.GetParams()) {
    *p.index = ctr;
    ctr++;
  }
  int usedValue = -3;
  EXPECT_CALL(*asynDrvr, setIntegerParam(_, Eq(usedValue))).Times(Exactly(0));
  ASSERT_FALSE(cons.SetOffset(usedValue));
  ASSERT_NE(cons.GetCurrentOffset(), usedValue);
  Mock::VerifyAndClear(asynDrvr);
}

TEST_F(KafkaConsumerEnv, WaitTest) {
  KafkaConsumer cons("some_addr", "some_topic", "some_group");
  auto start = std::chrono::steady_clock::now();

  int waitTime = 1000;

  auto msg = cons.WaitForPkg(waitTime);
  auto duration = std::chrono::duration_cast<TimeT>(
      std::chrono::steady_clock::now() - start);
  ASSERT_GE(duration.count(), waitTime - 10);
  ASSERT_EQ(msg, nullptr);
}

TEST_F(KafkaConsumerEnv, StatsStatusTest) {
  KafkaConsumer cons("some_addr", "some_topic", "some_group");
  auto params = cons.GetParams();
  cons.RegisterParamCallbackClass(asynDrvr);
  int ctr = 1;
  for (auto p : params) {
    *p.index = ctr;
    ctr++;
  }
  int statusIndex = *params[KafkaConsumerStandIn::PV::con_status].index;
  EXPECT_CALL(*asynDrvr, setIntegerParam(Ne(statusIndex), _)).Times(AtLeast(0));
  EXPECT_CALL(*asynDrvr, setIntegerParam(Eq(statusIndex), _)).Times(AtLeast(1));
  auto msg = cons.WaitForPkg(1000);

  Mock::VerifyAndClear(asynDrvr);
}

TEST_F(KafkaConsumerEnv, ErrorStateTest) {
  KafkaConsumerStandInAlt cons;
  cons.errorState = true;
  ASSERT_FALSE(cons.SetTopic("some_topic_2"));
  ASSERT_FALSE(cons.SetBrokerAddr("some_topic_2"));
  ASSERT_FALSE(cons.SetGroupId("some_topic_2"));
  ASSERT_FALSE(cons.SetStatsTimeIntervalMS(100));
}

TEST_F(KafkaConsumerEnv, NoErrorStateTest) {
  KafkaConsumer cons("addr", "tpic", "some_group");
  ASSERT_TRUE(cons.SetTopic("some_topic_2"));
  ASSERT_TRUE(cons.SetBrokerAddr("some_topic_2"));
  ASSERT_TRUE(cons.SetGroupId("some_topic_2"));
  ASSERT_TRUE(cons.SetStatsTimeIntervalMS(100));
}

TEST_F(KafkaConsumerEnv, NegativeTimeTest) {
  KafkaConsumer cons("addr", "tpic", "some_group");
  ASSERT_FALSE(cons.SetStatsTimeIntervalMS(-1));
}

TEST_F(KafkaConsumerEnv, ZeroTimeTest) {
  KafkaConsumer cons("addr", "tpic", "some_group");
  ASSERT_FALSE(cons.SetStatsTimeIntervalMS(0));
}

TEST_F(KafkaConsumerEnv, ZeroLengthGroupIdTest) {
  KafkaConsumer cons("addr", "tpic", "some_group");
  ASSERT_FALSE(cons.SetGroupId(""));
}

TEST_F(KafkaConsumerEnv, ZeroLengthBrokerTest) {
  KafkaConsumer cons("addr", "tpic", "some_group");
  ASSERT_FALSE(cons.SetBrokerAddr(""));
}

TEST_F(KafkaConsumerEnv, ZeroLengthTopicTest) {
  KafkaConsumer cons("addr", "tpic", "some_group");
  ASSERT_FALSE(cons.SetTopic(""));
}

TEST_F(KafkaConsumerEnv, RegCallbackTest) {
  KafkaConsumerStandIn cons;
  cons.RegisterParamCallbackClass(asynDrvr);
  ASSERT_EQ(asynDrvr, cons.paramCallback);
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
  cons.SetStatsTimeIntervalMS(100);
}

TEST_F(KafkaConsumerEnv, SetStatsTimeValueTest) {
  KafkaConsumer cons("addr", "tpic", "some_group");
  int usedTime = 100;
  cons.SetStatsTimeIntervalMS(usedTime);
  ASSERT_EQ(usedTime, cons.GetStatsTimeMS());
}

TEST_F(KafkaConsumerEnv, GetGroupIdTest) {
  std::string testString = "some_group_id1";
  KafkaConsumer cons("addr", "tpic", testString);
  ASSERT_EQ(testString, cons.GetGroupId());
}

TEST_F(KafkaConsumerEnv, GetGroupIdAltTest) {
  std::string testString = "some_group_id2";
  KafkaConsumer cons(testString);
  ASSERT_EQ(testString, cons.GetGroupId());
}

TEST_F(KafkaConsumerEnv, SetGetGroupIdTest) {
  std::string testString = "some_group_id3";
  KafkaConsumer cons("addr", "topic", "some_group");
  cons.SetGroupId(testString);
  ASSERT_EQ(testString, cons.GetGroupId());
}

TEST_F(KafkaConsumerEnv, SetConStatTest) {
  KafkaConsumerStandIn cons("addr", "tpic");
  auto params = cons.GetParams();
  int ctr = 1;
  for (auto p : params) {
    *p.index = ctr;
    ctr++;
  }
  int messageIndex = *params[KafkaConsumerStandIn::PV::con_msg].index;
  int statusIndex = *params[KafkaConsumerStandIn::PV::con_status].index;
  cons.RegisterParamCallbackClass(asynDrvr);
  EXPECT_CALL(*asynDrvr,
              setIntegerParam(Eq(statusIndex),
                              Eq(int(KafkaConsumerStandIn::ConStat::ERROR))))
      .Times(Exactly(1));
  EXPECT_CALL(*asynDrvr, setStringParam(Eq(messageIndex), _)).Times(Exactly(1));
  cons.SetConStatParent(KafkaConsumerStandIn::ConStat::ERROR, "some message");
  Mock::VerifyAndClear(asynDrvr);
}

TEST_F(KafkaConsumerEnv, TestNrOfParams) {
  KafkaConsumer prod("some_addr", "some_topic", "some_group");
  ASSERT_EQ(prod.GetParams().size(), prod.GetNumberOfPVs());
}
}
