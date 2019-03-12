TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS BELOW HERE

INC += KafkaDriver.h
INC += KafkaConsumer.h
INC += json.h
INC += NDArray_schema_generated.h
INC += ParamUtility.h
INC += NDArrayDeSerializer.h
LIBRARY_IOC += ADKafka
LIB_SRCS += KafkaDriver.cpp
LIB_SRCS += KafkaConsumer.cpp
LIB_SRCS += NDArrayDeSerializer.cpp
LIB_SRCS += jsoncpp.cpp

DBD += ADKafka.dbd

LDFLAGS += -lrdkafka
LDFLAGS += -lrdkafka++

CXXFLAGS += -std=c++11
CXXFLAGS += -I${AREA_DETECTOR}/ADCore/include

include $(ADCORE)/ADApp/commonLibraryMakefile

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD EXTRA GNUMAKE RULES BELOW HERE

