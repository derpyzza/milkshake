CC=clang

SRC=src
OBJ=obj
PROG_NAME = libmilkshake

TEST := test/bin/test

CFLAGS := -Wall -pedantic -Wextra -fno-common
CFLAGS += -DDLOG_USE_COLOR
CFLAGS += -DDLOG_COLORS

GLAD_SRC := libs/glad/src/glad.c
GLAD_OBJ := $(BIN_DIR)/libs/glad/src/glad.o
EXTERN_SRC := ./libs/extern.c
EXTERN_OBJ := ./libs/extern.o
LIBDERP := ./libs/libderp/libderp.a

DEPFLAGS := -MMD -MP

LDFLAGS := -lSDL3 -lGL -lm 
LDFLAGS += $(LIBDERP)

INCLUDE_DIR := -I include/
INCLUDE_DIR += -I ./libs/
INCLUDE_DIR += -I ./libs/glad/include
INCLUDE_DIR += -I ./libs/cglm/include
INCLUDE_DIR += -I ./libs/libderp/

LSAN_OPTIONS := suppressions=lsan.supp
export LSAN_OPTIONS

SRC_FILES := $(shell find $(SRC) -name '*.c')
OBJ_FILES := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRC_FILES))

INCLUDE_FILES = -I$(SRC) -Iinclude

PROG := $(PROG_NAME).a

all: debug

run: debug

check: $(PROG_NAME).a
	make -C test test

debug: CFLAGS += -ggdb -O0 -DDEBUG=1
debug: CFLAGS += -fsanitize=address
debug: $(PROG)

release: CFLAGS += -O2
release: clean

debug release: $(PROG_NAME).a

$(PROG_NAME).a: $(OBJ_FILES)
	ar rcs $@ $(OBJ_FILES)

$(OBJ)/%.o: $(SRC)/%.c
	@echo building $@...
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_FILES)

$(OBJ)/%.o: $(SRC)/*/%.c
	@echo building $@...
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_FILES)

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c $(TEST_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_FILES)

clean:
	rm -rf $(OBJ)/* $(PROG_NAME).a test/bin/*

.PHONY: all clean
