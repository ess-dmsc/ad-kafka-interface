/** Copyright (C) 2017 European Spallation Source */

/** @file  NDArraySerializer.h
 *  @brief Serialisation of NDArray data using Google Flatbuffer.
 */
#pragma once

#include "NDArray_schema_generated.h"
#include <NDArray.h>

/** @brief Class which is used to serialize NDArray data using flatbuffers.
 * The C++ flatbuffers implementatione has an internal buffer for storing the
 * serialized data. Thus
 * it only stores the data for one serialized NDArray. If more are required,
 * copy the data or use
 * several instances of this class.
 */
class NDArraySerializer {
public:
  /** @brief Initialize the flatbuffer builder with a given buffer size.
   * The default buffer size given here is 1MB though. If the buffer is to small
   * to store the
   * serialized data it will be automatically expanded to an appropriate size.
   * In order to
   * minimise the number of deallocations and allocations of memory, provide an
   * buffer size.
   * @param[in] bufferSize Size of flatbuffer buffer in bytes. Will be increase
   * if the data does
   * not fit.
   */
  explicit NDArraySerializer(const flatbuffers::uoffset_t bufferSize = 1048576);

  /** @brief Serializes data held in the input NDArray.
   * Note that the returned pointer is only valid until next time
   * NDArraySerializer::SerializeData() is called!
   * @param[in] pArray The data to be serialized.
   * @param[out] bufferPtr The pointer to the serialized data. Note that this
   * pointer is only
   * valid until the member function is called again.
   * @param[out] bufferSize Size of serialized data in bytes.
   */
  void SerializeData(NDArray &pArray, unsigned char *&bufferPtr,
                     size_t &bufferSize);

protected:
  /** @brief Used to convert from areaDetector data type to flatbuffer data
   * type.
   * Could be a function only available in the implementation file but is used
   * by the unit tests
   * and thus has to be made available here.
   * @param[in] arrType areaDetector data type.
   * @return flatbuffer data type.
   */
  static FB_Tables::DType GetFB_DType(NDDataType_t arrType);

  /** @brief Used to convert from flatbuffer data type to areaDetector data
   * type.
   * Could be a function only available in the implementation file but is used
   * by the unit tests
   * and thus has to be made available here.
   * @param[in] arrType areaDetector data type.
   * @return NDArray data type.
   */
  static NDDataType_t GetND_DType(FB_Tables::DType arrType);

  /** @brief Used to convert from areaDetector attribute data type to flatbuffer
   * data type.
   * This is used by the algorithm that packs attributes into the flatbuffer.
   * Could be a function
   * only available in the implementation file but is used by the unit tests and
   * thus has to be
   * made available here.
   * @param[in] attrType areaDetector attribute data type.
   * @return flatbuffer data type.
   */
  static FB_Tables::DType GetFB_DType(NDAttrDataType_t attrType);

  /** @brief Used to convert from flatbuffer attribute data type to areaDetector
   * data type.
   * Not strictly necessary but is used by the unit tests and thus is made
   * available here.
   * @param[in] attrType flatbuffer attribute data type.
   * @return areaDetector data type.
   */
  static NDAttrDataType_t GetND_AttrDType(FB_Tables::DType attrType);

private:
  /// @brief The flatbuffer builder which serializes the data.
  flatbuffers::FlatBufferBuilder builder;
};
