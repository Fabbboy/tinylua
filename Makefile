CC ?= $(shell which gcc)
BEAR ?= $(shell which bear)
LLVM_CONFIG ?= $(shell which llvm-config)
CFLAGS := -Wall -Wextra -Werror -std=c11 -g
CFLAGS += $(shell $(LLVM_CONFIG) --cflags)
LDLIBS := $(shell $(LLVM_CONFIG) --ldflags --libs core)

SRC := src
OBJ := obj
TARGET := $(OBJ)/luac

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
DEPS := $(patsubst $(SRC)/%.c,$(OBJ)/%.d,$(SRCS))

all: prepare $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(LDLIBS) -MMD -MP -c -o $@ $< 

-include $(DEPS)

.PHONY: prepare
prepare:
	mkdir -p $(OBJ)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(TARGET)

.PHONY: run
run: all
	./$(TARGET) $(ARGS)

.PHONY: mem
mem: all
	valgrind --leak-check=full ./$(TARGET) $(ARGS)

.PHONY: cm
cm: clean
	$(BEAR) -- make

.PHONY: gdb
gdb: all
	gdb ./$(TARGET)
