TOPDIR=$(shell pwd)

#include $(TOPDIR)/common.mk


LDFLAGS += -L/usr/lib/x86_64-linux-gnu -lpthread
SUBDIRS:=

ALL_TARGETS =
INSTALL_TARGETS =
CLEAN_TARGETS =
DISTCLEAN_TARGETS =

CPPFLAGS += -Iinclude -std=c++11

all: real-all

include src/main.mk
include boardtest/boardTest.mk

real-all: $(ALL_TARGETS)

install: $(INSTALL_TARGETS)


clean: $(CLEAN_TARGETS)

