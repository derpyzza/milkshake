CC=clang
SRC=src
OBJ=obj
PROG_NAME = libmatrix

TEST := test/bin/test

CFLAGS := -Wall -pedantic -g --std=c11

all: $(OBJ) debug

$(OBJ):
	mkdir -p ./$(OBJ)

INCLUDE_FILES = -I$(SRC) -Iinclude

# SRC_FILES := $(SRC)/matrix.c
# SRC_FILES += $(addprefix $(SRC)/types/matrix_, vec2.c vec3.c vec4.c mat.c)
SRC_FILES := $(addprefix $(SRC)/types/matrix_, vec2.c vec3.c vec4.c)
OBJ_FILES := $(addprefix $(OBJ)/, $(notdir $(SRC_FILES:.c=.o)))

check: $(PROG_NAME).a
	make -C test test

debug: CFLAGS += -DMATRIX_DEBUG_FUNCTIONS
debug: OBJ_FILES += $(OBJ)/matrix_debug.o
debug: $(OBJ)/matrix_debug.o $(OBJ_FILES)

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
