< envPaths
errlogInit(20000)

dbLoadDatabase("$(TOP)/dbd/ADPluginKafkaApp.dbd")
ADPluginKafkaApp_registerRecordDeviceDriver(pdbbase)

epicsEnvSet("PREFIX", "$(PREFIX=YSX26594)")
epicsEnvSet("SIMDET_PORT", "$(PREFIX)SIMDET")
epicsEnvSet("K_PORT", "$(PREFIX)K")
epicsEnvSet("XSIZE", "$(XSIZE=10000000)")
epicsEnvSet("YSIZE", "$(YSIZE=1)")
epicsEnvSet("QSIZE", "20")
epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(ADCORE)/db")

simDetectorConfig("$(SIMDET_PORT)", $(XSIZE), $(YSIZE), 1, 0, 0)
dbLoadRecords("$(SIMDET)/db/simDetector.template", "P=$(PREFIX):, R=SIM:, PORT=$(SIMDET_PORT), ADDR=0, TIMEOUT=1")

KafkaPluginConfigure("$(K_PORT)", 3, 1, "$(SIMDET_PORT)", 0, -1, "cs05r-sc-serv-15:9092", "sim_data_topic_14")
dbLoadRecords("$(ADPLUGINKAFKA)/db/ADPluginKafka.template", "P=$(PREFIX),R=:KFK:,PORT=$(K_PORT),ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(SIMDET_PORT)")

iocInit()

dbpf $(PREFIX):KFK:EnableCallbacks Enable
dbpf $(PREFIX):SIM:AcquirePeriod 0.0004
dbpf $(PREFIX):SIM:AcquireTime 0.0004
dbpf $(PREFIX):KFK:KafkaMaxQueueSize 1000
dbpf $(PREFIX):KFK:QueueSize 1000
dbpf $(PREFIX):SIM:Acquire 1

#Always end with a new line
