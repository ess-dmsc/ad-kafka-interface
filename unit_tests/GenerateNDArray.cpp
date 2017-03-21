/** Copyright (C) 2017 European Spallation Source */

/** @file  GenerateNDArray.cpp
 *  @brief Implementation of helper class and helper functions which are used to
 * generate fake data
 * for the unit tests.
 */

#include "GenerateNDArray.h"
#include <ciso646>
#include <cstdlib>
#include <cassert>

template <typename T> void PopulateArr(size_t elements, void *ptr) {
  T *arr = reinterpret_cast<T*>(ptr);
  for (int y = 0; y < elements; y++) {
    arr[y] = static_cast<T>(y);
  }
}

void GenerateData(NDDataType_t type, size_t elements, void *usedPtr) {
  if (NDInt8 == type) {
    PopulateArr<std::int8_t>(elements, usedPtr);
  } else if (NDUInt8 == type) {
    PopulateArr<std::uint8_t>(elements, usedPtr);
  } else if (NDInt16 == type) {
    PopulateArr<std::int16_t>(elements, usedPtr);
  } else if (NDUInt16 == type) {
    PopulateArr<std::uint16_t>(elements, usedPtr);
  } else if (NDUInt32 == type) {
    PopulateArr<std::uint32_t>(elements, usedPtr);
  } else if (NDInt32 == type) {
    PopulateArr<std::int32_t>(elements, usedPtr);
  } else if (NDFloat32 == type) {
    PopulateArr<std::float_t>(elements, usedPtr);
  } else if (NDFloat64 == type) {
    PopulateArr<std::double_t>(elements, usedPtr);
  } else {
      assert(false);
  }
}

NDArrayGenerator::NDArrayGenerator() : sendPool(new NDArrayPool(1, 0)) {
  idCtr = 0;
  eng = std::default_random_engine(r());
}

NDArrayGenerator::~NDArrayGenerator() {}

std::string NDArrayGenerator::RandomString(size_t length) {
  auto randchar = []() -> char {
    const char charset[] = "0123456789 "
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
                           "abcdefghijklmnopqrstuvwxyz ";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  if (usedAttrStrings.find(str) != usedAttrStrings.end()) {
    return RandomString(length);
  } else {
    usedAttrStrings.emplace(str);
  }
  return str;
}

void *NDArrayGenerator::GenerateAttrData(NDAttrDataType_t type) {
  void *ptr = nullptr;
  std::default_random_engine e1(r());

  if (NDAttrString == type) {
    std::uniform_int_distribution<size_t> strLenDist(10, 1000);
    std::string tempStr = RandomString(strLenDist(e1));
    char *buffer = new char[tempStr.size() + 1];
    std::strncpy(buffer, tempStr.c_str(), tempStr.size() + 1);
    return reinterpret_cast<void*>(buffer);
  } else if (NDAttrInt8 == type) {
    ptr = GenerateAttrDataT<std::int8_t>(INT8_MIN, INT8_MAX);
  } else if (NDAttrUInt8 == type) {
    ptr = GenerateAttrDataT<std::uint8_t>(0, UINT8_MAX);
  } else if (NDAttrUInt16 == type) {
    ptr = GenerateAttrDataT<std::uint16_t>(0, UINT16_MAX);
  } else if (NDAttrInt16 == type) {
    ptr = GenerateAttrDataT<std::int16_t>(INT16_MIN, INT16_MAX);
  } else if (NDAttrInt32 == type) {
    ptr = GenerateAttrDataT<std::int32_t>(INT32_MIN, INT32_MAX);
  } else if (NDAttrUInt32 == type) {
    ptr = GenerateAttrDataT<std::uint32_t>(0, UINT32_MAX);
  } else if (NDAttrFloat32 == type) {
    ptr = GenerateAttrDataT<std::float_t>(-1, 1);
  } else if (NDAttrFloat64 == type) {
    ptr = GenerateAttrDataT<std::double_t>(-1, 1);
  } else {
    assert(false);
  }
  return ptr;
}

void NDArrayGenerator::FreeAttrData(void *ptr, NDAttrDataType_t type) {
  if (NDAttrString == type) {
    char *tempPtr = reinterpret_cast<char*>(ptr);
    delete[] tempPtr;
  } else if (NDAttrInt8 == type) {
    FreeAttrDataT<std::int8_t>(ptr);
  } else if (NDAttrUInt8 == type) {
    FreeAttrDataT<std::uint8_t>(ptr);
  } else if (NDAttrUInt16 == type) {
    FreeAttrDataT<std::uint16_t>(ptr);
  } else if (NDAttrInt16 == type) {
    FreeAttrDataT<std::int16_t>(ptr);
  } else if (NDAttrInt32 == type) {
    FreeAttrDataT<std::int32_t>(ptr);
  } else if (NDAttrUInt32 == type) {
    FreeAttrDataT<std::uint32_t>(ptr);
  } else if (NDAttrFloat32 == type) {
    FreeAttrDataT<std::float_t>(ptr);
  } else if (NDAttrFloat64 == type) {
    FreeAttrDataT<std::double_t>(ptr);
  } else {
    assert(false);
  }
}

NDAttribute *NDArrayGenerator::GenerateAttribute() {
  std::vector<size_t> attrStringLengths = {1, 10, 100, 1000};

  std::default_random_engine e1(r());
  std::uniform_int_distribution<size_t> strLenDist(0, attrStringLengths.size() -
                                                          1);

  std::string attrName = RandomString(attrStringLengths[strLenDist(e1)]);
  std::string attrDesc = RandomString(attrStringLengths[strLenDist(e1)]);
  std::string attrSource = RandomString(attrStringLengths[strLenDist(e1)]);

  std::vector<NDAttrDataType_t> attrDataTypes = {
      NDAttrUInt8, NDAttrInt8,    NDAttrInt16,   NDAttrUInt16, NDAttrUInt32,
      NDAttrInt32, NDAttrFloat32, NDAttrFloat64, NDAttrString};

  std::uniform_int_distribution<size_t> dataTypeDist(0,
                                                     attrDataTypes.size() - 1);

  NDAttrDataType_t usedDType = attrDataTypes[dataTypeDist(e1)];

  void *attrDataPtr = GenerateAttrData(usedDType);
  NDAttribute *retAttr =
      new NDAttribute(attrName.c_str(), attrDesc.c_str(), NDAttrSourceDriver,
                      attrSource.c_str(), usedDType, attrDataPtr);
  FreeAttrData(attrDataPtr, usedDType);
  return retAttr;
}

NDArray *NDArrayGenerator::GenerateNDArray(size_t numAttr, size_t numElem,
                                           int dims, NDDataType_t dType) {

  size_t elements = numElem;
  std::vector<size_t> usedDimensions;
  usedDimensions.push_back(numElem);
  for (int k = 1; k < dims; k++) {
    elements *= (numElem + k * 2);
    usedDimensions.push_back(numElem + k * 2);
  }
  NDArray *pArr =
      sendPool->alloc(dims, usedDimensions.data(), dType, 0, nullptr);
  GenerateData(dType, elements, pArr->pData);
  pArr->uniqueId = idCtr;
  idCtr++;
  pArr->timeStamp = M_PI;
  pArr->epicsTS.nsec = 21212121;
  pArr->epicsTS.secPastEpoch = 1484046150;
  pArr->pAttributeList->clear();
  for (int y = 0; y < numAttr; y++) {
    pArr->pAttributeList->add(GenerateAttribute());
  }
  return pArr;
}
