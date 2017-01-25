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
#include "KafkaPlugin.h"

using ::testing::Test;
using ::testing::Mock;

class KafkaPluginStandInAlt1 : public KafkaPlugin {
public:
    KafkaPluginStandInAlt1() : KafkaPlugin("some_port", 10, 1, "some_arr_port", 1, 0, 1, 1) {};
    using KafkaPlugin::prod;
    
    std::vector<std::pair<int, std::string>> stringParamRes;
    std::vector<std::pair<int, int>> intParamRes;
    
    std::vector<std::tuple<std::string, asynParamType, int>> createdParams;
    
    virtual asynStatus setStringParam(int index, const char *value) override {
        stringParamRes.push_back(std::pair<int, std::string>(index, value));
        return asynStatus::asynSuccess;
    };
    virtual asynStatus setIntegerParam(int index, const int value) override {
        intParamRes.push_back(std::pair<int, int>(index, value));
        return asynStatus::asynSuccess;
    };
    int indexCtr = 1;
    virtual asynStatus createParam(const char *name, asynParamType type, int *index) override {
        *index = indexCtr;
        indexCtr++;
        createdParams.push_back(std::make_tuple(std::string(name), type, *index));
        return asynStatus::asynSuccess;
    };
    //MOCK_METHOD2(SetConStat, void(KafkaProducerStandIn::ConStat, std::string));
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

TEST_F(KafkaPluginEnv, InitParamsTest) {
    
}
