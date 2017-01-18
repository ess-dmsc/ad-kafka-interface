//
//  NDArraySerializer.hpp
//  KafkaPlugin
//
//  Created by Jonas Nilsson on 2017-01-05.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#pragma once

#include "NDArray_schema_generated.h"
#include <NDArray.h>

class NDArraySerializer {
public:
    NDArraySerializer();
    
    /** @brief Serializes data held in the input NDArray.
     * Note that the returned pointer is only valid until next time
     * SerializeData or DeserializeData is called!
     * @param[in] pArray The data from the areaDetector.
     * @param[out] bufferPtr The pointer which has the location of the
     * serialized data. Note that this pointer is only valid until the class
     * instance is called again.
     * @param[out] bufferSize Size of serialized data in bytes.
     */
    void SerializeData(NDArray &pArray, unsigned char *&bufferPtr, size_t &bufferSize);
    
    void DeSerializeData(NDArray *&pArray, NDArrayPool *pNDArrayPool, unsigned char *bufferPtr);
    
    void ReleaseSerializedData();
protected:
    /** @brief Used to convert from areaDetector data type to flatbuffer data type.
     * @param[in] arrType areaDetector data type.
     * @return flatbuffer data type.
     */
    static FB_Tables::DType GetFB_DType(NDDataType_t arrType);
    
    static NDDataType_t GetND_DType(FB_Tables::DType arrType);
    
    /** @brief Used to convert from areaDetector attribute data type to flatbuffer data type.
     * This is used by the algorithm that packs attributes into the flatbuffer.
     * @param[in] attrType areaDetector attribute data type.
     * @return flatbuffer data type.
     */
    static FB_Tables::DType GetFB_DType(NDAttrDataType_t attrType);
    
    static NDAttrDataType_t GetND_AttrDType(FB_Tables::DType attrType);
    
private:
    const int FB_builder_buffer = 1048576;
    
    //The flat buffer builder
    flatbuffers::FlatBufferBuilder builder;
};
