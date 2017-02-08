require adcore
require pvaSrv

epicsEnvSet("PREFIX", "$(PREFIX=DMSC)")
epicsEnvSet("KFKDET_PORT", "$(PREFIX)_AD_KAFKA")
epicsEnvSet("K_PORT", "$(PREFIX)K")
epicsEnvSet("QSIZE", "20")

KafkaDriverConfigure("$(KFKDET_PORT)", 0, 0, 0, 0, "some_broker", "some_topic")
dbLoadRecords("ADKafka.template", "P=$(PREFIX):, R=CAM:, PORT=$(KFKDET_PORT), ADDR=0, TIMEOUT=1")

#KafkaPluginConfigure("$(K_PORT)", 3, 1, "$(SIMDET_PORT)", 0)
#dbLoadRecords("KafkaPlugin.template", "P=$(PREFIX),R=:KFK:,PORT=$(K_PORT),ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(SIMDET_PORT)")

iocInit

startPVAServer

#dbpf $(PREFIX):KFK:EnableCallbacks Enable
#dbpf $(PREFIX):CAM:Acquire 1
#dbpf $(PREFIX):KFK:KafkaBrokers "10.4.0.215:9092"
#dbpf $(PREFIX):KFK:KafkaBrokerTopic "ad_topic"

#Remember; file MUST end with a new line
