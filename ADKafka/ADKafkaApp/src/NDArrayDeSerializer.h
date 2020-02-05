/** Copyright (C) 2017 European Spallation Source */

/** @file  NDArrayDeSerializer.h
 *  @brief Deserialisation of NDArray data using Google Flatbuffer.
 */

#pragma once

#include "NDArray_schema_generated.h"
#include <NDArray.h>

/** @brief Deserializes NDArray data previously serialized by flatbuffers.
 * The deserialization requires that a NDArrayPool provides a NDArray instance
 * to which the data can
 * be copied. The function currently does no checks to ensure that there is
 * memory available. This
 * should probably be rectified and taken care of here.
 * @param[in] pNDArrayPool A pointer to the NDArrayPool which is used to
 * allocate NDArray which
 * will store the data in the buffer.
 * @param[in] bufferPtr Pointer to the buffer containing the data which is to be
 * deserialized.
 * @param[in] size Size of the data in bytes. Note that this value is not used
 * and that no checks
 * are made for overflows.
 * @param[out] pArray The pointer to the NDArray containing the deserialized
 * data. Note that the
 * caller has ownership of the pointer and must thus call NDArray::release()
 * when the array is no
 * longer needed.
 */
void DeSerializeData(NDArrayPool *pNDArrayPool, const unsigned char *bufferPtr,
                     NDArray *&pArray);
