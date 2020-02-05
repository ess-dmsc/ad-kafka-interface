TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS BELOW HERE

INC += KafkaPlugin.h
INC += NDArraySerializer.h
INC += KafkaProducer.h
INC += ParamUtility.h
INC += json.h
INC += NDArray_schema_generated.h
LIBRARY_IOC += ADPluginKafka
LIB_SRCS += KafkaPlugin.cpp
LIB_SRCS += KafkaProducer.cpp
LIB_SRCS += NDArraySerializer.cpp
LIB_SRCS += jsoncpp.cpp

DBD += ADPluginKafka.dbd

LDFLAGS += -lrdkafka
LDFLAGS += -lrdkafka++

CXXFLAGS += -std=c++11
CXXFLAGS += -I${AREA_DETECTOR}/ADCore/include


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD EXTRA GNUMAKE RULES BELOW HERE

