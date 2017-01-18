/** Copyright (C) 2016 European Spallation Source */

/** @file  KafkaPlugin.cpp
 *  @brief C++ implementation file for an EPICS areaDetector Kafka-plugin.
 */


#include "ParamUtility.h"
#include <cassert>

PV_param::PV_param(std::string desc, asynParamType type, int index) : desc(desc), type(type), index(new int(index)) {
    
}

PV_param::PV_param() : desc("Not used"), type(asynParamType::asynParamNotDefined), index(nullptr) {
    
}

int InitPvParams(NDPluginDriver *ptr, std::map<std::string, PV_param> &params) {
    int minParamIndex = -1;
    for (auto &p : params) {
        ptr->createParam(p.second.desc.c_str(), p.second.type, p.second.index.get());
        if (-1 == minParamIndex) {
            minParamIndex = *p.second.index;
        } else if (minParamIndex > *p.second.index) {
            minParamIndex = *p.second.index;
        }
    }
    return minParamIndex;
}

asynStatus setParam(NDPluginDriver *ptr, const PV_param &p, const std::string value) {
    if (nullptr == ptr or 0 == *p.index) {
        return asynStatus::asynError;
    }
    asynStatus ret;
    if (asynParamOctet == p.type) {
        ret = ptr->setStringParam(*p.index, value.c_str());
    } else {
        assert(false);
    }
    return ret;
}

asynStatus setParam(NDPluginDriver *ptr, const PV_param &p, const int value) {
    if (nullptr == ptr or 0 == *p.index) {
        return asynStatus::asynError;
    }
    asynStatus ret;
    if (asynParamInt32 == p.type) {
        ret = ptr->setIntegerParam(*p.index, value);
    } else {
        assert(false);
    }
    return ret;
}
