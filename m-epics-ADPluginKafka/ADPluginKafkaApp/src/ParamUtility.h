/** Copyright (C) 2017 European Spallation Source */

/** @file  ParamUtility.h
 *  @brief Some helper functions and a PV-struct which simplifies the handling
 * of PV:s.
 */

#pragma once

#include <NDPluginDriver.h>
#include <string>
#include <vector>
#include <memory>

class PV_param {
public:
    PV_param(std::string desc, asynParamType type, int index = 0);
    PV_param();
    const std::string desc;
    const asynParamType type;
    std::shared_ptr<int> index;
};

int InitPvParams(NDPluginDriver *ptr, std::vector<PV_param> &param);

asynStatus setParam(NDPluginDriver *ptr, const PV_param &p, const std::string value);
asynStatus setParam(NDPluginDriver *ptr, const PV_param &p, const int value);
