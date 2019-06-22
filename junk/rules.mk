SRC := $(notdir $(shell find $(SRC_DIR) -name '*.c'))
OBJ := $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))

all: $(BUILD_DIR) $(OBJ) $(BUILD_DIR)/$(PRG)

$(BUILD_DIR)/$(PRG): $(OBJ)
	$(CC) $(LFLAGS) -o $@ $^ $(addprefix -lm2-, $(USED_LIBS))
	$(SZ) $@

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
