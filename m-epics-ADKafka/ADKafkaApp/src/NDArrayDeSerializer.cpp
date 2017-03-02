/** Copyright (C) 2017 European Spallation Source */

/** @file  NDArrayDeSerializer.cpp
 *  @brief Implementation of simple function which de-serializes EPICS NDArray
 * into a Flatbuffer.
 */

#include "NDArrayDeSerializer.h"
#include <ciso646>
#include <cstdlib>
#include <vector>
#include <cassert>

NDDataType_t GetND_DType(FB_Tables::DType arrType) {
    switch (arrType) {
    case FB_Tables::DType::DType_int8:
        return NDInt8;
    case FB_Tables::DType::DType_uint8:
        return NDUInt8;
    case FB_Tables::DType::DType_int16:
        return NDInt16;
    case FB_Tables::DType::DType_uint16:
        return NDUInt16;
    case FB_Tables::DType::DType_int32:
        return NDInt32;
    case FB_Tables::DType::DType_uint32:
        return NDUInt32;
    case FB_Tables::DType::DType_float32:
        return NDFloat32;
    case FB_Tables::DType::DType_float64:
        return NDFloat64;
    default:
        assert(false);
    }
    return NDInt8;
}

NDAttrDataType_t GetND_AttrDType(FB_Tables::DType attrType) {
    switch (attrType) {
    case FB_Tables::DType::DType_int8:
        return NDAttrInt8;
    case FB_Tables::DType::DType_uint8:
        return NDAttrUInt8;
    case FB_Tables::DType::DType_int16:
        return NDAttrInt16;
    case FB_Tables::DType::DType_uint16:
        return NDAttrUInt16;
    case FB_Tables::DType::DType_int32:
        return NDAttrInt32;
    case FB_Tables::DType::DType_uint32:
        return NDAttrUInt32;
    case FB_Tables::DType::DType_float32:
        return NDAttrFloat32;
    case FB_Tables::DType::DType_float64:
        return NDAttrFloat64;
    case FB_Tables::DType::DType_c_string:
        return NDAttrString;
    default:
        assert(false);
    }
    return NDAttrInt8;
}

size_t GetTypeSize(FB_Tables::DType attrType) {
    switch (attrType) {
    case FB_Tables::DType::DType_int8:
        return 1;
    case FB_Tables::DType::DType_uint8:
        return 1;
    case FB_Tables::DType::DType_int16:
        return 2;
    case FB_Tables::DType::DType_uint16:
        return 2;
    case FB_Tables::DType::DType_int32:
        return 4;
    case FB_Tables::DType::DType_uint32:
        return 4;
    case FB_Tables::DType::DType_float32:
        return 4;
    case FB_Tables::DType::DType_float64:
        return 8;
    case FB_Tables::DType::DType_c_string:
        return 1;
    default:
        assert(false);
    }
    return 1;
}

void DeSerializeData(NDArrayPool *pNDArrayPool, const unsigned char *bufferPtr, const size_t size,
                     NDArray *&pArray) {
    auto recvArr = FB_Tables::GetNDArray(bufferPtr);
    int id = recvArr->id();
    double timeStamp = recvArr->timeStamp();
    int EPICSsecPastEpoch = recvArr->epicsTS()->secPastEpoch();
    int nsec = recvArr->epicsTS()->nsec();
    std::vector<size_t> dims(recvArr->dims()->begin(), recvArr->dims()->end());
    NDDataType_t dataType = GetND_DType(recvArr->dataType());
    const void *pData = reinterpret_cast<const void*>(recvArr->pData()->Data());
    int pData_size = recvArr->pData()->size();

    pArray = pNDArrayPool->alloc(static_cast<int>(dims.size()), dims.data(), dataType, 0, nullptr);

    NDAttributeList *attrPtr = pArray->pAttributeList;
    attrPtr->clear();
    for (int i = 0; i < recvArr->pAttributeList()->size(); i++) {
        auto cAttr = recvArr->pAttributeList()->Get(i);
        attrPtr->add(new NDAttribute(cAttr->pName()->c_str(), cAttr->pDescription()->c_str(),
                                     NDAttrSourceDriver, cAttr->pSource()->c_str(),
                                     GetND_AttrDType(cAttr->dataType()),
                                     reinterpret_cast<void *>(const_cast<std::uint8_t*>(cAttr->pData()->Data()))));
    }

    std::memcpy(pArray->pData, pData, pData_size);

    pArray->uniqueId = id;
    pArray->timeStamp = timeStamp;
    pArray->epicsTS.secPastEpoch = EPICSsecPastEpoch;
    pArray->epicsTS.nsec = nsec;
}
