/** Copyright (C) 2017 European Spallation Source */

/** @file  ParamUtility.h
 *  @brief Some helper functions and a PV-struct which simplifies the handling
 * of PV:s.
 */

#pragma once

#include <asynNDArrayDriver.h>
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

template <class asynNDArrType>
int InitPvParams(asynNDArrType *ptr, std::vector<PV_param> &param) {
    int minParamIndex = -1;
    for (auto &p : param) {
        ptr->createParam(p.desc.c_str(), p.type, p.index.get());
        if (-1 == minParamIndex) {
            minParamIndex = *p.index;
        } else if (minParamIndex > *p.index) {
            minParamIndex = *p.index;
        }
    }
    return minParamIndex;
}

template <class asynNDArrType>
asynStatus setParam(asynNDArrType *ptr, const PV_param &p, const std::string value) {
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

template <typename asynNDArrType>
asynStatus setParam(asynNDArrType *ptr, const PV_param &p, const int value) {
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
