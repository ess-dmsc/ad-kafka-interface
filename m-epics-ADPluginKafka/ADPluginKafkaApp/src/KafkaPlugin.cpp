/** Copyright (C) 2016 European Spallation Source */

/** @file  KafkaPlugin.cpp
 *  @brief C++ implementation file for an EPICS areaDetector Kafka-plugin.
 */


#include <epicsEvent.h>
#include <epicsMessageQueue.h>
#include <epicsThread.h>
#include <epicsTime.h>
#include <iocsh.h>

#include <asynDriver.h>
#include <epicsExport.h>

#include "KafkaPlugin.h"

static const char *driverName = "KafkaPlugin";

void KafkaPlugin::processCallbacks(NDArray *pArray) {
    //@todo Check the order of these calls and if all of them are needed.
    NDArrayInfo_t arrayInfo;
    asynStandardInterfaces *pInterfaces = this->getAsynStdInterfaces();
    
    NDPluginDriver::processCallbacks(pArray);
    
    pArray->getInfo(&arrayInfo);
    
    this->unlock();
    unsigned char *bufferPtr;
    size_t bufferSize;
    serializer.SerializeData(*pArray, bufferPtr, bufferSize);
    prod.SendKafkaPacket(bufferPtr, bufferSize);
    
    this->lock();
    //@todo I need to check what is happening here
    if (this->pArrays[0])
        this->pArrays[0]->release();
    pArray->reserve();
    this->pArrays[0] = pArray;
    callParamCallbacks();
}

asynStatus KafkaPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars,
                                   size_t *nActual) {
    int addr = 0;
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *functionName = "writeOctet";
    
    status = getAddress(pasynUser, &addr);
    if (status != asynSuccess)
        return (status);
    
    // Set the parameter in the parameter library.
    
    //-------- This line here is wrong!!!!!!
    //status = setParam(this, paramList.at("addr"), std::string(value));
    
    //status = (asynStatus)setStringParam(addr, function, (char *)value);
    if (status != asynSuccess)
        return (status);
    
    std::string tempStr;
    // If a new Kafka paramater is received, destroy and re-create everything
    if (function == *paramsList.at(PV::kafka_addr).index) {
        tempStr = std::string(value, nChars);
        prod.SetBrokerAddr(tempStr);
    } else if (function == *paramsList.at(PV::kafka_topic).index) {
        tempStr = std::string(value, nChars);
        prod.SetTopic(tempStr);
    } else if (function < MIN_PARAM_INDEX) {
        // If this parameter belongs to a base class call its method
        status = NDPluginDriver::writeOctet(pasynUser, value, nChars, nActual);
    }
    
    // Do callbacks so higher layers see any changes
    callParamCallbacks(addr, addr);
    
    //@todo Part of the EPICS message logging system, should be expanded or removed
    if (status) {
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
                      "%s:%s: status=%d, function=%d, value=%s", driverName, functionName, status,
                      function, value);
    } else {
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, value=%s\n", driverName,
                  functionName, function, value);
    }
    
    // We are assuming that we wrote as many characters as we received
    *nActual = nChars;
    return status;
}

KafkaPlugin::KafkaPlugin(const char *portName, int queueSize, int blockingCallbacks,
                         const char *NDArrayPort, int NDArrayAddr, size_t maxMemory, int priority,
                         int stackSize)
// Invoke the base class constructor
: NDPluginDriver(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr, 1,
                 PV::count + KafkaProducer::GetNumberOfPVs(), 2, maxMemory,
                 
                 asynInt8ArrayMask | asynInt16ArrayMask | asynInt32ArrayMask |
                 asynFloat32ArrayMask | asynFloat64ArrayMask,
                 
                 asynInt8ArrayMask | asynInt16ArrayMask | asynInt32ArrayMask |
                 asynFloat32ArrayMask | asynFloat64ArrayMask,
                 0, 1, priority, stackSize) {
    
    MIN_PARAM_INDEX = InitPvParams(this, paramsList);
    InitPvParams(this, prod.GetParams());
    
    
    setStringParam(NDPluginDriverPluginType, "KafkaPlugin");
    setParam(this, paramsList.at(PV::kafka_addr), "");
    setParam(this, paramsList.at(PV::kafka_topic), "");
    
    // Disable ArrayCallbacks.
    // This plugin currently does not do array callbacks, so make the setting
    // reflect the behavior
    setIntegerParam(NDArrayCallbacks, 0);
    
    /* Try to connect to the NDArray port */
    connectToArrayPort();
}

KafkaPlugin::~KafkaPlugin() { }

// Configuration routine.  Called directly, or from the iocsh function
extern "C" int KafkaPluginConfigure(const char *portName, int queueSize, int blockingCallbacks,
                                    const char *NDArrayPort, int NDArrayAddr, size_t maxMemory,
                                    int priority, int stackSize) {
    KafkaPlugin *pPlugin = new KafkaPlugin(portName, queueSize, blockingCallbacks, NDArrayPort,
                                           NDArrayAddr, maxMemory, priority, stackSize);
    
    return (asynSuccess);
    return pPlugin->start();
}

// EPICS iocsh shell commands
static const iocshArg initArg0 = {"portName", iocshArgString};
static const iocshArg initArg1 = {"frame queue size", iocshArgInt};
static const iocshArg initArg2 = {"blocking callbacks", iocshArgInt};
static const iocshArg initArg3 = {"NDArrayPort", iocshArgString};
static const iocshArg initArg4 = {"NDArrayAddr", iocshArgInt};
static const iocshArg initArg5 = {"maxMemory", iocshArgInt};
static const iocshArg initArg6 = {"priority", iocshArgInt};
static const iocshArg initArg7 = {"stack size", iocshArgInt};
static const iocshArg *const initArgs[] = {&initArg0, &initArg1, &initArg2, &initArg3,
    &initArg4, &initArg5, &initArg6, &initArg7};
static const iocshFuncDef initFuncDef = {"KafkaPluginConfigure", 8, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
    KafkaPluginConfigure(args[0].sval, args[1].ival, args[2].ival, args[3].sval, args[4].ival,
                         args[5].ival, args[6].ival, args[7].ival);
}

extern "C" void KafkaPluginReg(void) { iocshRegister(&initFuncDef, initCallFunc); }

extern "C" {
    epicsExportRegistrar(KafkaPluginReg);
}
