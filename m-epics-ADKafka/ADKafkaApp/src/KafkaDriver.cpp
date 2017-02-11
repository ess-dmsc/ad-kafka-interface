/** Copyright (C) 2016 European Spallation Source */

/** @file  KafkaPlugin.cpp
 *  @brief C++ implementation file for an EPICS areaDetector Kafka-plugin.
 */

#include <epicsEvent.h>
#include <epicsMessageQueue.h>
#include <epicsThread.h>
#include <epicsTime.h>
#include <iocsh.h>

#include "KafkaDriver.h"
#include "NDArrayDeSerializer.h"
#include <asynDriver.h>
#include <ciso646>
#include <epicsExport.h>

static const char *driverName = "KafkaDriver";

asynStatus KafkaDriver::writeOctet(asynUser *pasynUser, const char *value, size_t nChars,
                                   size_t *nActual) {
    int addr = 0;
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *functionName = "writeOctet";

    status = getAddress(pasynUser, &addr);
    if (status != asynSuccess)
        return (status);

    /* Set the parameter in the parameter library. */
    status = (asynStatus)setStringParam(addr, function, (char *)value);

    if (function == *paramsList.at(PV::kafka_addr).index) {
        consumer.SetBrokerAddr(std::string(value, nChars));
    } else if (function == *paramsList.at(PV::kafka_topic).index) {
        consumer.SetTopic(std::string(value, nChars));
    } else if (function == *paramsList.at(PV::kafka_group).index) {
        consumer.SetGroupId(std::string(value, nChars));
    } else if (function < MIN_PARAM_INDEX) {
        status = ADDriver::writeOctet(pasynUser, value, nChars, nActual);
    }

    // Do callbacks so higher layers see any changes
    status = (asynStatus)callParamCallbacks(addr, addr);

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

asynStatus KafkaDriver::writeInt32(asynUser *pasynUser, epicsInt32 value) {
    int function = pasynUser->reason;
    int adstatus;
    int acquiring;
    int imageMode;
    asynStatus status = asynSuccess;

    /* Ensure that ADStatus is set correctly before we set ADAcquire.*/
    getIntegerParam(ADStatus, &adstatus);
    getIntegerParam(ADAcquire, &acquiring);
    getIntegerParam(ADImageMode, &imageMode);
    if (function == ADAcquire) {
        if (value && !acquiring) {
            setStringParam(ADStatusMessage, "Acquiring data");
        }
        if (!value && acquiring) {
            setStringParam(ADStatusMessage, "Acquisition stopped");
            if (imageMode == ADImageContinuous) {
                setIntegerParam(ADStatus, ADStatusIdle);
            } else {
                setIntegerParam(ADStatus, ADStatusAborted);
            }
            setIntegerParam(ADStatus, ADStatusAcquire);
        }
    }
    callParamCallbacks();

    for (auto pv : consumer.GetParams()) {
        if (*pv.index == function) {
            if ("KAFKA_CURRENT_OFFSET" == pv.desc) {
                consumer.SetOffset(value);
                break;
            }
        }
    }

    if (function == *paramsList[set_offset].index) {
        int cOffsetSetting;
        getIntegerParam(*paramsList[set_offset].index, &cOffsetSetting);
        if (value >= 0 and value <= 3) {
            if (KafkaDriver::Beginning == value) {
                consumer.SetOffset(RdKafka::Topic::OFFSET_BEGINNING);
            } else if (KafkaDriver::End == value) {
                consumer.SetOffset(RdKafka::Topic::OFFSET_END);
            } else if (KafkaDriver::Stored == value) {
                consumer.SetOffset(RdKafka::Topic::OFFSET_STORED);
            } else if (KafkaDriver::Manual == value) {
                int cOffsetValue;
                getIntegerParam(consumer.GetOffsetPVIndex(), &cOffsetValue);
                consumer.SetOffset(cOffsetValue);
            }
            usedOffsetSetting = OffsetSetting(value);
        } else {
            value = cOffsetSetting;
        }
    } else if (function == consumer.GetOffsetPVIndex()) {
        if (KafkaDriver::Manual == usedOffsetSetting) {
            consumer.SetOffset(value);
        }
    } else if (function == *paramsList[stats_time].index) {
        if (value > 0) {
            consumer.SetStatsTimeMS(value);
        }
    }
    /* Set the parameter and readback in the parameter library.  This may be overwritten when we
     * read back the
     * status at the end, but that's OK */
    status = setIntegerParam(function, value);

    /* For a real detector this is where the parameter is sent to the hardware */
    if (function == ADAcquire) {
        if (value && !acquiring) {
            /* Send an event to wake up the consumer task.
             * It won't actually start generating new images until we release the lock below */
            epicsEventSignal(startEventId_);
        }
        if (!value && acquiring) {
            /* This was a command to stop acquisition */
            /* Send the stop event */
            epicsEventSignal(stopEventId_);
        }
    } else {
        /* If this parameter belongs to a base class call its method */
        if (function < MIN_PARAM_INDEX)
            status = ADDriver::writeInt32(pasynUser, value);
    }

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();

    if (status)
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "%s:writeInt32 error, status=%d function=%d, value=%d\n", driverName, status,
                  function, value);
    else
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:writeInt32: function=%d, value=%d\n",
                  driverName, function, value);
    return status;
}

static void consumeTaskC(void *drvPvt) {
    KafkaDriver *pPvt = (KafkaDriver *)drvPvt;

    pPvt->consumeTask();
}

KafkaDriver::KafkaDriver(const char *portName, int maxBuffers, size_t maxMemory, int priority,
                         int stackSize, const char *brokerAddress, const char *brokerTopic)
    // Invoke the base class constructor
    : ADDriver(portName, 1, consumer.GetNumberOfPVs() + PV::count, maxBuffers, maxMemory, 0,
               0,    /* No interfaces beyond those set in ADDriver.cpp */
               0, 1, /* ASYN_CANBLOCK=0, ASYN_MULTIDEVICE=0, autoConnect=1 */
               priority, stackSize),
      consumer(brokerAddress, brokerTopic) {

    const char *functionName = "KafkaDriver";
    int status = asynStatus::asynSuccess;
    usedOffsetSetting = OffsetSetting::Stored;
    startEventId_ = epicsEventCreate(epicsEventEmpty);
    if (!startEventId_) {
        printf("%s:%s epicsEventCreate failure for start event\n", driverName, functionName);
        return;
    }
    stopEventId_ = epicsEventCreate(epicsEventEmpty);
    if (!stopEventId_) {
        printf("%s:%s epicsEventCreate failure for stop event\n", driverName, functionName);
        return;
    }

    threadExitEventId_ = epicsEventCreate(epicsEventEmpty);
    if (!threadExitEventId_) {
        printf("%s:%s epicsEventCreate failure for stop event\n", driverName, functionName);
        return;
    }

    MIN_PARAM_INDEX = InitPvParams(this, paramsList);

    // The following three calls must be made in this particular order
    InitPvParams(this, consumer.GetParams());
    consumer.RegisterParamCallbackClass(this);

    status = setParam(this, paramsList.at(PV::kafka_addr), brokerAddress);
    status |= setParam(this, paramsList.at(PV::kafka_topic), brokerTopic);
    status |= setParam(this, paramsList.at(PV::kafka_group), consumer.GetGroupId());
    status |= setParam(this, paramsList.at(PV::stats_time), consumer.GetStatsTimeMS());
    status |= setParam(this, paramsList.at(PV::set_offset), usedOffsetSetting);

    // Disable ArrayCallbacks.
    // This plugin currently does not do array callbacks, so make the setting
    // reflect the behavior
    setIntegerParam(NDArrayCallbacks, 0);

    if (status) {
        printf("%s: unable to set camera parameters\n", functionName);
        return;
    }

    /* Create the thread that updates the images */
    status = (epicsThreadCreate("ConsumeKafkaMsgsTask", epicsThreadPriorityMedium,
                                epicsThreadGetStackSize(epicsThreadStackMedium),
                                (EPICSTHREADFUNC)consumeTaskC, this) == NULL);
    if (status) {
        printf("%s:%s epicsThreadCreate failure for image task\n", driverName, functionName);
        return;
    }
}

void KafkaDriver::consumeTask() {
    int status = asynSuccess;
    int numImages, numImagesCounter;
    int imageMode;
    int arrayCallbacks;
    int acquire = 0;
    NDArray *pImage = nullptr;
    double acquirePeriod;
    const char *functionName = "consumeTask";
    double startWaitTimeout;
    keepThreadAlive = true;
    this->lock();
    /* Loop forever */
    while (keepThreadAlive) {
        /* If we are not acquiring then wait for a semaphore that is given when acquisition is
         * started */
        if (!acquire) {
            /* Release the lock while we wait for an event that says acquire has started, then lock
             * again */
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s:%s: waiting for acquire to start\n",
                      driverName, functionName);
            this->unlock();
            status = asynStatus::asynTimeout;
            startWaitTimeout = consumer.GetStatsTimeMS() / 1000.0;
            //            status = epicsEventWait(startEventId_);
            consumer.StopConsumption();
            while (status == asynStatus::asynTimeout) {
                status = epicsEventWaitWithTimeout(startEventId_, startWaitTimeout);
                if (not keepThreadAlive) {
                    goto exitConsumeTaskLabel;
                }
                KafkaInterface::KafkaMessage *fbImg = consumer.WaitForPkg(0);
                if (fbImg != nullptr) {
                    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                              "%s:%s: Got Kafka msg when none should be received.\n", driverName,
                              functionName);
                    std::abort(); // This should never happen
                }
            }
            consumer.StartConsumption();
            this->lock();
            acquire = 1;
            setStringParam(ADStatusMessage, "Acquiring data");
            setIntegerParam(ADNumImagesCounter, 0);
        }

        /* We are acquiring. */
        /* Get the current time */
        getIntegerParam(ADImageMode, &imageMode);

        setIntegerParam(ADStatus, ADStatusAcquire);

        /* Open the shutter */
        setShutter(ADShutterOpen);

        /* Call the callbacks to update any changes */
        callParamCallbacks();

        // Did we get a stop signal?
        status = epicsEventTryWait(stopEventId_);
        if (status == epicsEventWaitOK) {
            acquire = 0;
            if (imageMode == ADImageContinuous) {
                setIntegerParam(ADStatus, ADStatusIdle);
            } else {
                setIntegerParam(ADStatus, ADStatusAborted);
            }
            callParamCallbacks();
        }

        /* Update the image */
        getDoubleParam(ADAcquirePeriod, &acquirePeriod);
        this->unlock();
        KafkaInterface::KafkaMessage *fbImg = consumer.WaitForPkg(int(acquirePeriod * 1000));
        this->lock();

        // If we get no image, go to start of loop
        if (nullptr == fbImg)
            continue;

        // Copy data to an NDArray
        if (pImage) {
            pImage->release();
        }
        DeSerializeData(pImage, this->pNDArrayPool, (unsigned char *)fbImg->GetDataPtr(),
                        fbImg->size());
        delete fbImg;
        fbImg = nullptr;

        /* Close the shutter */
        setShutter(ADShutterClosed);

        // Make it possible to exit the loop again.
        if (!acquire)
            continue;

        setIntegerParam(ADStatus, ADStatusReadout);
        /* Call the callbacks to update any changes */
        callParamCallbacks();

        /* Get the current parameters */
        setIntegerParam(NDArrayCounter, pImage->uniqueId);

        getIntegerParam(ADNumImagesCounter, &numImagesCounter);
        numImagesCounter++;
        setIntegerParam(ADNumImagesCounter, numImagesCounter);

        // If callbacks are active, do them
        getIntegerParam(NDArrayCallbacks, &arrayCallbacks);
        if (arrayCallbacks) {
            /* Call the NDArray callback */
            /* Must release the lock here, or we can get into a deadlock, because we can
             * block on the plugin lock, and the plugin can be calling us */
            this->unlock();
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s:%s: calling imageData callback\n",
                      driverName, functionName);
            doCallbacksGenericPointer(pImage, NDArrayData, 0);
            this->lock();
        }

        /* See if acquisition is done */
        getIntegerParam(ADNumImages, &numImages);
        if ((imageMode == ADImageSingle) ||
            ((imageMode == ADImageMultiple) && (numImagesCounter >= numImages))) {

            /* First do callback on ADStatus. */
            setStringParam(ADStatusMessage, "Waiting for acquisition");
            setIntegerParam(ADStatus, ADStatusIdle);
            callParamCallbacks();

            acquire = 0;
            consumer.StopConsumption();
            setIntegerParam(ADAcquire, acquire);
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s:%s: acquisition completed\n",
                      driverName, functionName);
        }

        /* Call the callbacks to update any changes */
        callParamCallbacks();

        /* If we are acquiring then sleep for the acquire period minus elapsed time. */
        if (acquire) {
            setIntegerParam(ADStatus, ADStatusWaiting);
            callParamCallbacks();
            status = epicsEventTryWait(stopEventId_);
            if (status == epicsEventWaitOK) {
                acquire = 0;
                consumer.StopConsumption();
                if (imageMode == ADImageContinuous) {
                    setIntegerParam(ADStatus, ADStatusIdle);
                } else {
                    setIntegerParam(ADStatus, ADStatusAborted);
                }
                callParamCallbacks();
            }
        }
    }
    this->unlock();
exitConsumeTaskLabel:
    epicsEventSignal(threadExitEventId_);
}

KafkaDriver::~KafkaDriver() {
    keepThreadAlive = false;
    epicsEventSignal(startEventId_);
    epicsEventWait(threadExitEventId_);

    epicsEventDestroy(startEventId_);
    epicsEventDestroy(stopEventId_);
    epicsEventDestroy(threadExitEventId_);
}

// Configuration routine.  Called directly, or from the iocsh function
extern "C" int KafkaDriverConfigure(const char *portName, int maxBuffers, size_t maxMemory,
                                    int priority, int stackSize, const char *brokerAddrStr,
                                    const char *topicName) {
    KafkaDriver *pDriver = new KafkaDriver(portName, maxBuffers, maxMemory, priority, stackSize,
                                           brokerAddrStr, topicName);

    return (asynSuccess);
}

// EPICS iocsh shell commands
static const iocshArg initArg0 = {"portName", iocshArgString};
static const iocshArg initArg1 = {"maxBuffers", iocshArgInt};
static const iocshArg initArg2 = {"maxMemory", iocshArgInt};
static const iocshArg initArg3 = {"priority", iocshArgInt};
static const iocshArg initArg4 = {"stackSize", iocshArgInt};
static const iocshArg initArg5 = {"broker address", iocshArgString};
static const iocshArg initArg6 = {"broker topic", iocshArgString};
static const iocshArg *const initArgs[] = {&initArg0, &initArg1, &initArg2, &initArg3,
                                           &initArg4, &initArg5, &initArg6};
static const iocshFuncDef initFuncDef = {"KafkaDriverConfigure", 7, initArgs};

static void initCallFunc(const iocshArgBuf *args) {
    KafkaDriverConfigure(args[0].sval, args[1].ival, args[2].ival, args[3].ival, args[4].ival,
                         args[5].sval, args[6].sval);
}

extern "C" void KafkaDriverReg(void) { iocshRegister(&initFuncDef, initCallFunc); }

extern "C" {
epicsExportRegistrar(KafkaDriverReg);
}
