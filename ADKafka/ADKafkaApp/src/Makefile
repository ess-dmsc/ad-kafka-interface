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

LIB_LIBS += rdkafka++ rdkafka

USR_CXXFLAGS_Linux += -std=c++11
USR_CXXFLAGS += -I${AREA_DETECTOR}/ADCore/include

ifneq ($(findstring static,$(EPICS_HOST_ARCH)),)
USR_CXXFLAGS_WIN32 += -DLIBRDKAFKA_STATICLIB
endif

include $(ADCORE)/ADApp/commonLibraryMakefile

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD EXTRA GNUMAKE RULES BELOW HERE


$(COMMON_DIR)/NDArray_schema_generated.h : ../NDArray_schema.fbs $(FLATBUFFERS)/include/flatbuffers/flatbuffers.h
	$(FLATBUFFERS)/bin/$(EPICS_HOST_ARCH)/flatc --cpp -o $(COMMON_DIR)/ $<
