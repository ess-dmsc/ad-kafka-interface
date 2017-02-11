//
//  PortName.cpp
//  KafkaInterface
//
//  Created by Jonas Nilsson on 2017-02-06.
//
//

#include "PortName.h"

std::string PortName() {
  static int portNameCtr = 0;
  portNameCtr++;
  return std::string("port_name_") + std::to_string(portNameCtr);
}
