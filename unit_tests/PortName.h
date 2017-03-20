/** Copyright (C) 2017 European Spallation Source */

/** @file  PortName.h
 *  @brief Header file for simple name generator.
 */

#pragma once

#include <string>

/** @brief Generates unique port names.
 * All asynPortDriver instances must have unique names. This function generates
 * names on the form
 * "port_name_X". Used by the unit test code.
 * @return A unique port name for use with asynDriver.
 */
std::string PortName();
