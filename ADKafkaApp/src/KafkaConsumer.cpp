/** Copyright (C) 2017 European Spallation Source */

/** @file  KafkaConsumer.cpp
 *  @brief Implementation of a Kafka consumer. Used in a kafka areaDetector
 * driver.
 */

#include "KafkaConsumer.h"
#include <ciso646>

namespace KafkaInterface {

int KafkaConsumer::GetNumberOfPVs() { return PV::count; }

KafkaMessage::KafkaMessage(RdKafka::Message *msg) : msg(msg) {}

void *KafkaMessage::GetDataPtr() { return msg->payload(); }

size_t KafkaMessage::size() { return msg->len(); }

KafkaConsumer::KafkaConsumer(std::string const &broker,
                             std::string const &topic,
                             std::string const &groupId)
    : topicName(topic), conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)),
      brokerAddr(broker) {
  KafkaConsumer::InitRdKafka(groupId);
  KafkaConsumer::SetBrokerAddr(broker);
  KafkaConsumer::SetTopic(topic);
}

KafkaConsumer::KafkaConsumer(std::string const &groupId)
    : conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)) {
  KafkaConsumer::InitRdKafka(groupId);
}

KafkaConsumer::~KafkaConsumer() {
  if (nullptr != consumer) {
    consumer->unassign();
    consumer->close();
    delete consumer;
    consumer = nullptr;
  }
}

void KafkaConsumer::InitRdKafka(std::string const &groupId) {
  if (nullptr == conf) {
    errorState = true;
    KafkaConsumer::SetConStat(KafkaConsumer::ConStat::ERROR,
                              "Can not create global conf object.");
    return;
  }

  RdKafka::Conf::ConfResult configResult;
  configResult = conf->set("event_cb", this, errstr);
  if (RdKafka::Conf::CONF_OK != configResult) {
    errorState = true;
    KafkaConsumer::SetConStat(KafkaConsumer::ConStat::ERROR,
                              "Can not set event callback.");
    return;
  }

  configResult = conf->set("statistics.interval.ms",
                           std::to_string(kafka_stats_interval), errstr);
  if (RdKafka::Conf::CONF_OK != configResult) {
    KafkaConsumer::SetConStat(KafkaConsumer::ConStat::ERROR,
                              "Unable to set statistics interval.");
  }

  if (groupId.empty()) {
    KafkaConsumer::SetConStat(KafkaConsumer::ConStat::ERROR,
                              "Unable to set group id.");
    errorState = true;
    return;
  }
  configResult = conf->set("group.id", groupId, errstr);
  if (RdKafka::Conf::CONF_OK != configResult) {
    KafkaConsumer::SetConStat(KafkaConsumer::ConStat::ERROR,
                              "Unable to set group id.");
    errorState = true;
    return;
  }
  groupName = groupId;
}

std::vector<PV_param> &KafkaConsumer::GetParams() { return paramsList; }

bool KafkaConsumer::SetOffset(std::int64_t offset) {
  if (offset < 0) {
    if (RdKafka::Topic::OFFSET_BEGINNING != offset and
        RdKafka::Topic::OFFSET_STORED != offset and
        RdKafka::Topic::OFFSET_END != offset) {
      return false;
    }
  }
  topicOffset = offset;
  setParam(paramCallback, paramsList.at(msg_offset), static_cast<int>(offset));
  UpdateTopic();
  return true;
}

std::string KafkaConsumer::GetTopic() { return topicName; }

std::string KafkaConsumer::GetBrokerAddr() { return brokerAddr; }

std::unique_ptr<KafkaMessage> KafkaConsumer::WaitForPkg(int timeout) {
  if (nullptr != consumer and not topicName.empty()) {
    RdKafka::Message *msg = consumer->consume(timeout);
    if (msg->err() == RdKafka::ERR_NO_ERROR) {
      topicOffset = msg->offset();
      setParam(paramCallback, paramsList[PV::msg_offset],
               static_cast<int>(topicOffset));
      return std::unique_ptr<KafkaMessage>(new KafkaMessage(msg));
    } else {
      delete msg;
      return nullptr;
    }
  } else {
    return nullptr;
  }
  return nullptr;
}

void KafkaConsumer::event_cb(RdKafka::Event &event) {
  /// @todo This member function really needs some expanded capability
  switch (event.type()) {
  case RdKafka::Event::EVENT_ERROR:
    if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN) {
      SetConStat(KafkaConsumer::ConStat::DISCONNECTED,
                 "Brokers down. Attempting to reconnect.");
    } else {
      SetConStat(KafkaConsumer::ConStat::DISCONNECTED,
                 "Event error received: " + std::to_string(event.err()));
    }
    break;
  case RdKafka::Event::EVENT_LOG:
    /// @todo Add message/log or something
    break;
  case RdKafka::Event::EVENT_THROTTLE:
    /// @todo Add message/log or something
    break;
  case RdKafka::Event::EVENT_STATS:
    ParseStatusString(event.str());
    break;
  default:
    if ((event.type() == RdKafka::Event::EVENT_LOG) and
        (event.severity() == RdKafka::Event::EVENT_SEVERITY_ERROR)) {
      /// @todo Add message/log or something

    } else {
      /// @todo Add message/log or something
    }
  }
}

void KafkaConsumer::ParseStatusString(std::string const &msg) {
  /// @todo We should probably extract some more stats from the JSON message
  bool parseSuccess = reader.parse(msg, root);
  if (not parseSuccess) {
    SetConStat(KafkaConsumer::ConStat::ERROR, "Status msg.: Unable to parse.");
    return;
  }
  brokers = root["brokers"];
  if (brokers.isNull() or brokers.empty()) {
    SetConStat(KafkaConsumer::ConStat::ERROR, "Status msg.: No brokers.");
  } else {
    KafkaConsumer::ConStat tempStat = KafkaConsumer::ConStat::DISCONNECTED;
    std::string statString = "Brokers down. Attempting reconnection.";
    if (std::any_of(brokers.begin(), brokers.end(),
                    [](Json::Value const &CBrkr) {
                      return "UP" == CBrkr["state"].asString();
                    })) {
      tempStat = KafkaConsumer::ConStat::CONNECTED;
      statString = "No errors.";
    }
    SetConStat(tempStat, statString);
  }
}

std::int64_t KafkaConsumer::GetCurrentOffset() { return topicOffset; }

bool KafkaConsumer::UpdateTopic() {
  if (nullptr != consumer and not topicName.empty()) {
    consumer->unassign();
    std::vector<RdKafka::TopicPartition *> topics;
    topics.push_back(
        RdKafka::TopicPartition::create(topicName, 0, topicOffset));
    consumer->assign(topics);
    if (consumptionHalted) {
      consumer->pause(topics);
    }
  } else {
    return false;
  }
  return true;
}

void KafkaConsumer::StartConsumption() {
  if (consumptionHalted) {
    consumptionHalted = false;
    if (consumer != nullptr) {
      std::vector<RdKafka::TopicPartition *> topics;
      consumer->assignment(topics);
      consumer->resume(topics);
    }
  }
}

void KafkaConsumer::StopConsumption() {
  if (not consumptionHalted) {
    consumptionHalted = true;
    if (consumer != nullptr) {
      std::vector<RdKafka::TopicPartition *> topics;
      consumer->assignment(topics);
      consumer->pause(topics);
    }
  }
}

bool KafkaConsumer::MakeConnection() {
  if (consumer != nullptr) {
    consumer->unassign();
    consumer->close();
    delete consumer;
    consumer = nullptr;
  }
  if (not brokerAddr.empty()) {
    consumer = RdKafka::KafkaConsumer::create(conf.get(), errstr);
    if (nullptr == consumer) {
      SetConStat(KafkaConsumer::ConStat::ERROR, "Unable to create consumer.");
      return false;
    }
    UpdateTopic();
  }
  return true;
}

bool KafkaConsumer::SetTopic(std::string const &topicName) {
  if (errorState or topicName.empty()) {
    return false;
  }
  KafkaConsumer::topicName = topicName;
  UpdateTopic();
  return true;
}

bool KafkaConsumer::SetBrokerAddr(std::string const &brokerAddr) {
  if (errorState or brokerAddr.empty()) {
    return false;
  }
  RdKafka::Conf::ConfResult cRes;
  cRes = conf->set("metadata.broker.list", brokerAddr, errstr);
  if (RdKafka::Conf::CONF_OK != cRes) {
    SetConStat(KafkaConsumer::ConStat::ERROR, "Can not set new broker.");
    return false;
  }
  KafkaConsumer::brokerAddr = brokerAddr;
  MakeConnection();
  return true;
}

std::string KafkaConsumer::GetGroupId() { return groupName; }

bool KafkaConsumer::SetGroupId(std::string const &groupId) {
  if (errorState or groupId.empty()) {
    return false;
  }
  RdKafka::Conf::ConfResult cRes;
  cRes = conf->set("group.id", groupId, errstr);
  if (RdKafka::Conf::CONF_OK != cRes) {
    SetConStat(KafkaConsumer::ConStat::ERROR, "Can not set new group id.");
    return false;
  }
  groupName = groupId;
  MakeConnection();
  return true;
}

void KafkaConsumer::SetConStat(ConStat stat, std::string const &msg) {
  setParam(paramCallback, paramsList[PV::con_status], static_cast<int>(stat));
  setParam(paramCallback, paramsList[PV::con_msg], msg);
}

void KafkaConsumer::RegisterParamCallbackClass(asynNDArrayDriver *ptr) {
  paramCallback = ptr;
  setParam(paramCallback, paramsList[PV::msg_offset],
           static_cast<int>(RdKafka::Topic::OFFSET_STORED));
}

bool KafkaConsumer::SetStatsTimeIntervalMS(int timeInterval) {
  if (errorState or timeInterval <= 0) {
    return false;
  }
  RdKafka::Conf::ConfResult configResult;
  configResult =
      conf->set("statistics.interval.ms", std::to_string(timeInterval), errstr);
  if (RdKafka::Conf::CONF_OK != configResult) {
    SetConStat(KafkaConsumer::ConStat::ERROR,
               "Unable to set statistics interval.");
    return false;
  }
  kafka_stats_interval = timeInterval;
  MakeConnection();
  return true;
}

int KafkaConsumer::GetStatsTimeMS() { return kafka_stats_interval; }

int KafkaConsumer::GetOffsetPVIndex() {
  return *paramsList[PV::msg_offset].index;
}
} // namespace KafkaInterface
