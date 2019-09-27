/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaDriver.h
 *  @brief Header file for the C++ implementation of an EPICS areaDetector
 * Kafka-plugin.
 */

#pragma once

#include <ADDriver.h>
#include <atomic>
#include <epicsEvent.h>
#include <map>
#include <string>

#include "KafkaConsumer.h"
#include "ParamUtility.h"

using KafkaInterface::KafkaConsumer;

/** @brief An EPICS areaDetector driver which consumes Kafka messages containing
 * NDArray data.
 * This class implements the interface and some of the logic required by an
 * EPICS areaDetector
 * driver which
 * consumes Kafka messages containing NDArray data serialized using flatbuffers.
 * In order to
 * simplify developement, including unit testing, the Kafka communication code
 * is implemented in
 * the class KafkaInterface::KafkaConsumer().
 */
class epicsShareClass KafkaDriver : public ADDriver {
public:
  /** @brief Instantiates the areaDetector driver.
   * Configures the driver and starts the consumer thread. Note that consumption
   * of messages is
   * not started but must be done seperately using the appropriate PV.
   * @param[in] portName Name of the asyn port driver to be created. Can be used
   * to connect a
   * plugin to that particular detector.
   * @param[in] maxBuffers The maximum number of NDArray buffers that the
   * NDArrayPool used by the
   * driver will allocate. There will be no limitation if this is set to 0
   * though this is not
   * recommended a bug in the driver or a plugin can make the driver consume all
   * the available
   * RAM.
   * @param[in] maxMemory The maximum amount of memory (in bytes) that the
   * NDArrayPool will
   * allocate for data in all the NDArray objects in the pool. Setting this to 0
   * will allow an
   * unlimited amount of data. Can be used instead of or in conjunction with the
   * previous
   * parameter in order to limit the maximum amount of ram used by the driver.
   * @param[in] priority The thread priority for the asyn port driver thread if
   * ASYN_CANBLOCK is
   * set in asynFlags. If it is 0 then the default value of
   * epicsThreadPriorityMedium will be
   * assigned by asynManager.
   * @param[in] stackSize The stack size for the asyn port driver thread if
   * ASYN_CANBLOCK is set
   * in asynFlags. If it is 0 then the default value of
   * epicsThreadGetStackSize(epicsThreadStackMedium) will be assigned by
   * asynManager.
   * @param[in] brokerAddress The address of the Kafka broker in the form
   * "address:port". Can take
   * several addresses seperated by a comma (e.g.
   * "address1:port1,address2:port2").
   * @param[in] brokerTopic Topic from which the driver should consume messages.
   * Note that only
   * one topic can be specified.
   */
  KafkaDriver(const char *portName, int maxBuffers, size_t maxMemory,
              int priority, int stackSize, const char *brokerAddress,
              const char *brokerTopic);

  /** @brief Shuts down consumer thread and deallocates dynamically allocated
   * resources which are
   * not deallocated automatically.
   * The destructor will set a boolean which is used by the processing thread to
   * determine if it
   * should be running. It then waits for an event from the thread which is
   * created when it
   * it actually exits.
   * @note If the destructor is called before the thread has actually started
   * the event will never
   * be created which will cause a deadlock. This problem should only be visible
   * during unit
   * testing.
   * @todo Fix the constructor and destructor to actually keep track of the
   * state of the thread
   * in order to ensure that no deadlock is possible.
   */
  ~KafkaDriver();

  /** @brief Used to set the string parameters of the Kafka consumer.
   * When called, the underlying implementation will try to keep the connection
   * to the broker if
   * possible. However, if the broker address is changed the current connection
   * must be destroyed.
   * @note Due to the use of writeOctet to set configuration strings, these
   * strings are limited in
   * length to 40 characters.
   * @todo Possibly, change from writeOctet to writeInt8Array in order to make
   * it possible to have
   * longer configuration strings.
   * @param[in] pasynUser pasynUser structure that encodes the reason and
   * address.
   * @param[in] value Address of the string to write.
   * @param[in] nChars Number of characters to write.
   * @param[out] nActual Number of characters actually written.
   */
  virtual asynStatus writeOctet(asynUser *pasynUser, const char *value,
                                size_t nChars, size_t *nActual);

  /** @brief Used to set integer parameters of the Kafka consumer.
   * Implements the setting of integer parameters as well as some logic for
   * doing this. The actual
   * parameters available and honored by the driver are documented in the
   * README.md file of this
   * EPICS module.
   * @todo This member function could potentially use some cleanup.
   * @param[in] pasynUser pasynUser structure that encodes the reason and
   * address.
   * @param[in] value New integer value to use.
   */
  virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

  /** @brief The thread function which does the heavy lifting in this driver.
   * This function uses an endless loop to consume NDArray messages. Should be
   * protected/private
   * but must be public due to the use of the C language to start it. As the
   * main thread can not
   * keep track of the processing thread directly, communication between it and
   * the main thread
   * is done by the use of events (see KafkaDriver::~KafkaDriver()).
   */
  virtual void consumeTask();

protected:
  /** @brief Used to keep track of the lowest PV index in order to know which
   * write events should
   * be passed to the parent class.
   */
  int MIN_PARAM_INDEX;

  /** @brief Implements all communication with the Kafka brokers.
   */
  KafkaConsumer consumer;

  /// @brief Used to pass a start acquisition event from writeInt32 to the
  /// processing thread.
  epicsEventId startEventId_;
  /// @brief Used to pass a stop acquisition event from writeInt32 to the
  /// processing thread.
  epicsEventId stopEventId_;
  /** @brief Used to pass an event from the processing thread to the thread
   * calling the destructor
   * when the proccesing thread has exited.
   */
  epicsEventId threadExitEventId_;

  /// @brief Used to keep track of the PV:s made available by this driver.
  enum PV {
    kafka_addr,
    kafka_topic,
    kafka_group,
    stats_time,
    set_offset,
    count,
  };

  /// @brief Defines possible Kafka message offset settings.
  enum OffsetSetting {
    Beginning = 0,
    Stored = 1,
    Manual = 2,
    End = 3,
  };

  /// @brief Keeps track of the current Kafka message offset setting.
  OffsetSetting usedOffsetSetting;

  /// @brief The list of PV:s created by the driver and their definition.
  std::vector<PV_param> paramsList = {
      PV_param("KAFKA_BROKER_ADDRESS", asynParamOctet), // kafka_addr
      PV_param("KAFKA_TOPIC", asynParamOctet),          // kafka_topic
      PV_param("KAFKA_GROUP", asynParamOctet),          // kafka_group
      PV_param("KAFKA_STATS_INT_MS", asynParamInt32),   // stats_time
      PV_param("KAFKA_SET_OFFSET", asynParamInt32),     // set_offset
  };

  /// @brief The consumeTask() function will keep running as long as this
  /// variable is set to true.
  std::atomic_bool keepThreadAlive{false};
};
