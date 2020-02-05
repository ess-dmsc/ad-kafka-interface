/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaPlugin.h
 *  @brief Header file for the C++ implementation of an EPICS areaDetector
 * Kafka-plugin.
 */

#pragma once

#include <epicsTypes.h>
#include <string>

#include "KafkaProducer.h"
#include "NDArraySerializer.h"
#include "ParamUtility.h"
#include <NDPluginDriver.h>
#include <map>

using namespace KafkaInterface;
/** @brief areaDetector plugin that produces Kafka messages and sends them to a
 * broker.
 * This class is an areaDetector plugin which can be used to transmit data from
 * an
 * areaDetector to a Kafka broker. The data is packed in a flatbuffer.
 */
class epicsShareClass KafkaPlugin : public NDPluginDriver {
public:
  /** @brief Used to initialize KafkaPlugin.
   * Configures the plugin, initializes PV:s and starts the Kafka producer part.
   * @param[in] portName The name of the asyn port driver to be created. Can be
   * used to chain
   * plugins.
   * @param[in] queueSize The number of NDArrays that the input queue for this
   * plugin can hold
   * when NDPluginDriverBlockingCallbacks=0.  Larger queues can decrease the
   * number of
   * dropped arrays, at the expense of more NDArray buffers being allocated from
   * the underlying
   * driver's NDArrayPool.
   * @param[in] blockingCallbacks Initial setting for the
   * NDPluginDriverBlockingCallbacks flag.
   * 0=callbacks are queued and executed by the callback thread; 1 callbacks
   * execute in the thread
   * of the driver doing the callbacks.
   * @param[in] NDArrayPort Name of asyn port driver for initial source of
   * NDArray callbacks.
   * @param[in] NDArrayAddr asyn port driver address for initial source of
   * NDArray callbacks.
   * @param[in] maxMemory The maximum amount of memory that the NDArrayPool for
   * this driver is
   * allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
   * @param[in] priority The thread priority for the asyn port driver thread if
   * ASYN_CANBLOCK is
   * set in asynFlags.
   * @param[in] stackSize The stack size for the asyn port driver thread if
   * ASYN_CANBLOCK is set
   * in asynFlags.
   * @param[in] brokerAddress The address of the Kafka broker in the form
   * "address:port". Can take
   * several addresses seperated by a comma (e.g.
   * "address1:port1,address2:port2").
   * @param[in] brokerTopic Topic from which the driver should consume messages.
   * Note that only
   * one topic can be specified.
   */
  KafkaPlugin(const char *portName, int queueSize, int blockingCallbacks,
              const char *NDArrayPort, int NDArrayAddr, size_t maxMemory,
              int priority, int stackSize, const char *brokerAddress,
              const char *brokerTopic);

  /// @brief Destructor, currently empty.
  ~KafkaPlugin() = default;

  /** @brief Called when new data from the areaDetector is available.
   * Based on a implementation in one of the standard plugins. Calls
   * KafkaPlugin::SendKafkaPacket().
   * This member function will throw away packets if the Kafka queue is full!
   * @param[in] pArray The NDArray from the callback.
   */
  void processCallbacks(NDArray *pArray);

  /** @brief Used to set the string parameters of the Kafka producer.
   * If a configuration string is updated, the Kafka prdoucer will be immediatly
   * destroyed and
   * re-created using the new parameter. This means that unsent data will be
   * lost.
   * @param[in] pasynUser pasynUser structure that encodes the reason and
   * address.
   * @param[in] value Address of the string to write.
   * @param[in] nChars Number of characters to write.
   * @param[out] nActual Number of characters actually written.
   * @return asynStatus value corresponding to the success of setting a new
   * value.
   */
  asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars,
                        size_t *nActual);

  /** @brief Used to set integer paramters of the plugin.
   * @param[in] pasynUser pasynUser structure that encodes the reason and
   * address.
   * @param[in] value The new value of the paramter.
   * @return asynStatus value corresponding to the success of setting a new
   * value.
   */
  asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

protected:
  /** @brief Interrupt mask passed to NDPluginDriver.
   * @todo What does the interrupt mask actually do?
   */
  static const int intMask{asynInt32ArrayMask | asynOctetMask |
                           asynGenericPointerMask};

  /** @brief Used to keep track of the lowest PV index in order to know which
   * write events should
   * be passed to the parent class.
   */
  int MIN_PARAM_INDEX;

  /// @brief The kafka producer which is used to send serialized NDArray data to
  /// the broker.
  KafkaProducer producer;

  /// @brief The class instance used to serialize NDArray data.
  NDArraySerializer serializer;

  /// @brief Used to keep track of the PV:s made available by this driver.
  enum PV {
    kafka_addr,
    kafka_topic,
    stats_time,
    queue_size,
    count,
  };

  /// @brief The list of PV:s created by the driver and their definition.
  std::vector<PV_param> paramsList = {
      PV_param("KAFKA_BROKER_ADDRESS", asynParamOctet), // kafka_addr
      PV_param("KAFKA_TOPIC", asynParamOctet),          // kafka_topic
      PV_param("KAFKA_STATS_INT_MS", asynParamInt32),   // stats_time
      PV_param("KAFKA_QUEUE_SIZE", asynParamInt32),     // queue_size
  };
};
