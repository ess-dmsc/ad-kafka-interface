/** Copyright (C) 2017 European Spallation Source */

/** @file
 *  @brief Some helper functions and a PV-struct which simplifies the handling
 * of PV:s.
 */

#pragma once

#include <asynNDArrayDriver.h>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

/** @brief A simple class for collection all the relevant information about a PV
 * in a single
 * location.
 */
class PV_param {
public:
  /** @brief A simple constructor which sets the appropriate member variables
   * using the input
   * paramters.
   * @param[in] desc The std::string which is used to connect between the PV
   * index and the entry
   * in the database template file.
   * @param[in] type The data type of PV in question.
   * @param[in] index The index value of the PV.
   */
  PV_param(std::string const &desc, asynParamType type, int index = 0)
      : desc(desc), type(type), index(std::make_shared<int>(index)){};

  /** @brief An empty constructor for PV_param. It is required by some parts of
   * the code.
   * This constructor is required by some parts of the code though its use is
   * minimised in order
   * decrease the probability of bugs. Will set the PV description to "Not used"
   * which aids in
   * debugging.
   */
  PV_param()
      : desc("Not used"), type(asynParamType::asynParamNotDefined),
        index(nullptr){};

  /// @brief Connection between index and database template name.
  const std::string desc;

  /// @brief Data type of the PV.
  const asynParamType type;

  /// @brief The index of the PV, must be a std::shared_ptr<> in order to
  std::shared_ptr<int> index;
};

/** @brief Utility function for initializing PVs in the database.
 * This function is written as a template in order to make it reusable without
 * forcing the developer
 * to cast to asynPortDriver. Could potentially be improved so that it only
 * accepts types inherited
 * from asynPortDriver. Sets the index of the PV_param objects passed to it.
 * @param[in] driverPtr Pointer to the instance of the class which calls this
 * function. Must be
 * pointer to type which inherits from asynPortDriver though this is currently
 * not enforced.
 * @param[in] paramList List of PV_param which is to be initialized. When the
 * function returns the
 * index of these should be set to something other than 0.
 * @return The minimum index set to the PV_param objects passed to the function.
 * This minimum value
 * is used to keep track of which PV index should be handled by a parent class.
 */
template <class asynNDArrType>
int InitPvParams(asynNDArrType *driverPtr, std::vector<PV_param> &paramList) {
  int minParamIndex = -1;
  for (auto &p : paramList) {
    driverPtr->createParam(p.desc.c_str(), p.type, p.index.get());
    if (-1 == minParamIndex) {
      minParamIndex = *p.index;
    } else if (minParamIndex > *p.index) {
      minParamIndex = *p.index;
    }
  }
  return minParamIndex;
}

/** @brief Overloaded function used to set PV string values.
 * Implemented as a template in order to minimise casting. Should probably be
 * modified to only
 * accept types which inherits from asynPortDriver. Note that if the type of the
 * PV is not
 * asynParamOctet this function will call std::arbort() which quits the
 * application. This might not
 * be the best choice.
 * @param[in] driverPtr Pointer to the instance of the class which calls this
 * function. Must be
 * pointer to type which inherits from asynPortDriver though this is currently
 * not enforced.
 * @param[in] param Has the relevant PV information for updating the value in
 * the PV database.
 * @param[in] value The new value of the PV.
 * @return The result of setting the parameter in the form of
 * asynPortDriver::asynStatus.
 */
template <class asynNDArrType>
asynStatus setParam(asynNDArrType *driverPtr, const PV_param &param,
                    std::string const &value) {
  if (nullptr == driverPtr or 0 == *param.index) {
    return asynStatus::asynError;
  }
  asynStatus retStatus;
  if (asynParamOctet == param.type) {
    retStatus = driverPtr->setStringParam(*param.index, value.c_str());
  } else {
    std::abort();
  }
  return retStatus;
}

/** @brief Overloaded function used to set PV integer values.
 * Implemented as a template in order to minimise casting. Should probably be
 * modified to only
 * accept types which inherits from asynPortDriver. Note that if the type of the
 * PV is not
 * asynParamInt32 this function will call std::arbort() which quits the
 * application. This might not
 * be the best choice.
 * @param[in] driverPtr Pointer to the instance of the class which calls this
 * function. Must be
 * pointer to type which inherits from asynPortDriver though this is currently
 * not enforced.
 * @param[in] param Has the relevant PV information for updating the value in
 * the PV database.
 * @param[in] value The new value of the PV.
 * @return The result of setting the parameter in the form of
 * asynPortDriver::asynStatus.
 */
template <typename asynNDArrType>
asynStatus setParam(asynNDArrType *driverPtr, const PV_param &param,
                    const int value) {
  if (nullptr == driverPtr or 0 == *param.index) {
    return asynStatus::asynError;
  }
  asynStatus retStatus;
  if (asynParamInt32 == param.type) {
    retStatus = driverPtr->setIntegerParam(*param.index, value);
  } else {
    std::abort();
  }
  return retStatus;
}
