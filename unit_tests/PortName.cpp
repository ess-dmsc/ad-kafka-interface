/** Copyright (C) 2017 European Spallation Source */

/** @file  PortName.cpp
 *  @brief Implementation file for simple name generator.
 */

#include "PortName.h"

std::string PortName() {
  static int portNameCtr = 0;
  portNameCtr++;
  return std::string("port_name_") + std::to_string(portNameCtr);
}
