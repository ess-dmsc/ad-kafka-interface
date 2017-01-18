/** Copyright (C) 2016 European Spallation Source */

/** @file  Utility.h
 *  @brief Header file for the C++ implementation of an EPICS areaDetector Kafka-plugin.
 */

#pragma once

#include <NDPluginDriver.h>
#include <string>
#include <map>
#include <memory>

class PV_param {
public:
    PV_param(std::string desc, asynParamType type, int index = 0);
    PV_param();
    const std::string desc;
    const asynParamType type;
    std::shared_ptr<int> index;
};

int InitPvParams(NDPluginDriver *ptr, std::map<std::string, PV_param> &param);

asynStatus setParam(NDPluginDriver *ptr, const PV_param &p, const std::string value);
asynStatus setParam(NDPluginDriver *ptr, const PV_param &p, const int value);
