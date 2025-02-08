CC?=$(shell which gcc)
BEAR:=$(shell which bear)
CFLAGS:=-Wall -Wextra -Werror -std=c11 -g

SRC:=src
OBJ:=obj
TARGET:=$(OBJ)/luac

SRCS:=$(wildcard $(SRC)/*.c)
OBJS:=$(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))

all: prepare $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: prepare
prepare:
	mkdir -p $(OBJ)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(TARGET)

.PHONY: run
run: all
	./$(TARGET)

.PHONY: mem
mem: all
	valgrind --leak-check=full ./$(TARGET)

.PHONY: cm
cm: clean
	$(BEAR) -- make

.PHONY: gdb
gdb: all
	gdb ./$(TARGET)