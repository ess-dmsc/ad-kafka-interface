TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS BELOW HERE

INC += KafkaPlugin.h
INC += json.h
INC += NDArray_schema_generated.h
INC_DIR += /opt/epics/base-3.15.4/include
LIBRARY_IOC += KafkaPlugin
LIB_SRCS += KafkaPlugin.cpp
LIB_SRCS += jsoncpp.cpp

DBD += KafkaPlugin.dbd

CXXFLAGS += -std=c++11
CXXFLAGS += -I${AREA_DETECTOR}/ADCore/include

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD EXTRA GNUMAKE RULES BELOW HERE

