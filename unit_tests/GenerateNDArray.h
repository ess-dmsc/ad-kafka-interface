/** Copyright (C) 2017 European Spallation Source */

/** @file  GenerateNDArray.h
 *  @brief Header file of the helper class which is used to generate fake data
 * for the unit tests.
 */

#pragma once

#include <NDPluginDriver.h>
#include <algorithm>
#include <ciso646>
#include <cstring>
#include <memory>
#include <random>
#include <set>

class NDArrayGenerator {
public:
  NDArrayGenerator();
  ~NDArrayGenerator();

  NDArray *GenerateNDArray(size_t numAttr, size_t numElem, int dims,
                           NDDataType_t dType);
  std::set<std::string> usedAttrStrings;

private:
  std::string RandomString(size_t length);
  void *GenerateAttrData(NDAttrDataType_t type);

  template <typename T> void FreeAttrDataT(void *ptr) {
    T *tempPtr = reinterpret_cast<T*>(ptr);
    delete tempPtr;
  }

  template <typename T> void *GenerateAttrDataT(T min, T max) {
    if (std::is_same<T, std::double_t>::value or
        std::is_same<T, std::float_t>::value) {
      std::uniform_real_distribution<> valueDist(min, max);
      return reinterpret_cast<void*>(new T(valueDist(eng)));
    }
    std::uniform_int_distribution<> valueDist(min, max);
    return reinterpret_cast<void*>(new T(valueDist(eng)));
  }

  void FreeAttrData(void *ptr, NDAttrDataType_t type);
  NDAttribute *GenerateAttribute();

  std::default_random_engine eng;
  std::random_device r;
  int idCtr;
  std::unique_ptr<NDArrayPool> sendPool;
};
