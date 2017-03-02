/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaConsumer.h
 *  @brief Header file of a Kafka consumer class. Used together with an areaDetector driver.
 */

#pragma once

#include "ParamUtility.h"
#include "json.h"
#include <asynNDArrayDriver.h>
#include <librdkafka/rdkafkacpp.h>
#include <memory>
#include <string>
#include <vector>

/** @brief The KafkaInterface namespace is used primarily to seperate KafkaInterface::KafkaConsumer
 * from the class with the same name in librdkafka.
 */
namespace KafkaInterface {

/** @brief Hides and keeps track of the consumed message as stored in a RdKafka::Message class
 * instance.
 */
class KafkaMessage {
  public:
    /** @brief Stores a pointer to a RdKafka:Message.
     * @param[in] msg The pointer to the RdKafka::Message which is to be stored.
     */
    KafkaMessage(RdKafka::Message *msg);
    /// @brief De-allocates the stored RdKafka::Message.
    ~KafkaMessage();
    /** @brief Returns the pointer to the data stored in the RdKafka::message.
     * @return The pointer returned by this member function is still owned by the class and the data
     * it points to will become unavailable when the class instance is de-allocated.
     */
    void *GetDataPtr();

    /** @brief The size of the data in number of bytes as pointed to by the pointer returned by
     * KafkaMessage::GetDataPtr().
     */
    size_t size();

  private:
    /// @brief The pointer to the actual RdKafka::Message.
    std::unique_ptr<RdKafka::Message> msg;
};

/** @brief Consumes Kafka messages and returns a pointer to those messages for deserialisation.
 * Although the actual communication with the Kafka broker appears to be done in a separate thread
 * by librdkafka this class does not implement any extra threads for handling the data.
 * To correctly use this class, the following initlialization steps MUST be followed.
 * 1. Call the constructor of the class.
 * 2. Initialize the PV:s used by this class. Call KafkaConsumer::GetParams() to get PV definitons.
 * 3. Call KafkaConsumer::RegisterParamCallbackClass() to enable setting of PV:s.
 * @todo Add code to better keep track of the current client offset.
 * @todo Consider removing the default group id in the constructors.
 * @todo Move the callback functionality to a separate class when it is extended.
 */
class KafkaConsumer : public RdKafka::EventCb {
  public:
    /** @brief Sets up the class to consume messages from a Kafka broker.
     * @note After calling the constructor, the PV:s must be configured and
     * KafkaConsumer::RegisterParamCallbackClass() must be called according to the instructions
     * given in the class description.
     * Will attempt to initialize all parts required for consumption of messages. Having followed
     * the initilaized the PV:s and called KafkaConsumer::RegisterParamCallbackClass() only a call
     * to KafkaConsumer::StartConsumption() should be required.
     * @param[in] broker The address of the Kafka broker in the form "address:port". Can take
     * several addresses seperated by a comma (e.g. "address1:port1,address2:port2").
     * @param[in] topic Topic from which the driver should consume messages. Note that only
     * one topic can be specified.
     * @param[in] groupId The group id of the consumer, see the documentation for
     * KafkaConsumer::GetGroupId().
     */
    KafkaConsumer(std::string broker, std::string topic, std::string groupId);

    /** @brief Simple consumer constructor which will not connect to a broker.
     * @note After calling the constructor, the PV:s must be configured and
     * KafkaConsumer::RegisterParamCallbackClass() must be called according to the instructions
     * given in the class description.
     * Requires the setting of a broker address and topic name before consumption can be started.
     * @param[in] groupId The group id of the consumer, see the documentation for
     * KafkaConsumer::GetGroupId().
     */
    KafkaConsumer(std::string groupId);

    /** @brief Disconnect topic and deletes dynamically allocated objects.
     */
    ~KafkaConsumer();

    /** @brief Used to register the param callback class.
     * @note This member function must be called after the relevant PV:s have been initilaized. See
     * the class documentation for more information.
     * @param ptr Pointer to driver class instance.
     */
    virtual void RegisterParamCallbackClass(asynNDArrayDriver *ptr);

    /** @brief Set topic to consume messages from.
     * Will try to set a new topic and if successfull; will attempt to drop the current topic and
     * connect to the new one.
     * @param topicName The new topic.
     * @return True on succes, false on failure.
     */
    virtual bool SetTopic(std::string topicName);

    /** @brief Get the current topic name.
     * Will return the topic name stored by KafkaInterface::KafkaConsumer.
     * @return The current topic name.
     */
    virtual std::string GetTopic();

    /** @brief Set a new broker address.
     * Will drop the current broker/topic connection and attept to create a new one using the new
     * broker address. Has some limited error checking.
     * @param[in] brokerAddr The new broker address to use.
     * @return True on success, false on failure.
     */
    virtual bool SetBrokerAddr(std::string brokerAddr);

    /** @brief Return the current broker address stored by KafkaInterface::KafkaConsumer.
     * @return The current broker address as configured using KafkaConsumer::KafkaConsumer() or
     * KafkaConsumer::SetBrokerAddr().
     */
    virtual std::string GetBrokerAddr();

    /** @brief Used to consume messages made available by a Kafka broker.
     * This function will automatically call event KafkaConsumer::event_cb() at intervals as set by
     * KafkaConsumer::SetStatsTimeMS(). Note that the function will return immediatly if the
     * librdkafka is not set-up correctly to consume messages. This includes not having set a topic.
     * @todo Better handling of connection errors in order to prevent hammering of this function
     * when it return nullptr.
     * @return If the function times out or the connection (including topic) is not configured
     * correctly, returns nullptr. Returns a pointer to a KafkaInterface::KafkaMessage on success.
     * Note that the caller is responsible for calling delete on the returned pointer.
     */
    virtual std::unique_ptr<KafkaMessage> WaitForPkg(int timeout);

    /** @brief Start the consumption of messages.
     * KafkaInterface::KafkaConsumer does not start consumption automatically. This function must be
     * called to start consmumption. If consumption is stopped, KafkaConsumer::WaitForPkg() will
     * return nullptr. However, Kafka broker connection stats will be updated.
     */
    virtual void StartConsumption();

    /** @brief Stops the consumption of messages.
     * Stops the consumption of messages. If consumption is stopped, KafkaConsumer::WaitForPkg()
     * will return nullptr. However, Kafka broker connection stats will be updated.
     */
    virtual void StopConsumption();

    /** @brief Returns the current message offset as stored by KafkaInterface::KafkaConsumer.
     */
    virtual std::int64_t GetCurrentOffset();

    /** @brief Set a new message offset.
     * Will drop the current topic and re-create it using the provided new offset. Will also update
     * the relevant PV. This member function accepts three different negative offset values:
     * * -1000 : Sets the offset to the offset stored by the broker for the current group id. If no
     * offset is stored, the offset is set to that of the latest received message.
     * * -2 : Sets the offset to that of the first message still stored in the broker log.
     * * -1 : Sets the offset to that of the latest stored message.
     * The negative offsets are defined by librdkafa.
     * @param[in] offset The new message offset.
     * @return True on succes, false on failure to set the new offset.
     */
    virtual bool SetOffset(std::int64_t offset);

    /** @brief Used by the driver class in order for it to be able set the message offset.
     * @return The PV index used to set or get the current offset value in the PV database.
     */
    virtual int GetOffsetPVIndex();

    /** @brief Set a new group name/d.
     * The group id is used to keep track of the current message offset for a specific topic and
     * group of consumers. It is also used (I think) to make sure that only one consumer in a group
     * of consumers with identical group names recives a message from the broker.
     * @param[in] groupId The new group id to be used.
     * @return True if successfull, false otherwise.
     */
    virtual bool SetGroupId(std::string groupId);

    /** @brief Returns the group name/id stored by the KafkaConsumer.
     * Does not gurantee that  the returned string is the configured group name/id.
     * @return A text string which should represent the group name/id in use by the consumer.
     */
    virtual std::string GetGroupId();

    /** @brief Set the Kafka connection stats time interval.
     * Has some error checking to determine if it is possible to update this configuration and if it
     * is successfull. Note that even if successfull, the actual time between stats messages can
     * vary quite a bit based on how often KafkaConsumer::WaitForPkg() is called and other things.
     * @param[in] time The time in milliseconds (ms) between the reporting of connection statistics
     * by librdkafka.
     * @return True on success, false on failure.
     */
    virtual bool SetStatsTimeIntervalMS(int timeInterval);

    /** @brief Returns the current Kafka stats interval time as stored by KafkaConsumer.
     * Does not guarantee that this is the acutal interval between times the connection stats are
     * obtained.
     */
    virtual int GetStatsTimeMS();

    /** @brief Returns the PV definitions used by the KafkaInterface::KafkaConsumer.
     * KafkaInterface::KafkaConsumer can not initialize its own PV:s as the driver needs to know:
     * * How many PVs will be used in order to allocate enough memory for them.
     * * The lowest index of the PVs that the driver is responsible for in order to determine which
     * PV indexes has to be handled by a parent class.
     * @return The definitions of the PVs which values are modified by this class. Note that the
     * location of the index is kept track of by a std::shared_ptr.
     */
    virtual std::vector<PV_param> &GetParams();

    /** @brief Returns KafkaConsumer::PV::count. Required by the driver parent class to allocate
     * enough memory for the PV:s used by this class.
     */
    static int GetNumberOfPVs();

  protected:
    /** @brief I set to tru if initialization of librdkafka fails. Only changed by
    * KafkaConsumer::InitRdKafka().
    */
    bool errorState = false;

    /// @brief Used keep track of if consumption is currently halted.
    bool consumptionHalted = true;

    size_t bufferSize = 100000000;

    /** @brief Used to store the current message offset. Updated by KafkaConsumer::WaitForPkg().
    */
    std::int64_t topicOffset;

    /** @brief Used as a textual representation of the current state of the Kafka broker connection.
     */
    enum class ConStat {
        CONNECTED = 0,
        CONNECTING = 1,
        DISCONNECTED = 2,
        ERROR = 3,
    };

    /** @brief Sets the status PV:s with a status id and status string.
     * Is not guaranteed to actually set any PV:s if they are not initialized.
     * @param[in] stat The integer value representing the current status of the Kafka system.
     * Should be a KafkaPlugin::ConStat enum value.
     * @param[in] msg Text string which represents the current status of the Kafka system. Can not
     * be more than 40 characters.
     */
    virtual void SetConStat(ConStat stat, std::string msg);

    /** @brief Allocates the broker configuration object and sets some configurations to their
     * initial values.
     * This member function is called by the constructor and although it calls
     * KafkaConsumer::SetConStat() no connection state will be logged as the PVs are not correctly
     * initialized at this point. This should probably be changed.
     */
    virtual void InitRdKafka(std::string groupId);

    /** @brief Helper function which recreates a broker connection.
     * Attempts to close the current broker connection and create a new one based on the current
     * configurations. Called by several other member functions.
     */
    virtual bool MakeConnection();

    /** @brief Helper function which recreates a topic connection.
     * This function is called by other member functions when some settings relevant to the broker
     * and/or topic connection has changed. If a broker address is set, this function will then
     * attempt to connect to a topic (if set). If the consumption is set to "halted" internally,
     * this wil be honored when creating the new topic connection.
     * @return True on success, false on failure.
     */
    virtual bool UpdateTopic();

    /** @brief Parses a Json string as obtained from an Rdkafka::Event object and extract some
     * connection stats.
     * This function currently only extracts number of unsent messages in the librdkafka buffer and
     * if the number of connected brokers are 0. Based on this it sets the relevant PVs containing
     * the number of packets in the buffer and connection status.
     */
    virtual void ParseStatusString(std::string msg);

    int kafka_stats_interval = 500; /// @brief Saved Kafka connection stats interval in ms.

    /** @brief The event callback function called by librdkafka for purposes of getting connection
     * information.
     * Calls KafkaConsumer::ParseStatusString() to get current connection information. This function
     * currently only handles some error events and status events. Its functionality could be
     * improved in order to handle more error events, log events, throttling events etc.
     * @param[in] event The event object containing error information, stats information or other
     * information.
     */
    void event_cb(RdKafka::Event &event);

    /** @brief The pointer to the actual driver class which instantiated this class. Required for
     * updating PVs.
     */
    asynNDArrayDriver *paramCallback;

    std::string topicName;  /// @brief Stores the current topic used by the consumer.
    std::string brokerAddr; /// @brief Stores the current broker address used by the consumer.
    std::string groupName;  /// @brief Stores the current group name used by the consumer.

    /// @brief Used to take care of error strings returned by verious librdkafka functions.
    std::string errstr;

    /// @brief Stores the pointer to a librdkafka configruation object.
    std::unique_ptr<RdKafka::Conf> conf;

    /// @brief Pointer to Kafka consumer in librdkafka.
    RdKafka::KafkaConsumer *consumer = nullptr;

    /// @brief The root and broker json objects extracted from a json string.
    Json::Value root, brokers;

    /// @brief Parses std:string objects into a Json::value.
    Json::Reader reader;

    /// @brief Used to keep track of the PV:s made available by this driver.
    enum PV {
        max_msg_size,
        con_status,
        con_msg,
        msg_offset,
        count,
    };

    /// @brief The list of PV:s created by the driver and their definition.
    std::vector<PV_param> paramsList = {
        PV_param("KAFKA_MAX_MSG_SIZE", asynParamInt32),          // max_msg_size
        PV_param("KAFKA_CONNECTION_STATUS", asynParamInt32),     // con_status
        PV_param("KAFKA_CONNECTION_MESSAGE", asynParamOctet),    // con_msg
        PV_param("KAFKA_CURRENT_OFFSET", asynParamInt32),        // msg_offset
    };
};
}
