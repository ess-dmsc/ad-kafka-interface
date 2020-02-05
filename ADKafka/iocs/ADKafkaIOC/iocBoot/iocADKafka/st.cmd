< envPaths
errlogInit(20000)

dbLoadDatabase("$(TOP)/dbd/ADKafkaApp.dbd")
ADKafkaApp_registerRecordDeviceDriver(pdbbase)

epicsEnvSet("PREFIX", "$(PREFIX=DMSC)")
epicsEnvSet("KFKDET_PORT", "$(PREFIX)_AD_KAFKA")
epicsEnvSet("K_PORT", "$(PREFIX)K")
epicsEnvSet("QSIZE", "20")
epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(ADCORE)/db")


KafkaDriverConfigure("$(KFKDET_PORT)", 10, 0, 0, 0, "10.4.0.216:9092", "test_topic")
dbLoadRecords("$(ADKAFKA)/db/ADKafka.template", "P=$(PREFIX):, R=KFK_DRVR:, PORT=$(KFKDET_PORT), ADDR=0, TIMEOUT=1")

NDPvaConfigure("PVA", $(QSIZE), 0, "$(KFKDET_PORT)", 0, $(PREFIX):PVA:Image, 0, 0, 0)
dbLoadRecords("NDPva.template",  "P=$(PREFIX):,R=PVA:, PORT=PVA,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(KFKDET_PORT)")

startPVAServer()

iocInit()

dbpf $(PREFIX):PVA:EnableCallbacks Enable
dbpf $(PREFIX):KFK_DRVR:Acquire 1
