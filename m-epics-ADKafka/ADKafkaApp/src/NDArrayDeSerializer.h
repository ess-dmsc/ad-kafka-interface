/** Copyright (C) 2017 European Spallation Source */

/** @file  NDArraySerializer.h
 *  @brief Serialisation of NDArray data using Google Flatbuffer.
 */
#pragma once

#include "NDArray_schema_generated.h"
#include <NDArray.h>

void DeSerializeData(NDArray *&pArray, NDArrayPool *pNDArrayPool, unsigned char *bufferPtr,
                     const size_t length);
