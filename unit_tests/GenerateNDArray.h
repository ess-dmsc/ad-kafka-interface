//
//  GenerateNDArray.hpp
//  KafkaInterface
//
//  Created by Jonas Nilsson on 2017-01-26.
//
//

#pragma once

#include <set>
#include <random>
#include <NDPluginDriver.h>

class NDArrayGenerator {
public:
    NDArrayGenerator();
    ~NDArrayGenerator();
    
    NDArray* GenerateNDArray(size_t numAttr, size_t numElem, int dims, NDDataType_t dType);
    std::set<std::string> usedAttrStrings;
private:
    std::string RandomString(size_t length);
    void* GenerateAttrData(NDAttrDataType_t type);
    
    template<typename T>
    void FreeAttrDataT(void *ptr) {
        T *tempPtr = (T*) ptr;
        delete tempPtr;
    }
    
    template<typename T>
    void* GenerateAttrDataT(T min, T max) {
        if (std::is_same<T, std::double_t>::value or std::is_same<T, std::float_t>::value) {
            std::uniform_real_distribution<> valueDist(min, max);
            return (void*) new T(valueDist(eng));
        }
        std::uniform_int_distribution<> valueDist(min, max);
        return (void*) new T(valueDist(eng));
    }
    
    void FreeAttrData(void* ptr, NDAttrDataType_t type);
    NDAttribute* GenerateAttribute();
    
    std::default_random_engine eng;
    std::random_device r;
    int idCtr;
    NDArrayPool *sendPool;
};
