require adcore,2.6+
require ADPluginKafka,1.0.0-BETA

epicsEnvSet("PREFIX", "$(PREFIX=DMSC)")
epicsEnvSet("KFKDET_PORT", "$(PREFIX)_AD_KAFKA")
epicsEnvSet("K_PORT", "$(PREFIX)K")
epicsEnvSet("QSIZE", "20")

KafkaDriverConfigure("$(KFKDET_PORT)", 10, 0, 0, 0, "10.4.0.216:9092", "test_topic")
dbLoadRecords("ADKafka.template", "P=$(PREFIX):, R=KFK_DRVR:, PORT=$(KFKDET_PORT), ADDR=0, TIMEOUT=1")

KafkaPluginConfigure("$(K_PORT)", 3, 1, "$(KFKDET_PORT)", 0, -1, "10.4.0.216:9092", "ad_kafka_plugin_topic")
dbLoadRecords("ADPluginKafka.template", "P=$(PREFIX):, R=KFK_PLG:,PORT=$(K_PORT),ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(KFKDET_PORT)")

iocInit

dbpf $(PREFIX):KFK_PLG:EnableCallbacks Enable
dbpf $(PREFIX):KFK_DRVR:Acquire 1

#Remember; file MUST end with a new line
