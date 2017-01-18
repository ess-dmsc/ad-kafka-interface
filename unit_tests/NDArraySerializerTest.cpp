//
//  NDArraySerializerTest.cpp
//  KafkaPlugin
//
//  Created by Jonas Nilsson on 2017-01-05.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <map>
#include <set>
#include <random>
#include "NDArraySerializer.h"

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

class Serializer : public ::testing::Test {
public:
    static void SetUpTestCase() {
        
    };
    
    static void TearDownTestCase() {
        
    };
    
    virtual void SetUp() {
        idCtr = 0;
        sendPool = new NDArrayPool(1, 0);
        recvPool = new NDArrayPool(1, 0);
        eng = std::default_random_engine(r());
    };
    
    virtual void TearDown() {
        delete sendPool;
        delete recvPool;
    };
    
    std::string RandomString(size_t length) {
        auto randchar = []() -> char {
            const char charset[] =
            "0123456789 "
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
            "abcdefghijklmnopqrstuvwxyz ";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[ rand() % max_index ];
        };
        std::string str(length,0);
        std::generate_n( str.begin(), length, randchar );
        if (usedAttrStrings.find(str) != usedAttrStrings.end()) {
            return RandomString(length);
        } else {
            usedAttrStrings.emplace(str);
        }
        return str;
    }
    
    template<typename T>
    void* GenerateAttrDataT(T min, T max) {
        if (std::is_same<T, std::double_t>::value or std::is_same<T, std::float_t>::value) {
            std::uniform_real_distribution<T> valueDist(min, max);
            return (void*) new T(valueDist(eng));
        }
        std::uniform_int_distribution<T> valueDist(min, max);
        return (void*) new T(valueDist(eng));
    }
    
    void* GenerateAttrData(NDAttrDataType_t type) {
        void * ptr = nullptr;
        std::default_random_engine e1(r());
        
        if (NDAttrString == type) {
            std::uniform_int_distribution<size_t> strLenDist(10, 1000);
            std::string tempStr = RandomString(strLenDist(e1));
            char *buffer = new char[tempStr.size() + 1];
            std::strncpy(buffer, tempStr.c_str(), tempStr.size() + 1);
            return (void*) buffer;
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
            assert(true);
        }
        return ptr;
    }
    
    template<typename T>
    void FreeAttrDataT(void *ptr) {
        T *tempPtr = (T*) ptr;
        delete tempPtr;
    }
    
    void FreeAttrData(void* ptr, NDAttrDataType_t type) {
        if (NDAttrString == type) {
            char *tempPtr = (char*) ptr;
            delete [] tempPtr;
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
            assert(true);
        }
    }
    
    NDAttribute* GenerateAttribute() {
        std::vector<size_t> attrStringLengths = {1, 10, 100, 1000};
        
        std::default_random_engine e1(r());
        std::uniform_int_distribution<size_t> strLenDist(0, attrStringLengths.size() - 1);
        
        std::string attrName = RandomString(attrStringLengths[strLenDist(e1)]);
        std::string attrDesc = RandomString(attrStringLengths[strLenDist(e1)]);
        std::string attrSource = RandomString(attrStringLengths[strLenDist(e1)]);
        
        std::vector<NDAttrDataType_t> attrDataTypes = {NDAttrUInt8, NDAttrInt8, NDAttrInt16, NDAttrUInt16, NDAttrUInt32, NDAttrInt32, NDAttrFloat32, NDAttrFloat64, NDAttrString};
        
        std::uniform_int_distribution<size_t> dataTypeDist(0, attrDataTypes.size() - 1);
        
        NDAttrDataType_t usedDType = attrDataTypes[dataTypeDist(e1)];
        
        
        void *attrDataPtr = GenerateAttrData(usedDType);
        NDAttribute *retAttr = new NDAttribute(attrName.c_str(), attrDesc.c_str(), NDAttrSourceDriver, attrSource.c_str(), usedDType, attrDataPtr);
        FreeAttrData(attrDataPtr, usedDType);
        return retAttr;
    }
    
    NDArray* GenerateNDArray(size_t numAttr, size_t numElem, int dims, NDDataType_t dType) {
        
        size_t elements = numElem;
        std::vector<size_t> usedDimensions;
        usedDimensions.push_back(numElem);
        for (int k = 1; k < dims; k++) {
            elements *= (numElem    + k * 2);
            usedDimensions.push_back(numElem + k * 2);
        }
        NDArray *pArr = sendPool->alloc(dims, usedDimensions.data(), dType, 0, NULL);
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
    
    std::default_random_engine eng;
    std::set<std::string> usedAttrStrings;
    std::random_device r;
    int idCtr;
    NDArrayPool *sendPool;
    NDArrayPool *recvPool;
};

TEST_F(Serializer, SerializeTest) {
    NDArraySerializer ser;
    std::vector<size_t> numAttr = {0, 1, 10};
    std::vector<size_t> numElements = {1, 10, 50};
    std::vector<NDDataType_t> dataTypes = {NDUInt8, NDInt8, NDUInt16, NDInt16, NDInt32, NDUInt32, NDFloat32, NDFloat64};
    std::vector<int> numDims = {1, 2, 3, 4};
    NDArray *sendArr = nullptr;
    for (auto nAttr : numAttr) {
        for (auto nElem : numElements  ) {
            for (auto dType : dataTypes) {
                for (auto nDim : numDims) {
                    sendArr = GenerateNDArray(nAttr, nElem, nDim, dType);
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
                    usedAttrStrings.clear();
                }
            }
        }
    }
    delete sendArr;
}

TEST_F(Serializer, SerializeDeserializeTest) {
    NDArraySerializer ser;
    std::vector<size_t> numAttr = {0, 1, 10};
    std::vector<size_t> numElements = {1, 10, 50};
    std::vector<NDDataType_t> dataTypes = {NDUInt8, NDInt8, NDUInt16, NDInt16, NDInt32, NDUInt32, NDFloat32, NDFloat64};
    std::vector<int> numDims = {1, 2, 3, 4};
    NDArray *sendArr = nullptr;
    NDArray *recvArr = nullptr;
    for (auto nAttr : numAttr) {
        for (auto nElem : numElements  ) {
            for (auto dType : dataTypes) {
                for (auto nDim : numDims) {
                    sendArr = GenerateNDArray(nAttr, nElem, nDim, dType);
                    unsigned char *bufferPtr = nullptr;
                    size_t bufferSize;
                    ser.SerializeData(*sendArr, bufferPtr, bufferSize);
                    ser.DeSerializeData(recvArr, recvPool, bufferPtr);
                    CompareDataTypes(sendArr, recvArr);
                    CompareSizeAndDims(sendArr, recvArr);
                    CompareTimeStamps(sendArr, recvArr);
                    CompareData(sendArr, recvArr);
                    CompareAttributes(sendArr, recvArr);
                    sendArr->release();
                    recvArr->release();
                    usedAttrStrings.clear();
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
    ASSERT_EQ(arr1->dataType, NDArraySerializerStandIn::GetND_DType(arr2->dataType()));
}

void CompareSizeAndDims(NDArray *arr1, NDArray *arr2) {
    std::map<NDDataType_t, int> sizeList = {
        {NDInt8, 1},
        {NDUInt8, 1},
        {NDInt16, 2},
        {NDUInt16, 2},
        {NDInt32, 4},
        {NDUInt32, 4},
        {NDFloat32, 4},
        {NDFloat64, 8},
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
        {NDInt8, 1},
        {NDUInt8, 1},
        {NDInt16, 2},
        {NDUInt16, 2},
        {NDInt32, 4},
        {NDUInt32, 4},
        {NDFloat32, 4},
        {NDFloat64, 8},
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
    
    //memcmp returns 0 if the data is the same, something else otherwise
    ASSERT_EQ(std::memcmp(arr1->pData, arr2->pData, arr1Info.totalBytes), 0);
}

void CompareData(NDArray *arr1, const FB_Tables::NDArray *arr2) {
    NDArrayInfo_t arr1Info;
    arr1->getInfo(&arr1Info);
    
    //memcmp returns 0 if the data is the same, something else otherwise
    ASSERT_EQ(std::memcmp(arr1->pData, arr2->pData()->data(), arr1Info.totalBytes), 0);
}

void CompareAttributes(NDArray *arr1, const FB_Tables::NDArray *arr2) {
    ASSERT_EQ(arr1->pAttributeList->count(), arr2->pAttributeList()->size());

    std::set<const FB_Tables::NDAttribute*> attrPtrs;
    
    
    std::map<std::string,const FB_Tables::NDAttribute*> compAttrList;
    for (int u = 0; u < arr2->pAttributeList()->size(); u++) {
        const FB_Tables::NDAttribute *attPtr = arr2->pAttributeList()->Get(u);
        compAttrList[attPtr->pName()->str()] = attPtr;
    }
    ASSERT_EQ(compAttrList.size(), arr1->pAttributeList->count());
    
    NDAttribute *cAttr = arr1->pAttributeList->next(NULL);
    while (cAttr != NULL) {
        ASSERT_NE(compAttrList.find(std::string(cAttr->getName())), compAttrList.end());
        const FB_Tables::NDAttribute *compAttr = compAttrList[std::string(cAttr->getName())];
        
        std::string descStr = std::string(compAttr->pDescription()->c_str());
        
        ASSERT_EQ(std::string(cAttr->getDescription()), descStr);
        
        std::string srcStr = std::string(compAttr->pSource()->c_str());
        
        ASSERT_EQ(std::string(cAttr->getSource()), srcStr);
        
        ASSERT_EQ(cAttr->getDataType(), NDArraySerializerStandIn::GetND_AttrDType(compAttr->dataType()));
        
        size_t dataSize1;
        NDAttrDataType_t dType1;
        NDAttrDataType_t dType2;
        size_t dataSize2;
        ASSERT_NE(cAttr->getValueInfo(&dType1, &dataSize1), ND_ERROR);
        
        dType2 = NDArraySerializerStandIn::GetND_AttrDType(compAttr->dataType());
        
        ASSERT_EQ(dType1, dType2);
        
        dataSize2 = compAttr->pData()->size();
        
        ASSERT_EQ(dataSize1, dataSize2);
        
        unsigned char* valuePtr1 = new unsigned char[dataSize1];
        cAttr->getValue(dType1, valuePtr1, dataSize1);
        
        const unsigned char* arr2ValuePtr = compAttr->pData()->data();
        
        ASSERT_EQ(std::memcmp(valuePtr1, arr2ValuePtr, dataSize1), 0);
        
        ASSERT_NE(valuePtr1, compAttr->pData()->data());
        
        delete [] valuePtr1;
        attrPtrs.emplace(compAttr);
        cAttr = arr1->pAttributeList->next(cAttr);
    }
    ASSERT_EQ(attrPtrs.size(), arr2->pAttributeList()->size());
}

void CompareAttributes(NDArray *arr1, NDArray *arr2) {
    ASSERT_EQ(arr1->pAttributeList->count(), arr2->pAttributeList->count());
    
    std::set<NDAttribute*> attrPtrs;
    
    NDAttribute *cAttr = arr1->pAttributeList->next(NULL);
    NDAttribute *compAttr;
    while (cAttr != NULL) {
        compAttr = arr2->pAttributeList->find(cAttr->getName());
        ASSERT_NE(NULL, long(compAttr));
        
        ASSERT_EQ(std::string(cAttr->getDescription()), std::string(compAttr->getDescription()));
        
        ASSERT_EQ(std::string(cAttr->getSource()), std::string(compAttr->getSource()));
        
        ASSERT_EQ(cAttr->getDataType(), compAttr->getDataType());
        
        size_t dataSize1;
        NDAttrDataType_t dType1;
        size_t dataSize2;
        NDAttrDataType_t dType2;
        ASSERT_NE(cAttr->getValueInfo(&dType1, &dataSize1), ND_ERROR);
        
        ASSERT_NE(compAttr->getValueInfo(&dType2, &dataSize2), ND_ERROR);
        
        ASSERT_EQ(dType1, dType2);
        
        ASSERT_EQ(dataSize1, dataSize2);
        
        unsigned char* valuePtr1 = new unsigned char[dataSize1];
        unsigned char* valuePtr2 = new unsigned char[dataSize1];
        cAttr->getValue(dType1, valuePtr1, dataSize1);
        compAttr->getValue(dType2, valuePtr2, dataSize2);
        
        ASSERT_EQ(std::memcmp(valuePtr1, valuePtr2, dataSize1), 0);
        
        ASSERT_EQ(attrPtrs.find(compAttr), attrPtrs.end());
        
        ASSERT_NE(valuePtr1, valuePtr2);
        
        delete [] valuePtr1;
        delete [] valuePtr2;
        attrPtrs.emplace(compAttr);
        cAttr = arr1->pAttributeList->next(cAttr);
    }
    ASSERT_EQ(attrPtrs.size(), arr2->pAttributeList->count());
}

template <typename T>
void PopulateArr(size_t elements, void *ptr) {
    T *arr = (T*)ptr;
    for (int y = 0; y < elements; y++) {
        arr[y] = (T)y;
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
    } else  if (NDFloat32 == type) {
        PopulateArr<std::float_t>(elements, usedPtr);
    } else  if (NDFloat64 == type) {
        PopulateArr<std::double_t>(elements, usedPtr);
    } else {
        assert(false);
    }
}
