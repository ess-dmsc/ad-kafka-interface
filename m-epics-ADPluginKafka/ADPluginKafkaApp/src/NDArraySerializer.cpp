/** Copyright (C) 2017 European Spallation Source */

/** @file  NDArraySerializer.cpp
 *  @brief Implementation of simple class which serializes EPICS NDArray into a
 * Flatbuffer.
 */

#include "NDArraySerializer.h"
#include <ciso646>
#include <memory>
#include <vector>
#include <cassert>

NDArraySerializer::NDArraySerializer(const flatbuffers::uoffset_t bufferSize)
    : builder(bufferSize) {}

void NDArraySerializer::SerializeData(NDArray &pArray, unsigned char *&bufferPtr,
                                      size_t &bufferSize) {
    NDArrayInfo ndInfo;
    pArray.getInfo(&ndInfo);

    // Required to not have a memory leak
    builder.Clear();

    auto epics_ts = FB_Tables::epicsTimeStamp(pArray.epicsTS.secPastEpoch, pArray.epicsTS.nsec);
    std::vector<std::uint64_t> tempDims;
    for (size_t y = 0; y < pArray.ndims; y++) {
        tempDims.push_back(pArray.dims[y].size);
    }
    auto dims = builder.CreateVector(tempDims);
    auto dType = GetFB_DType(pArray.dataType);

    std::uint8_t *tempPtr;
    auto payload = builder.CreateUninitializedVector(ndInfo.totalBytes, 1, &tempPtr);
    std::memcpy(tempPtr, pArray.pData, ndInfo.totalBytes);

    // Get all attributes of this data package
    std::vector<flatbuffers::Offset<FB_Tables::NDAttribute>> attrVec;

    // When passing NULL, get first element
    NDAttribute *attr_ptr = pArray.pAttributeList->next(nullptr);

    // Itterate over attributes, next(ptr) returns NULL when there are no more
    while (attr_ptr != nullptr) {
        auto temp_attr_str = builder.CreateString(attr_ptr->getName());
        auto temp_attr_desc = builder.CreateString(attr_ptr->getDescription());
        auto temp_attr_src = builder.CreateString(attr_ptr->getSource());
        size_t bytes;
        NDAttrDataType_t c_type;
        attr_ptr->getValueInfo(&c_type, &bytes);
        auto attrDType = GetFB_DType(c_type);

        std::unique_ptr<char[]> attrValueBuffer(new char[bytes]);
        int attrValueRes = attr_ptr->getValue(c_type, reinterpret_cast<void*>(attrValueBuffer.get()), bytes);
        if (ND_SUCCESS == attrValueRes) {
            auto attrValuePayload =
                builder.CreateVector(reinterpret_cast<unsigned char*>(attrValueBuffer.get()), bytes);

            auto attr = FB_Tables::CreateNDAttribute(builder, temp_attr_str, temp_attr_desc,
                                                     temp_attr_src, attrDType, attrValuePayload);
            attrVec.push_back(attr);
        } else {
            assert(false);
        }

        attr_ptr = pArray.pAttributeList->next(attr_ptr);
    }
    auto attributes = builder.CreateVector(attrVec);
    auto kf_pkg = FB_Tables::CreateNDArray(builder, pArray.uniqueId, pArray.timeStamp, &epics_ts,
                                           dims, dType, payload, attributes);

    // Write data to buffer
    builder.Finish(kf_pkg);

    bufferPtr = builder.GetBufferPointer();
    bufferSize = builder.GetSize();
}

FB_Tables::DType NDArraySerializer::GetFB_DType(NDDataType_t arrType) {
    switch (arrType) {
    case NDInt8:
        return FB_Tables::DType::DType_int8;
    case NDUInt8:
        return FB_Tables::DType::DType_uint8;
    case NDInt16:
        return FB_Tables::DType::DType_int16;
    case NDUInt16:
        return FB_Tables::DType::DType_uint16;
    case NDInt32:
        return FB_Tables::DType::DType_int32;
    case NDUInt32:
        return FB_Tables::DType::DType_uint32;
    case NDFloat32:
        return FB_Tables::DType::DType_float32;
    case NDFloat64:
        return FB_Tables::DType::DType_float64;
    default:
        assert(false);
    }
    return FB_Tables::DType::DType_int8;
}

NDDataType_t NDArraySerializer::GetND_DType(FB_Tables::DType arrType) {
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

FB_Tables::DType NDArraySerializer::GetFB_DType(NDAttrDataType_t attrType) {
    switch (attrType) {
    case NDAttrInt8:
        return FB_Tables::DType::DType_int8;
    case NDAttrUInt8:
        return FB_Tables::DType::DType_uint8;
    case NDAttrInt16:
        return FB_Tables::DType::DType_int16;
    case NDAttrUInt16:
        return FB_Tables::DType::DType_uint16;
    case NDAttrInt32:
        return FB_Tables::DType::DType_int32;
    case NDAttrUInt32:
        return FB_Tables::DType::DType_uint32;
    case NDAttrFloat32:
        return FB_Tables::DType::DType_float32;
    case NDAttrFloat64:
        return FB_Tables::DType::DType_float64;
    case NDAttrString:
        return FB_Tables::DType::DType_c_string;
    default:
        assert(false);
    }
    return FB_Tables::DType::DType_int8;
}

NDAttrDataType_t NDArraySerializer::GetND_AttrDType(FB_Tables::DType attrType) {
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
