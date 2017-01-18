require adcore,2.5-BETA
require adexample,2.2-BETA
require pvaSrv

epicsEnvSet("PREFIX", "$(PREFIX=DMSC)")
epicsEnvSet("SIMDET_PORT", "$(PREFIX)SIMDET")
epicsEnvSet("K_PORT", "$(PREFIX)K")
epicsEnvSet("XSIZE", "$(XSIZE=200)")
epicsEnvSet("YSIZE", "$(YSIZE=200)")
epicsEnvSet("QSIZE", "20")

simDetectorConfig("$(SIMDET_PORT)", $(XSIZE), $(YSIZE), 1, 0, 0)
dbLoadRecords("simDetector.template", "P=$(PREFIX):, R=CAM:, PORT=$(SIMDET_PORT), ADDR=0, TIMEOUT=1")

KafkaPluginConfigure("$(K_PORT)", 3, 1, "$(SIMDET_PORT)", 0)
dbLoadRecords("KafkaPlugin.template", "P=$(PREFIX),R=:KFK:,PORT=$(K_PORT),ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(SIMDET_PORT)")

iocInit

startPVAServer

dbpf $(PREFIX):KFK:EnableCallbacks Enable
dbpf $(PREFIX):CAM:AcquirePeriod 2
dbpf $(PREFIX):CAM:Acquire 1
dbpf $(PREFIX):KFK:KafkaBrokers "10.4.0.215:9092"
dbpf $(PREFIX):KFK:KafkaBrokerTopic "ad_topic"
