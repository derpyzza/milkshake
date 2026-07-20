PLATFORM     ?= linux

BUILD_DIR := ./build
OBJ_DIR   := obj
OBJ       := $(BUILD_DIR)/$(OBJ_DIR)/$(PLATFORM)
PROG_NAME := libmilkshake
PROG      := $(BUILD_DIR)/$(PLATFORM)/$(PROG_NAME).a

ifeq ($(PLATFORM), linux)
    CC          := clang
    AR          := ar
    TARGET      := x86_64-linux-gnu
	SDL3_INCLUDE := /usr/include/SDL3

    EX_LDFLAGS  := `pkg-config --cflags --libs sdl3 gl` -lm
else ifeq ($(PLATFORM), windows)
    CC          := clang
    AR          := llvm-ar
    TARGET      := x86_64-w64-mingw32

	SDL3_INCLUDE := /usr/x86_64-w64-mingw32/sys-root/mingw/include/SDL3

    EX_LDFLAGS  := -lSDL3 -lopengl32
    
    # Set MINGW_SYSROOT if clang cannot locate mingw headers automatically,
    # e.g. MINGW_SYSROOT=/usr/x86_64-w64-mingw32
    ifdef MINGW_SYSROOT
        SYSROOT := --sysroot=$(MINGW_SYSROOT)
    endif
else
    $(error Unknown PLATFORM "$(PLATFORM)" — use linux, windows, or android)
endif

CFLAGS   := -Wall -Wextra -pedantic -fno-common
CFLAGS   += --target=$(TARGET) $(SYSROOT)
CFLAGS   += -DDLOG_USE_COLOR -DDLOG_COLORS
DEPFLAGS := -MMD -MP

INCLUDES := -I include/
INCLUDES += -I libs/
INCLUDES += -I libs/glad/include
INCLUDES += -I libs/cglm/include
INCLUDES += -I libs/libderp/
INCLUDES += -I $(SDL3_INCLUDE)

SRC_FILES    := $(shell find src -name '*.c')
SRC_OBJS     := $(patsubst src/%.c, $(OBJ)/src/%.o, $(SRC_FILES))

GLAD_SRC     := libs/glad/src/glad.c
GLAD_OBJ     := $(OBJ)/libs/glad.o

EXTERN_SRC   := libs/extern.c
EXTERN_OBJ   := $(OBJ)/libs/extern.o

LIBDERP_SRC  := $(wildcard libs/libderp/*.c)
LIBDERP_OBJS := $(patsubst libs/libderp/%.c, $(OBJ)/libderp/%.o, $(LIBDERP_SRC))

ALL_OBJS := $(SRC_OBJS) $(GLAD_OBJ) $(EXTERN_OBJ) $(LIBDERP_OBJS)
DEP_FILES := $(ALL_OBJS:.o=.d)

LSAN_OPTIONS := suppressions=lsan.supp
export LSAN_OPTIONS

.PHONY: all debug release clean

all: debug examples

debug: CFLAGS += -ggdb -O0 -DDEBUG=1
# debug: CFLAGS += -fsanitize=address,undefined
debug: $(PROG)

release: CFLAGS += -O2 -DDEBUG=0 -g -ggdb
release: $(PROG)

install:
	@test -f $(PROG) || (echo "Error: Build the project first using 'make release' before installing." && exit 1)
	cp -r $(PROG) /usr/local/lib/
	mkdir -p /usr/local/include/milkshake/
	cp -r include/milkshake/* /usr/local/include/milkshake/

$(PROG): $(ALL_OBJS)
	@mkdir -p $(@D)
	@echo "  AR  $@"
	$(AR) rcs $@ $^

EX_DIR := ./examples
EX_OUT := $(BUILD_DIR)/$(PLATFORM)/examples

EX_SRC := $(shell find $(EX_DIR) -name "*.c")
EX_BIN := $(patsubst $(EX_DIR)/%.c,$(EX_OUT)/%,$(EX_SRC))

$(EX_BIN): $(EX_OUT)/% : $(EX_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $< -o $@ $(CFLAGS) $(INCLUDES) $(EX_LDFLAGS) $(PROG)

examples: $(PROG) $(EX_BIN) 
	@echo $(EX_BIN)

$(OBJ)/src/%.o: src/%.c
	@mkdir -p $(@D)
	@echo "  CC  $<"
	$(CC) -c $< -o $@ $(CFLAGS) $(DEPFLAGS) $(INCLUDES)

$(GLAD_OBJ): libs/glad/src/glad.c
	@mkdir -p $(@D)
	@echo "  CC  $<"
	$(CC) -c $< -o $@ $(CFLAGS) $(DEPFLAGS) $(INCLUDES)

$(EXTERN_OBJ): libs/extern.c
	@mkdir -p $(@D)
	@echo "  CC  $<"
	$(CC) -c $< -o $@ $(CFLAGS) $(DEPFLAGS) $(INCLUDES)

$(OBJ)/libderp/%.o: libs/libderp/%.c
	@mkdir -p $(@D)
	@echo "  CC  $<"
	$(CC) -c $< -o $@ $(CFLAGS) $(DEPFLAGS) $(INCLUDES)

-include $(DEP_FILES)

clean:
	rm -rf $(OBJ) $(BUILD_DIR)/$(PLATFORM)

clean-all:
	rm -rf $(BUILD_DIR)/*
