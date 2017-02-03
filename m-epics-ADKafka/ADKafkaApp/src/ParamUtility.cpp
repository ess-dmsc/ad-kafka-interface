/** Copyright (C) 2017 European Spallation Source */

/** @file  ParamUtility.cpp
 *  @brief Implemetation of EPICS PV utility functions.
 */


#include "ParamUtility.h"
#include <cassert>

PV_param::PV_param(std::string desc, asynParamType type, int index) : desc(desc), type(type), index(new int(index)) {
    
}

PV_param::PV_param() : desc("Not used"), type(asynParamType::asynParamNotDefined), index(nullptr) {
    
}
