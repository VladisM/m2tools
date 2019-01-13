USED_LIBS = obj isa
override CFLAGS += -I inc/ -I ../../libs/inc/
SRC_DIR = src/

#-------------------------------------------------------------------------------

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))

PRG = $(addprefix m2-, $(CURRENT_DIR))

include ../../flags.mk
include ../../rules.mk
