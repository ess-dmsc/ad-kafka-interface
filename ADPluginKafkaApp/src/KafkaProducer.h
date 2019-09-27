/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaProducer.h
 *  @brief Kafka producer part of EPICS C++ areaDetector plugin.
 */

#pragma once

#include "ParamUtility.h"
#include "json.h"
#include <asynNDArrayDriver.h>
#include <atomic>
#include <librdkafka/rdkafkacpp.h>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

/** @brief The KafkaInterface namespace is used primarily to seperate
 * KafkaInterface::KafkaConsumer
 * from the class with the same name in librdkafka.
 */
namespace KafkaInterface {

/** @brief The class which handles the production of Kafka messages, i.e. it
 * sends data to the
 * broker.
 * This can not do all the initialization steps on its own. Instead the
 * following instructions for
 * making the class ready to produce messages MUST be followed:
 * 1. Call the constructor of the class.
 * 2. Initialize the PV:s used by this class. Call KafkaConsumer::GetParams() to
 * get PV definitons.
 * 3. Call KafkaConsumer::RegisterParamCallbackClass() to enable setting of
 * PV:s.
 * 4. Call KafkaConsumer::StartThread() to enable periodic polling of the
 * connection status to the
 * Kafka brokers.
 * @todo This class copies the data that is to be sent, make it so that it does
 * not have to.
 */
class KafkaProducer : public RdKafka::EventCb {
public:
  /** @brief Sets up the producer to send messages to a Kafka broker.
   * @note The steps for setting up this class as described in the class
   * description MUST be
   * followed.
   * @param[in] broker The address of the Kafka broker in the form
   * "address:port". Can take
   * several addresses seperated by a comma (e.g.
   * "address1:port1,address2:port2").
   * @param[in] topic Topic from which the driver should consume messages. Note
   * that only
   * one topic can be specified.
   * @param[in] queueSize The maximum number of messages that the librdkafka
   * will store in its
   * buffer.
   */
  KafkaProducer(std::string const &broker, std::string topic,
                int queueSize = 10);

  /** @brief Simple consumer constructor which will not connect to a broker.
   * @note After calling the constructor, the rest of the instructions given in
   * the class
   * description must also be followed.
   * Requires the setting of a broker address and topic name before production
   * can be started.
   * @param[in] queueSize The maximum number of messages that the librdkafka
   * will store in its
   * buffer.
   */
  explicit KafkaProducer(int queueSize = 10);

  /** @brief Destructor.
   * Will signal the stats thread to exit and will only return when it has done
   * so which might
   * take some time. Will attempt to gracefully shut down the Kafka connection.
   */
  ~KafkaProducer();

  /** @brief Returns the PV definitions used by the KafkaInterface::Producer.
   * KafkaInterface::KafkaProducer can not initialize its own PV:s as the driver
   * needs to know:
   * * How many PVs will be used in order to allocate enough memory for them.
   * * The lowest index of the PVs that the driver is responsible for in order
   * to determine which
   * PV indexes has to be handled by a parent class.
   * @return The definitions of the PVs which values are modified by this class.
   * Note that the
   * location of the index is kept track of by a std::shared_ptr.
   */
  virtual std::vector<PV_param> &GetParams();

  /** @brief Used to register the param callback class.
   * @note This member function must be called after the relevant PV:s have been
   * initilaized. See
   * the class documentation for more information.
   * Will set the maximum message size PV when called.
   * @param ptr Pointer to driver class instance.
   */
  virtual void RegisterParamCallbackClass(asynNDArrayDriver *ptr);

  /** @brief Set topic to consume messages from.
   * Will try to set a new topic and if successfull; will attempt to drop the
   * current topic and
   * connect to the new one.
   * @param topicName The new topic.
   * @return True on succes, false on failure.
   */
  virtual bool SetTopic(std::string const &topicName);

  /** @brief Get the current topic name.
   * Will return the topic name stored by KafkaInterface::KafkaProducer.
   * @return The current topic name.
   */
  virtual std::string GetTopic();

  /** @brief Set a new broker address.
   * Will drop the current broker/topic connection and attept to create a new
   * one using the new
   * broker address. Has some limited error checking.
   * @param[in] brokerAddr The new broker address to use.
   * @return True on success, false on failure.
   */
  virtual bool SetBrokerAddr(std::string const &brokerAddr);

  /** @brief Return the current broker address stored by
   * KafkaInterface::KafkaProducer.
   * @return The current broker address as configured using
   * KafkaProducer::KafkaProducer() or
   * KafkaProducer::SetBrokerAddr().
   */
  virtual std::string GetBrokerAddr();

  /** @brief Used to set the maximum message size that the producer will handle.
   * Note that the maximum message size has a hardcoded upper limit which
   * currently is 1e9 bytes
   * (approx. 954 MB). Will destroy the current connection and do a re-connect
   * using the new
   * limit.
   * @param[in] msgSize Maximum message size in bytes.
   * @return True on success and false on failure.
   */
  virtual bool SetMaxMessageSize(size_t msgSize);

  /** @brief Used to set the size of the Kafka message buffer in kb.
   * Will destroy the current connection and do a re-connect
   * using the new limit.
   * @param[in] msgBufferSize New buffer size in kilo bytes.
   * @return True on success and false on failure.
   */
  virtual bool SetMessageBufferSizeKbytes(size_t msgBufferSize);

  /** @brief The current Kafka message buffer size as stored by
   * KafkaInterface::KafkaProducer.
   * @return Kafka message buffer suze in kilo bytes.
   */
  virtual size_t GetMessageBufferSizeKbytes();

  /** @brief The maximum message size as stored by
   * KafkaInterface::KafkaProducer.
   * @return Maximum message size in number of bytes.
   */
  virtual size_t GetMaxMessageSize();

  /** @brief Sets the maximum number of messages in the Kafka producer buffer.
   * Callling this function will destroy the current connection and do a
   * reconnect with the new
   * setting if possible.
   * @return True on success and false on failure.
   */
  virtual bool SetMessageQueueLength(int queue);

  /** @brief Get the maximum number of queued messages in the Kafka producer
   * buffer.
   * @return The maximum number of messages.
   */
  virtual int GetMessageQueueLength();

  /** @brief Set the Kafka connection stats time interval.
   * Has some error checking to determine if it is possible to update this
   * configuration and if it
   * is successfull. Note that even if successfull, the actual time between
   * stats messages can
   * vary quite a bit based on how often KafkaConsumer::WaitForPkg() is called
   * and other things.
   * @param[in] time The time in milliseconds (ms) between the reporting of
   * connection statistics
   * by librdkafka.
   * @return True on success, false on failure.
   */
  virtual bool SetStatsTimeMS(int time);

  /** @brief Returns the current Kafka stats interval time as stored by
   * KafkaConsumer.
   * Does not guarantee that this is the acutal interval between times the
   * connection stats are
   * obtained.
   */
  virtual int GetStatsTimeMS();

  /** @brief Set if the class should try to flush messages from the buffer when
   * disconnecting
   * from the broker.
   * @param[in] flush Should a flush be attempted?
   * @param[in] timeout_ms What is the timeout in milliseconds (ms) before the
   * attempt should be
   * abandoned.
   */
  virtual void AttemptFlushAtReconnect(bool flush, int timeout_ms);

  /** @brief Starts the thread that keeps track of the status of the Kafka
   * connection.
   * @note Call this thread only after the PV parameters have been registered
   * with the
   * EPICS subsystem as the indexes are not protected against simultaneous
   * access from
   * different threads.
   */
  virtual bool StartThread();

  /** @brief Sends the binary data stored in the buffer to the Kafka broker.
   * \todo Complete documentation.
   */
  virtual bool SendKafkaPacket(const unsigned char *buffer, size_t buffer_size);

  static int GetNumberOfPVs();

protected:
  bool errorState{
      false}; /// @brief Set to true if librdkafka could not be initialized.
  bool doFlush{true}; /// @brief Should a flush attempt be made at disconnect?
  int flushTimeout{500}; /// @brief What is the timeout of the flush attempt?

  size_t maxMessageSize{
      10000000}; /// @brief Stored maximum message size in bytes.
  size_t maxMessageBufferSizeKb{
      500000};      /// @brief Message buffer size in kilo bytes.
  int msgQueueSize; /// @brief Stored maximum Kafka producer queue length.

  /** @brief Helper function for cleanly shutting down a topic.
   * Implements the flushing functionality.
   */
  virtual void ShutDownTopic();

  /** @brief Helper function for shutting down and deallocating the producer.
   * Also calls KafkaProducer::ShutDownTopic() if needed.
   */
  virtual void ShutDownProducer();

  /** @brief Callback member function used by the status and error handling
   * system of librdkafka.
   * This member function is registered as a callback function with librdkafka
   * for error and
   * status messages. Status messages are received as JSON strings which are
   * decoded using
   * jsoncpp. Other events are currently barely handled.
   * @param[in] event RdKafka::Event instance that holds information on
   * statistics, errors or
   * other events.
   */
  virtual void event_cb(RdKafka::Event &event);

  /** @brief Thread member function. Should only be called by
   * KafkaProducer::StartThread().
   */
  virtual void ThreadFunction();

  // Kafka connection status enum
  enum class ConStat {
    CONNECTED = 0,
    CONNECTING = 1,
    DISCONNECTED = 2,
    ERROR = 3,
  };

  /** @brief Sets the correct status PV:s.
   * Will call KafkaPlugin::DestroyKafkaConnection() if the status id is equal
   * to
   * KafkaPlugin::ERROR.
   * @param[in] stat Takes an integer value representing the current status of
   * the Kafka system.
   * Should be a KafkaPlugin::ConStat enum value.
   * @param[in] msg Text string which represents the current status of the Kafka
   * system. Can not
   * be more than 40 characters.
   */
  virtual void SetConStat(ConStat stat, std::string const &msg);

  /** @brief Parses JSON status string from Kafka system and updates PV:s.
   * Uses jsoncpp to parse the status string and extracts current connection
   * status of available
   * brokers as well as the number of messages not yet transmitted to the Kafka
   * broker. This
   * information is then used to update the relevant PV:s.
   * @param[in] msg JSON status message obtained from the Kafka producer system.
   */
  virtual void ParseStatusString(std::string const &msg);

  int kafka_stats_interval{
      500}; /// @brief Saved Kafka connection stats interval in ms.

  /// @brief Sleep time between poll()-calls. See
  /// KafkaProducer::ThreadFunction().
  const int sleepTime{50};

  mutable std::mutex
      brokerMutex; /// @brief Prevents access to shared resources.

  /** @brief Attempts to init the Kafka producer system of librdkafka.
   * Failure to init the Kafka system results in a error message written to the
   * relevant PV and
   * an attempt in cleaning up (deleting) all the variables related to the Kafka
   * producer.
   * Note that this function does not actually connect to a Kafka broker but
   * instead starts the
   * Kafka producer system that will attempt to connect to a broker.
   */
  void InitRdKafka();

  /** @brief Helper function which recreates a broker connection.
   * Attempts to close the current broker connection and create a new one based
   * on the current
   * configurations. Called by several other member functions.
   */
  virtual bool MakeConnection();

  /// @brief Used to take care of error strings returned by verious librdkafka
  /// functions.
  std::string errstr;

  /// @brief Pointer to Kafka topic in librdkafka.
  RdKafka::Topic *topic{nullptr};

  /// @brief Pointer to Kafka producer in librdkafka.
  RdKafka::Producer *producer{nullptr};

  /// @brief Stores the pointer to a librdkafka configruation object.
  std::unique_ptr<RdKafka::Conf> conf;

  /// @brief Stores the pointer to a librdkafka topic configruation object.
  std::unique_ptr<RdKafka::Conf> tconf;

  std::string
      topicName; /// @brief Stores the current topic used by the consumer.
  std::string brokerAddr; /// @brief Stores the current broker address used by
                          /// the consumer.

  /// @brief The root and broker json objects extracted from a json string.
  Json::Value root, brokers;
  Json::Reader reader; /// @brief Parses std:string objects into a Json::value.

  /// @brief C++11 thread which periodically polls for connection stats.
  std::thread statusThread;

  /** @brief The pointer to the actual driver class which instantiated this
   * class. Required for
   * updating PVs.
   */
  asynNDArrayDriver *paramCallback{nullptr};

  /// @brief Used to shut down the stats thread.
  std::atomic_bool runThread{false};

  /// @brief Used to keep track of the PV:s made available by this driver.
  enum PV {
    con_status,
    con_msg,
    msgs_in_queue,
    max_msg_size,
    msg_buffer_size,
    count,
  };

  /// @brief The list of PV:s created by the driver and their definition.
  std::vector<PV_param> paramsList = {
      PV_param("KAFKA_CONNECTION_STATUS", asynParamInt32),  // con_status
      PV_param("KAFKA_CONNECTION_MESSAGE", asynParamOctet), // con_msg
      PV_param("KAFKA_UNSENT_PACKETS", asynParamInt32),     // msgs_in_queue
      PV_param("KAFKA_MAX_MSG_SIZE", asynParamInt32),       // max_msg_size
      PV_param("KAFKA_MSG_BUFFER_SIZE", asynParamInt32),    // msg_buffer_size
  };
};
} // namespace KafkaInterface
