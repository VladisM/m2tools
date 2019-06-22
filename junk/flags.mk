override WARNINGS += \
	-W \
	-Wformat-nonliteral \
	-Wcast-align \
	-Winline -Wundef \
	-Wnested-externs \
	-Wcast-qual \
	-Wwrite-strings \
	-Wunused-parameter \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wpointer-arith \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wbad-function-cast \
	-Wunused \
	-Wuninitialized \
	-Wmissing-declarations \
	-Wlogical-op \
	-Waggregate-return \
	-Wfloat-equal \
	-Wswitch-enum \
	-Wpadded \
	-Wconversion

override CFLAGS += \
	-g \
	-DDEBUG \
	--std=gnu99 \
	-ffunction-sections \
	-fdata-sections \
	$(WARNINGS)

override LFLAGS += \
	-ffunction-sections \
	-fdata-sections

BUILD_DIR = build

CC = gcc
SZ = size
