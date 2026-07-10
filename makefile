PLATFORM     ?= linux
ANDROID_ARCH ?= aarch64
ANDROID_API  ?= 21

# Per-platform output dirs so all three can coexist without cleaning
OBJ       := ./obj/$(PLATFORM)
PROG_NAME := libmilkshake
PROG      := ./out/$(PLATFORM)/$(PROG_NAME).a

ifeq ($(PLATFORM), linux)
    CC          := clang
    AR          := ar
    TARGET      := x86_64-linux-gnu

else ifeq ($(PLATFORM), windows)
    CC          := clang
    AR          := llvm-ar
    TARGET      := x86_64-w64-mingw32
    # Set MINGW_SYSROOT if clang cannot locate mingw headers automatically,
    # e.g. MINGW_SYSROOT=/usr/x86_64-w64-mingw32
    ifdef MINGW_SYSROOT
        SYSROOT := --sysroot=$(MINGW_SYSROOT)
    endif

else ifeq ($(PLATFORM), android)
    ifndef ANDROID_NDK
        $(error ANDROID_NDK is not set — export it to your NDK root directory)
    endif
    _NDK_TC := $(ANDROID_NDK)/toolchains/llvm/prebuilt/linux-x86_64
    CC       := $(_NDK_TC)/bin/clang
    AR       := $(_NDK_TC)/bin/llvm-ar
    SYSROOT  := --sysroot=$(_NDK_TC)/sysroot

    ifeq ($(ANDROID_ARCH), aarch64)
        TARGET := aarch64-linux-android$(ANDROID_API)
    else ifeq ($(ANDROID_ARCH), armv7a)
        TARGET := armv7a-linux-androideabi$(ANDROID_API)
    else ifeq ($(ANDROID_ARCH), x86_64)
        TARGET := x86_64-linux-android$(ANDROID_API)
    else
        $(error Unknown ANDROID_ARCH "$(ANDROID_ARCH)" — use aarch64, armv7a, or x86_64)
    endif

else
    $(error Unknown PLATFORM "$(PLATFORM)" — use linux, windows, or android)
endif

CFLAGS   := -Wall -Wextra -pedantic -fno-common
CFLAGS   += --target=$(TARGET) $(SYSROOT)
CFLAGS   += -DDLOG_USE_COLOR -DDLOG_COLORS
DEPFLAGS := -MMD -MP

SDL3_INCLUDE ?= /usr/include/SDL3

INCLUDES := -I include/
INCLUDES += -I libs/
INCLUDES += -I libs/glad/include
INCLUDES += -I libs/cglm/include
INCLUDES += -I libs/libderp/
INCLUDES += -I $(SDL3_INCLUDE)

# Android ships GL ES, not desktop GL; its headers live in the NDK sysroot
ifneq ($(PLATFORM), android)
    # Desktop GL headers — override GL_INCLUDE if non-standard
    GL_INCLUDE ?= /usr/include
    INCLUDES   += -I $(GL_INCLUDE)
endif

SRC_FILES    := $(shell find src -name '*.c')
SRC_OBJS     := $(patsubst src/%.c, $(OBJ)/src/%.o, $(SRC_FILES))

GLAD_SRC     := libs/glad/src/glad.c
GLAD_OBJ     := $(OBJ)/libs/glad/src/glad.o

EXTERN_SRC   := libs/extern.c
EXTERN_OBJ   := $(OBJ)/libs/extern.o

LIBDERP_SRC  := $(wildcard libs/libderp/*.c)
LIBDERP_OBJS := $(patsubst libs/libderp/%.c, $(OBJ)/libderp/%.o, $(LIBDERP_SRC))

ALL_OBJS := $(SRC_OBJS) $(GLAD_OBJ) $(EXTERN_OBJ) $(LIBDERP_OBJS)
DEP_FILES := $(ALL_OBJS:.o=.d)

LSAN_OPTIONS := suppressions=lsan.supp
export LSAN_OPTIONS

.PHONY: all debug release clean

all: debug

debug: CFLAGS += -ggdb -O0 -DDEBUG=1
# debug: CFLAGS += -fsanitize=address,undefined
debug: $(PROG)

release: CFLAGS += -O2 -DDEBUG=0
# release: clean $(PROG)

install:
	@test -f $(PROG) || (echo "Error: Build the project first using 'make release' before installing." && exit 1)
	cp -r $(PROG) /usr/local/lib/
	mkdir -p /usr/local/include/milkshake/
	cp -r include/milkshake/* /usr/local/include/milkshake/

$(PROG): $(ALL_OBJS)
	@mkdir -p $(@D)
	@echo "  AR  $@"
	$(AR) rcs $@ $^


$(OBJ)/src/%.o: src/%.c
	@mkdir -p $(@D)
	@echo "  CC  $<"
	$(CC) -c $< -o $@ $(CFLAGS) $(DEPFLAGS) $(INCLUDES)

$(OBJ)/libs/glad/src/glad.o: libs/glad/src/glad.c
	@mkdir -p $(@D)
	@echo "  CC  $<"
	$(CC) -c $< -o $@ $(CFLAGS) $(DEPFLAGS) $(INCLUDES)

$(OBJ)/libs/extern.o: libs/extern.c
	@mkdir -p $(@D)
	@echo "  CC  $<"
	$(CC) -c $< -o $@ $(CFLAGS) $(DEPFLAGS) $(INCLUDES)

$(OBJ)/libderp/%.o: libs/libderp/%.c
	@mkdir -p $(@D)
	@echo "  CC  $<"
	$(CC) -c $< -o $@ $(CFLAGS) $(DEPFLAGS) $(INCLUDES)

-include $(DEP_FILES)

clean:
	rm -rf obj/$(PLATFORM) out/$(PLATFORM)

clean-all:
	rm -rf obj/ out/
