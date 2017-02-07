/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaPlugin.h
 *  @brief Header file for the C++ implementation of an EPICS areaDetector Kafka-plugin.
 */

#pragma once

#include <epicsTypes.h>
#include <string>

#include <NDPluginDriver.h>
#include <map>
#include "KafkaProducer.h"
#include "NDArraySerializer.h"
#include "ParamUtility.h"

using namespace KafkaInterface;
/** @brief areaDetector plugin that produces Kafka messages and sends them to a broker.
 * This class is an areaDetector plugin which can be used to transmit data from an
 * areaDetector to a Kafka broker. The data is packed in a flatbuffer.
 */
class epicsShareClass KafkaPlugin : public NDPluginDriver {
  public:
	/** @brief Constructor for the KafkaPlugin.
	 * Paramaters are passed straight to the parent class. The parameter documentation is taken
     * straight from the documentation of NDPluginDriver::NDPluginDriver().
	 * @param[in] portName The name of the asyn port driver to be created.
	 * @param[in] queueSize The number of NDArrays that the input queue for this plugin can hold when 
	 *            NDPluginDriverBlockingCallbacks=0.  Larger queues can decrease the number of dropped arrays,
	 *            at the expense of more NDArray buffers being allocated from the underlying driver's NDArrayPool.
	 * @param[in] blockingCallbacks Initial setting for the NDPluginDriverBlockingCallbacks flag.
	 *            0=callbacks are queued and executed by the callback thread; 1 callbacks execute in the thread
	 *            of the driver doing the callbacks.
	 * @param[in] NDArrayPort Name of asyn port driver for initial source of NDArray callbacks.
	 * @param[in] NDArrayAddr asyn port driver address for initial source of NDArray callbacks.
	 * @param[in] maxAddr The maximum  number of asyn addr addresses this driver supports. 1 is minimum.
	 * @param[in] maxBuffers The maximum number of NDArray buffers that the NDArrayPool for this driver is 
	 *            allowed to allocate. Set this to -1 to allow an unlimited number of buffers.
	 * @param[in] maxMemory The maximum amount of memory that the NDArrayPool for this driver is 
	 *            allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
	 * @param[in] interfaceMask Bit mask defining the asyn interfaces that this driver supports.
	 * @param[in] interruptMask Bit mask definining the asyn interfaces that can generate interrupts (callbacks)
	 * @param[in] asynFlags Flags when creating the asyn port driver; includes ASYN_CANBLOCK and ASYN_MULTIDEVICE.
	 * @param[in] autoConnect The autoConnect flag for the asyn port driver.
	 * @param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
	 * @param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
	 */
    KafkaPlugin(const char *portName, int queueSize, int blockingCallbacks, const char *NDArrayPort,
                int NDArrayAddr, size_t maxMemory, int priority, int stackSize, const char *brokerAddress, const char *brokerTopic);
				
    /** @brief Destructor.
	 * Calls KafkaPlugin::DestroyKafkaConnection().
				*/
    ~KafkaPlugin();
    
    /** @brief Called when new data from the areaDetector is available.
     * Based on a implementation in one of the standard plugins. Calls KafkaPlugin::SendKafkaPacket().
     * This member function will throw away packets if the Kafka queue is full!
     * @param[in] pArray  The NDArray from the callback.
     */
    void processCallbacks(NDArray *pArray);
    
    /** @brief Used to set the string parameters of the Kafka producer.
     * If a configuration string is updated, the Kafka prdoucer will be immediatly destroyed and
     * re-created using the new parameter. This means that unsent data will be lost.
     * Based on code used in one of the standard plugins. Parameter description is taken from the
     * documentation of NDPluginDriver::writeOctet().
     * @param[in] pasynUser pasynUser structure that encodes the reason and address.
     * @param[in] value Address of the string to write.
     * @param[in] nChars Number of characters to write.
     * @param[out] nActual Number of characters actually written.
     */
    asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);
    
    
    asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

  protected:
    static const int intMask = asynInt32ArrayMask | asynOctetMask | asynGenericPointerMask;
    int MIN_PARAM_INDEX;
    KafkaProducer prod;
    NDArraySerializer serializer;
    
    enum PV {
        kafka_addr,
        kafka_topic,
        stats_time,
        queue_size,
        count,
    };
    
    std::vector<PV_param> paramsList = {
        PV_param("KAFKA_BROKER_ADDRESS", asynParamOctet), //kafka_addr
        PV_param("KAFKA_TOPIC", asynParamOctet), //kafka_topic
        PV_param("KAFKA_STATS_INT_MS", asynParamInt32), //stats_time
        PV_param("KAFKA_QUEUE_SIZE", asynParamInt32), //queue_size
    };
};
