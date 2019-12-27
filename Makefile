# For TMote Sky (emulated in Cooja) use the following target
TARGET ?= sky

DEFINES=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = app

PROJECT_SOURCEFILES += nd.c
PROJECT_SOURCEFILES += nd-rdc.c netstack.c nd-netstack.c

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_RIME=0
CONTIKI_WITH_IPV6=0
CONTIKI ?= ../../contiki
include $(CONTIKI)/Makefile.include
