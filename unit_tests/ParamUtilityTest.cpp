/** Copyright (C) 2017 European Spallation Source */

/** @file  ParamUtilityTest.cpp
 *  @brief Unit tests of the PV utility functions.
 */

#include "ParamUtility.h"
#include <NDPluginDriver.h>
#include <ciso646>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace testing;

/// @brief Simple stand-in class used for unit tests.
class NDPluginDriverStandIn : public NDPluginDriver {
public:
  NDPluginDriverStandIn(const char *portName, int queueSize,
                        int blockingCallbacks, const char *NDArrayPort,
                        int NDArrayAddr, int maxAddr, int numParams,
                        int maxBuffers, size_t maxMemory, int interfaceMask,
                        int interruptMask, int asynFlags, int autoConnect,
                        int priority, int stackSize)
      : NDPluginDriver(portName, queueSize, blockingCallbacks, NDArrayPort,
                       NDArrayAddr, maxAddr, numParams, maxBuffers, maxMemory,
                       interfaceMask, interruptMask, asynFlags, autoConnect,
                       priority, stackSize){};
  MOCK_METHOD2(setStringParam, asynStatus(int, const char *));
  MOCK_METHOD2(setIntegerParam, asynStatus(int, int));
  MOCK_METHOD3(createParam, asynStatus(const char *, asynParamType, int *));
};

/// @brief A testing fixture used for setting up unit tests.
class ParamUtility : public ::testing::Test {
public:
  static void SetUpTestCase() {
    std::string portName("someNameAgain");
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
    plugin = new NDPluginDriverStandIn(
        portName.c_str(), queueSize, blockingCallbacks, NDArrayPort.c_str(),
        NDArrayAddr, 1, numberOfParams, 2, maxMemory, mask1, mask2, 0, 1,
        priority, stackSize);
  };

  static void TearDownTestCase() { delete plugin; };

  virtual void SetUp(){

  };

  virtual void TearDown(){

  };

  static NDPluginDriverStandIn *plugin;
};

NDPluginDriverStandIn *ParamUtility::plugin = nullptr;

MATCHER_P(CharToStringMatcher, matchStr, "") {
  return matchStr == std::string(arg);
}

TEST_F(ParamUtility, InitPvTest) {
  std::vector<PV_param> testParams;
  std::string cKey;
  for (int i = 0; i < 7; i++) {
    testParams.push_back(
        PV_param("PARAM_DESC_" + std::to_string(i), asynParamType(i), i + 64));
    EXPECT_CALL(*plugin,
                createParam(CharToStringMatcher(testParams[i].desc),
                            testParams[i].type, testParams[i].index.get()))
        .Times(testing::Exactly(1));
  }
  InitPvParams(plugin, testParams);
}

TEST_F(ParamUtility, SetIntegerParamTest) {
  std::string descStr = "DESC_1";
  int testValue = 42;
  int testIndex = 11;
  PV_param test(descStr.c_str(), asynParamInt32, testIndex);
  EXPECT_CALL(*plugin, setIntegerParam(testIndex, testValue)).Times(Exactly(1));
  ASSERT_EQ(asynStatus::asynSuccess, setParam(plugin, test, testValue));
}

TEST_F(ParamUtility, SetStringParamTest) {
  std::string descStr = "DESC_1";
  std::string testValue = "some test string,.-";
  int testIndex = 11;
  PV_param test(descStr.c_str(), asynParamOctet, testIndex);
  EXPECT_CALL(*plugin,
              setStringParam(testIndex, CharToStringMatcher(testValue)))
      .Times(Exactly(1));
  ASSERT_EQ(asynStatus::asynSuccess, setParam(plugin, test, testValue));
}

TEST_F(ParamUtility, SetStringParamFailTest) {
  std::string descStr = "DESC_1";
  std::string testValue = "some test string,.-";
  int testIndex = 11;
  PV_param test(descStr.c_str(), asynParamInt32, testIndex);
  ASSERT_DEATH(setParam(plugin, test, testValue), "");
}

TEST_F(ParamUtility, SetIntegerParamFailTest) {
  std::string descStr = "DESC_1";
  int testValue = 42;
  int testIndex = 11;
  PV_param test(descStr.c_str(), asynParamOctet, testIndex);
  ASSERT_DEATH(setParam(plugin, test, testValue), "");
}
