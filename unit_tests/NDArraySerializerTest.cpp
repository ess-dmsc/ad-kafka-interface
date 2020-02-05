/** Copyright (C) 2017 European Spallation Source */

/** @file  NDArraySerializerTest.cpp
 *  @brief Unit tests of the serialization and de-serialization of NDArray data.
 */

#include "GenerateNDArray.h"
#include "NDArrayDeSerializer.h"
#include "NDArraySerializer.h"
#include "NDArray_schema_generated.h"
#include <ciso646>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <map>
#include <random>
#include <set>

/// @brief Simple stand-in class used for unit tests.
class NDArraySerializerStandIn : public NDArraySerializer {
public:
  using NDArraySerializer::GetFB_DType;
  using NDArraySerializer::GetND_DType;
  using NDArraySerializer::GetND_AttrDType;
};

void CompareDataTypes(NDArray *arr1, NDArray *arr2);
void CompareDataTypes(NDArray *arr1, const FB_Tables::NDArray *arr2);

void CompareSizeAndDims(NDArray *arr1, NDArray *arr2);
void CompareSizeAndDims(NDArray *arr1, const FB_Tables::NDArray *arr2);

void CompareTimeStamps(NDArray *arr1, NDArray *arr2);
void CompareTimeStamps(NDArray *arr1, const FB_Tables::NDArray *arr2);

void CompareData(NDArray *arr1, NDArray *arr2);
void CompareData(NDArray *arr1, const FB_Tables::NDArray *arr2);

void CompareAttributes(NDArray *arr1, NDArray *arr2);
void CompareAttributes(NDArray *arr1, const FB_Tables::NDArray *arr2);

void GenerateData(NDDataType_t type, size_t elements, void *ptr);

/// @brief A testing fixture used for setting up unit tests.
class Serializer : public ::testing::Test {
public:
  static void SetUpTestCase(){

  };

  static void TearDownTestCase(){

  };

  virtual void SetUp() {
    arrGen = new NDArrayGenerator();
    recvPool = new NDArrayPool(nullptr, 0);
  };

  virtual void TearDown() {
    delete arrGen;
    delete recvPool;
  };
  NDArrayGenerator *arrGen;
  NDArrayPool *recvPool;
};

TEST_F(Serializer, SerializeTest) {
  NDArraySerializer ser;
  std::vector<size_t> numAttr = {0, 1, 10};
  std::vector<size_t> numElements = {1, 10, 50};
  std::vector<NDDataType_t> dataTypes = {NDUInt8,   NDInt8,   NDUInt16,
                                         NDInt16,   NDInt32,  NDUInt32,
                                         NDFloat32, NDFloat64};
  std::vector<int> numDims = {1, 2, 3, 4};
  NDArray *sendArr = nullptr;
  for (auto nAttr : numAttr) {
    for (auto nElem : numElements) {
      for (auto dType : dataTypes) {
        for (auto nDim : numDims) {
          sendArr = arrGen->GenerateNDArray(nAttr, nElem, nDim, dType);
          unsigned char *bufferPtr = nullptr;
          size_t bufferSize;
          ser.SerializeData(*sendArr, bufferPtr, bufferSize);
          auto recvArr = FB_Tables::GetNDArray(bufferPtr);
          CompareDataTypes(sendArr, recvArr);
          CompareSizeAndDims(sendArr, recvArr);
          CompareTimeStamps(sendArr, recvArr);
          CompareData(sendArr, recvArr);
          CompareAttributes(sendArr, recvArr);
          sendArr->release();
          arrGen->usedAttrStrings.clear();
        }
      }
    }
  }
  delete sendArr;
}

/// @brief A testing fixture used for setting up unit tests.
class DeSerializer : public ::testing::Test {
public:
  static void SetUpTestCase() {
    std::string DataFilePath{TEST_DATA_PATH};
    std::ifstream inFile(DataFilePath + "someNDArray.data",
                         std::ifstream::in | std::ifstream::binary);
    inFile.seekg(0, inFile.end);
    fileSize = inFile.tellg();
    rawData = new unsigned char[fileSize];
    inFile.seekg(0, inFile.beg);
    inFile.read((char *)rawData, fileSize);
  };

  static void TearDownTestCase() { delete[] rawData; };

  virtual void SetUp(){};

  virtual void TearDown(){};
  static unsigned char *rawData;
  static size_t fileSize;
};

unsigned char *DeSerializer::rawData = nullptr;
size_t DeSerializer::fileSize = 0;

TEST_F(DeSerializer, FileIntegrityTest) {
  ASSERT_NE(rawData, nullptr);
  flatbuffers::Verifier verifier(DeSerializer::rawData, DeSerializer::fileSize);
  ASSERT_GE(DeSerializer::fileSize, 1);
  ASSERT_TRUE(FB_Tables::VerifyNDArrayBuffer(verifier));
}

TEST_F(DeSerializer, FileContentTest) {
  ASSERT_NE(rawData, nullptr);
  ASSERT_GE(DeSerializer::fileSize, 1);
  auto recvArr = FB_Tables::GetNDArray(rawData);
  ASSERT_EQ(recvArr->id(), 2720);
}

// TEST_F(Serializer, SerializeDeserializeProfiling) {
//    NDArraySerializer ser;
//    size_t numAttr = 10;
//    size_t numElements = 50;
//    NDDataType_t dataType = NDInt32;
//    int numDims = 4;
//    NDArray *sendArr = nullptr;
//    NDArray *recvArr = nullptr;
//    for (int i = 0; i < 2000; i++) {
//        sendArr = arrGen->GenerateNDArray(numAttr, numElements, numDims,
//        dataType);
//        unsigned char *bufferPtr = nullptr;
//        size_t bufferSize;
//        ser.SerializeData(*sendArr, bufferPtr, bufferSize);
//        DeSerializeData(recvPool, bufferPtr, bufferSize, recvArr);
//        CompareDataTypes(sendArr, recvArr);
//        CompareSizeAndDims(sendArr, recvArr);
//        CompareTimeStamps(sendArr, recvArr);
//        CompareData(sendArr, recvArr);
//        CompareAttributes(sendArr, recvArr);
//        sendArr->release();
//        recvArr->release();
//        arrGen->usedAttrStrings.clear();
//    }
//    delete sendArr;
//    delete recvArr;
//}

TEST_F(Serializer, SerializeDeserializeTest) {
  NDArraySerializer ser;
  std::vector<size_t> numAttr = {0, 1, 10};
  std::vector<size_t> numElements = {1, 10, 50};
  std::vector<NDDataType_t> dataTypes = {NDUInt8,   NDInt8,   NDUInt16,
                                         NDInt16,   NDInt32,  NDUInt32,
                                         NDFloat32, NDFloat64};
  std::vector<int> numDims = {1, 2, 3, 4};
  NDArray *sendArr = nullptr;
  NDArray *recvArr = nullptr;
  for (auto nAttr : numAttr) {
    for (auto nElem : numElements) {
      for (auto dType : dataTypes) {
        for (auto nDim : numDims) {
          sendArr = arrGen->GenerateNDArray(nAttr, nElem, nDim, dType);
          unsigned char *bufferPtr = nullptr;
          size_t bufferSize;
          ser.SerializeData(*sendArr, bufferPtr, bufferSize);
          DeSerializeData(recvPool, bufferPtr, recvArr);
          CompareDataTypes(sendArr, recvArr);
          CompareSizeAndDims(sendArr, recvArr);
          CompareTimeStamps(sendArr, recvArr);
          CompareData(sendArr, recvArr);
          CompareAttributes(sendArr, recvArr);
          sendArr->release();
          recvArr->release();
          arrGen->usedAttrStrings.clear();
        }
      }
    }
  }
  delete sendArr;
  delete recvArr;
}

void CompareDataTypes(NDArray *arr1, NDArray *arr2) {
  ASSERT_EQ(arr1->dataType, arr2->dataType);
}

void CompareDataTypes(NDArray *arr1, const FB_Tables::NDArray *arr2) {
  ASSERT_EQ(arr1->dataType,
            NDArraySerializerStandIn::GetND_DType(arr2->dataType()));
}

void CompareSizeAndDims(NDArray *arr1, NDArray *arr2) {
  std::map<NDDataType_t, int> sizeList = {
      {NDInt8, 1},  {NDUInt8, 1},  {NDInt16, 2},   {NDUInt16, 2},
      {NDInt32, 4}, {NDUInt32, 4}, {NDFloat32, 4}, {NDFloat64, 8},
  };
  NDArrayInfo_t arr1Info;
  NDArrayInfo_t arr2Info;
  arr1->getInfo(&arr1Info);
  arr2->getInfo(&arr2Info);

  auto getNumElements = [](NDArray *arr) {
    if (0 == arr->ndims) {
      return std::uint64_t(0);
    }
    std::uint64_t retVal = arr->dims[0].size;
    for (int j = 1; j < arr->ndims; j++) {
      retVal *= arr->dims[j].size;
    }
    return retVal;
  };

  std::uint64_t arr1Size = getNumElements(arr1) * sizeList[arr1->dataType];
  std::uint64_t arr2Size = getNumElements(arr2) * sizeList[arr2->dataType];

  ASSERT_EQ(arr1Info.totalBytes, arr2Info.totalBytes);
  ASSERT_EQ(arr2Info.totalBytes, arr1Size);
  ASSERT_EQ(arr1Size, arr2Size);
}

void CompareSizeAndDims(NDArray *arr1, const FB_Tables::NDArray *arr2) {
  std::map<NDDataType_t, int> sizeList = {
      {NDInt8, 1},  {NDUInt8, 1},  {NDInt16, 2},   {NDUInt16, 2},
      {NDInt32, 4}, {NDUInt32, 4}, {NDFloat32, 4}, {NDFloat64, 8},
  };
  NDArrayInfo_t arr1Info;
  arr1->getInfo(&arr1Info);

  auto getNumElements = [](NDArray *arr) {
    if (0 == arr->ndims) {
      return std::uint64_t(0);
    }
    std::uint64_t retVal = arr->dims[0].size;
    for (int j = 1; j < arr->ndims; j++) {
      retVal *= arr->dims[j].size;
    }
    return retVal;
  };

  size_t arr1Size = getNumElements(arr1) * sizeList[arr1->dataType];
  size_t arr2Size = arr2->pData()->size();
  ASSERT_EQ(arr1Info.totalBytes, arr1Size);
  ASSERT_EQ(arr1Size, arr2Size);
}

void CompareTimeStamps(NDArray *arr1, NDArray *arr2) {
  ASSERT_EQ(arr1->timeStamp, arr2->timeStamp);
  ASSERT_EQ(arr1->epicsTS.secPastEpoch, arr2->epicsTS.secPastEpoch);
  ASSERT_EQ(arr1->epicsTS.nsec, arr2->epicsTS.nsec);
}

void CompareTimeStamps(NDArray *arr1, const FB_Tables::NDArray *arr2) {
  ASSERT_EQ(arr1->timeStamp, arr2->timeStamp());
  ASSERT_EQ(arr1->epicsTS.secPastEpoch, arr2->epicsTS()->secPastEpoch());
  ASSERT_EQ(arr1->epicsTS.nsec, arr2->epicsTS()->nsec());
}

void CompareData(NDArray *arr1, NDArray *arr2) {
  NDArrayInfo_t arr1Info;
  NDArrayInfo_t arr2Info;
  arr1->getInfo(&arr1Info);
  arr2->getInfo(&arr2Info);

  // memcmp returns 0 if the data is the same, something else otherwise
  ASSERT_EQ(std::memcmp(arr1->pData, arr2->pData, arr1Info.totalBytes), 0);
}

void CompareData(NDArray *arr1, const FB_Tables::NDArray *arr2) {
  NDArrayInfo_t arr1Info;
  arr1->getInfo(&arr1Info);

  // memcmp returns 0 if the data is the same, something else otherwise
  ASSERT_EQ(
      std::memcmp(arr1->pData, arr2->pData()->data(), arr1Info.totalBytes), 0);
}

void CompareAttributes(NDArray *arr1, const FB_Tables::NDArray *arr2) {
  ASSERT_EQ(arr1->pAttributeList->count(), arr2->pAttributeList()->size());

  std::set<const FB_Tables::NDAttribute *> attrPtrs;

  std::map<std::string, const FB_Tables::NDAttribute *> compAttrList;
  for (int u = 0; u < arr2->pAttributeList()->size(); u++) {
    const FB_Tables::NDAttribute *attPtr = arr2->pAttributeList()->Get(u);
    compAttrList[attPtr->pName()->str()] = attPtr;
  }
  ASSERT_EQ(compAttrList.size(), arr1->pAttributeList->count());

  NDAttribute *cAttr = arr1->pAttributeList->next(nullptr);
  while (cAttr != nullptr) {
    ASSERT_NE(compAttrList.find(std::string(cAttr->getName())),
              compAttrList.end());
    const FB_Tables::NDAttribute *compAttr =
        compAttrList[std::string(cAttr->getName())];

    std::string descStr = std::string(compAttr->pDescription()->c_str());

    ASSERT_EQ(std::string(cAttr->getDescription()), descStr);

    std::string srcStr = std::string(compAttr->pSource()->c_str());

    ASSERT_EQ(std::string(cAttr->getSource()), srcStr);

    ASSERT_EQ(cAttr->getDataType(),
              NDArraySerializerStandIn::GetND_AttrDType(compAttr->dataType()));

    size_t dataSize1;
    NDAttrDataType_t dType1;
    NDAttrDataType_t dType2;
    size_t dataSize2;
    ASSERT_NE(cAttr->getValueInfo(&dType1, &dataSize1), ND_ERROR);

    dType2 = NDArraySerializerStandIn::GetND_AttrDType(compAttr->dataType());

    ASSERT_EQ(dType1, dType2);

    dataSize2 = compAttr->pData()->size();

    ASSERT_EQ(dataSize1, dataSize2);

    unsigned char *valuePtr1 = new unsigned char[dataSize1];
    cAttr->getValue(dType1, valuePtr1, dataSize1);

    const unsigned char *arr2ValuePtr = compAttr->pData()->data();

    ASSERT_EQ(std::memcmp(valuePtr1, arr2ValuePtr, dataSize1), 0);

    ASSERT_NE(valuePtr1, compAttr->pData()->data());

    delete[] valuePtr1;
    attrPtrs.emplace(compAttr);
    cAttr = arr1->pAttributeList->next(cAttr);
  }
  ASSERT_EQ(attrPtrs.size(), arr2->pAttributeList()->size());
}

void CompareAttributes(NDArray *arr1, NDArray *arr2) {
  ASSERT_EQ(arr1->pAttributeList->count(), arr2->pAttributeList->count());

  std::set<NDAttribute *> attrPtrs;

  NDAttribute *cAttr = arr1->pAttributeList->next(nullptr);
  NDAttribute *compAttr;
  while (cAttr != nullptr) {
    compAttr = arr2->pAttributeList->find(cAttr->getName());
    ASSERT_NE(nullptr, compAttr);

    ASSERT_EQ(std::string(cAttr->getDescription()),
              std::string(compAttr->getDescription()));

    ASSERT_EQ(std::string(cAttr->getSource()),
              std::string(compAttr->getSource()));

    ASSERT_EQ(cAttr->getDataType(), compAttr->getDataType());

    size_t dataSize1;
    NDAttrDataType_t dType1;
    size_t dataSize2;
    NDAttrDataType_t dType2;
    ASSERT_NE(cAttr->getValueInfo(&dType1, &dataSize1), ND_ERROR);

    ASSERT_NE(compAttr->getValueInfo(&dType2, &dataSize2), ND_ERROR);

    ASSERT_EQ(dType1, dType2);

    ASSERT_EQ(dataSize1, dataSize2);

    unsigned char *valuePtr1 = new unsigned char[dataSize1];
    unsigned char *valuePtr2 = new unsigned char[dataSize1];
    cAttr->getValue(dType1, valuePtr1, dataSize1);
    compAttr->getValue(dType2, valuePtr2, dataSize2);

    ASSERT_EQ(std::memcmp(valuePtr1, valuePtr2, dataSize1), 0);

    ASSERT_EQ(attrPtrs.find(compAttr), attrPtrs.end());

    ASSERT_NE(valuePtr1, valuePtr2);

    delete[] valuePtr1;
    delete[] valuePtr2;
    attrPtrs.emplace(compAttr);
    cAttr = arr1->pAttributeList->next(cAttr);
  }
  ASSERT_EQ(attrPtrs.size(), arr2->pAttributeList->count());
}
