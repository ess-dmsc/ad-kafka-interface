/** Copyright (C) 2017 European Spallation Source */

/** @file  NDArraySerializer.h
 *  @brief Serialisation of NDArray data using Google Flatbuffer.
 */
#pragma once

#include "NDArray_schema_generated.h"
#include <NDArray.h>

void DeSerializeData(NDArray *&pArray, NDArrayPool *pNDArrayPool, unsigned char *bufferPtr);

static FB_Tables::DType GetFB_DType(NDDataType_t arrType);
    
static NDDataType_t GetND_DType(FB_Tables::DType arrType);
    
/** @brief Used to convert from areaDetector attribute data type to flatbuffer data type.
* This is used by the algorithm that packs attributes into the flatbuffer.
* @param[in] attrType areaDetector attribute data type.
* @return flatbuffer data type.
*/
static FB_Tables::DType GetFB_DType(NDAttrDataType_t attrType);
    
static NDAttrDataType_t GetND_AttrDType(FB_Tables::DType attrType);
