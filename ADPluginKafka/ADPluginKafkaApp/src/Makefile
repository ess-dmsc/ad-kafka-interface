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

LIB_LIBS += NDPlugin
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
