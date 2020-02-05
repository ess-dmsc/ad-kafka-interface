/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaPlugin.cpp
 *  @brief C++ implementation file for an EPICS areaDetector Kafka-plugin.
 */

#include <epicsEvent.h>
#include <epicsMessageQueue.h>
#include <epicsThread.h>
#include <epicsTime.h>
#include <iocsh.h>

#include <asynDriver.h>
#include <ciso646>
#include <epicsExport.h>

#include "KafkaPlugin.h"

static const char *driverName = "KafkaPlugin";

void KafkaPlugin::processCallbacks(NDArray *pArray) {
  // We do not need to call reserve/release as this is done by the caller when
  // in blocking mode
  // and by the thread in non-blocking mode.
  /// @todo Check the order of these calls and if all of them are needed.
  NDArrayInfo_t arrayInfo;

  NDPluginDriver::beginProcessCallbacks(pArray);

  pArray->getInfo(&arrayInfo);

  unsigned char *bufferPtr;
  size_t bufferSize;

  serializer.SerializeData(*pArray, bufferPtr, bufferSize);
  this->unlock();
  bool addToQueueSuccess = producer.SendKafkaPacket(bufferPtr, bufferSize);
  this->lock();
  if (not addToQueueSuccess) {
    int droppedArrays;
    getIntegerParam(NDPluginDriverDroppedArrays, &droppedArrays);
    droppedArrays++;
    setIntegerParam(NDPluginDriverDroppedArrays, droppedArrays);
  }
  callParamCallbacks();
}

asynStatus KafkaPlugin::writeOctet(asynUser *pasynUser, const char *value,
                                   size_t nChars, size_t *nActual) {
  int addr = 0;
  int function{pasynUser->reason};
  asynStatus status{asynSuccess};
  const char *functionName = "writeOctet";

  status = getAddress(pasynUser, &addr);
  if (status != asynSuccess) {
    return (status);
  }

  /* Set the parameter in the parameter library. */
  setStringParam(addr, function, const_cast<char *>(value));

  std::string tempStr;
  if (function == *paramsList.at(PV::kafka_addr).index) {
    tempStr = std::string(value, nChars);
    producer.SetBrokerAddr(tempStr);
  } else if (function == *paramsList.at(PV::kafka_topic).index) {
    tempStr = std::string(value, nChars);
    producer.SetTopic(tempStr);
  } else if (function < MIN_PARAM_INDEX) {
    NDPluginDriver::writeOctet(pasynUser, value, nChars, nActual);
  }

  // Do callbacks so higher layers see any changes
  status = callParamCallbacks(addr, addr);

  if (status != 0) {
    epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
                  "%s:%s: status=%d, function=%d, value=%s", driverName,
                  functionName, status, function, value);
  } else {
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, value=%s\n",
              driverName, functionName, function, value);
  }

  // We are assuming that we wrote as many characters as we received
  *nActual = nChars;
  return status;
}

asynStatus KafkaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value) {
  const int function{pasynUser->reason};
  asynStatus status{asynSuccess};
  static const char *functionName = "writeInt32";

  /* Set the parameter in the parameter library. */
  setIntegerParam(function, value);

  if (function == *paramsList[stats_time].index) {
    producer.SetStatsTimeMS(value);
  } else if (function == *paramsList[queue_size].index) {
    producer.SetMessageQueueLength(value);
  } else {
    /* If this parameter belongs to a base class call its method */
    if (function < MIN_PARAM_INDEX) {
      NDPluginDriver::writeInt32(pasynUser, value);
    }
  }

  /* Do callbacks so higher layers see any changes */
  status = callParamCallbacks();

  if (status != 0) {
    epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
                  "%s:%s: status=%d, function=%d, value=%d", driverName,
                  functionName, status, function, value);
  } else {
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, value=%d\n",
              driverName, functionName, function, value);
  }
  return status;
}

KafkaPlugin::KafkaPlugin(const char *portName, int queueSize,
                         int blockingCallbacks, const char *NDArrayPort,
                         int NDArrayAddr, size_t maxMemory, int priority,
                         int stackSize, const char *brokerAddress,
                         const char *brokerTopic)
    // Invoke the base class constructor
    : NDPluginDriver(portName, queueSize, blockingCallbacks, NDArrayPort,
                     NDArrayAddr, 1, 2, maxMemory, intMask, intMask, 0, 1,
                     priority, stackSize, 1),
      producer(brokerAddress, brokerTopic) {

  MIN_PARAM_INDEX = InitPvParams(this, paramsList);

  // The following three calls must be made in this particular order
  InitPvParams(this, producer.GetParams());
  producer.RegisterParamCallbackClass(this);
  producer.StartThread();

  setStringParam(NDPluginDriverPluginType, "KafkaPlugin");
  setParam(this, paramsList.at(PV::kafka_addr), brokerAddress);
  setParam(this, paramsList.at(PV::kafka_topic), brokerTopic);
  setParam(this, paramsList.at(PV::stats_time), producer.GetStatsTimeMS());
  setParam(this, paramsList.at(PV::queue_size),
           producer.GetMessageQueueLength());

  // Disable ArrayCallbacks.
  // This plugin currently does not do array callbacks, so make the setting
  // reflect the behavior
  setIntegerParam(NDArrayCallbacks, 0);

  /* Try to connect to the NDArray port */
  connectToArrayPort();
}

// Configuration routine.  Called directly, or from the iocsh function
extern "C" int KafkaPluginConfigure(const char *portName, int queueSize,
                                    int blockingCallbacks,
                                    const char *NDArrayPort, int NDArrayAddr,
                                    size_t maxMemory, const char *brokerAddress,
                                    const char *topic) {
  auto *pPlugin =
      new KafkaPlugin(portName, queueSize, blockingCallbacks, NDArrayPort,
                      NDArrayAddr, maxMemory, 0, 0, brokerAddress, topic);

  return pPlugin->start();
}

// EPICS iocsh shell commands
static const iocshArg initArg0 = {"portName", iocshArgString};
static const iocshArg initArg1 = {"frame queue size", iocshArgInt};
static const iocshArg initArg2 = {"blocking callbacks", iocshArgInt};
static const iocshArg initArg3 = {"NDArrayPort", iocshArgString};
static const iocshArg initArg4 = {"NDArrayAddr", iocshArgInt};
static const iocshArg initArg5 = {"maxMemory", iocshArgInt};
// static const iocshArg initArg6 = {"priority", iocshArgInt};
// static const iocshArg initArg7 = {"stack size", iocshArgInt};
static const iocshArg initArg8 = {"broker address", iocshArgString};
static const iocshArg initArg9 = {"topic", iocshArgString};
// static const iocshArg *const initArgs[] = {&initArg0, &initArg1, &initArg2,
// &initArg3,
//    &initArg4, &initArg5, &initArg6, &initArg7, &initArg8, &initArg9};
static const iocshArg *const initArgs[] = {&initArg0, &initArg1, &initArg2,
                                           &initArg3, &initArg4, &initArg5,
                                           &initArg8, &initArg9};
static const iocshFuncDef initFuncDef = {"KafkaPluginConfigure", 8, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
  KafkaPluginConfigure(args[0].sval, args[1].ival, args[2].ival, args[3].sval,
                       args[4].ival, args[5].ival, args[6].sval, args[7].sval);
}

extern "C" void KafkaPluginReg(void) {
  iocshRegister(&initFuncDef, initCallFunc);
}

extern "C" {
epicsExportRegistrar(KafkaPluginReg);
}
