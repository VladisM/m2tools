override CFLAGS += -I inc/
SRC_DIR = src/
INSTALL_LIB_PATH = /usr/lib/

#-------------------------------------------------------------------------------

include ../flags.mk

override CFLAGS += -fPIC

SRC := $(notdir $(shell find $(SRC_DIR) -name '*.c'))
LIBS := $(addprefix $(BUILD_DIR)/, $(SRC:.c=.so))

all: $(BUILD_DIR) $(LIBS)

$(BUILD_DIR)/%.so: src/%.c
	$(CC) $(CFLAGS) -c $< -o $(@:.so=.o)
	$(CC) -shared $(@:.so=.o) -o $@
	chmod 644 $@

$(BUILD_DIR):
	mkdir -p $@

install: $(BUILD_DIR) $(LIBS)
	cp $(BUILD_DIR)/*.so $(INSTALL_LIB_PATH)

uninstall:
	rm $(addprefix $(INSTALL_LIB_PATH), $(notdir $(LIBS)))

.PHONY: clean doc uninstall

clean:
	rm -rf $(BUILD_DIR) doc/

doc:
	doxygen Doxyfile
