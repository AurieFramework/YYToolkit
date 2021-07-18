_mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
I := $(patsubst %/,%,$(dir $(_mkfile_path)))

ifneq ($(words $(MAKECMDGOALS)),1)
.DEFAULT_GOAL = all
%:
	@$(MAKE) $@ --no-print-directory -rRf $(firstword $(MAKEFILE_LIST))
else
ifndef ECHO
T := $(shell $(MAKE) $(MAKECMDGOALS) --no-print-directory -nrRf $(firstword $(MAKEFILE_LIST)) ECHO="COUNTTHIS" | grep -c "COUNTTHIS")
N := x
C = $(words $N)$(eval N := x $N)
ECHO = echo [$C/$T]
endif

# (Recursively) finds every file in a folder with given mask (super useful)
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

BUILD_DIR ?= _build
SOURCE_DIR ?= YYToolkit/Src
TARGET_EXEC ?= yytoolkit.dll

CC := gcc
CXX := g++

SRC := $(call rwildcard,$(SOURCE_DIR),*.cpp)
OBJ := $(SRC:%=$(BUILD_DIR)/%.obj)
CPPFLAGS ?= -m32 -Ofast -std=c++14
CFLAGS ?= -m32 -std=c11 -Ofast
LDFLAGS ?= -m32 -Ofast -shared -lkernel32 -lpsapi

BUILD_DIRS ?= $(sort $(dir $(OBJ)))

$(BUILD_DIR)/$(TARGET_EXEC): $(BUILD_DIR) $(OBJ)
	@$(ECHO) Linking $<
	@$(CXX) $(OBJ) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.cpp.obj: %.cpp
	@$(ECHO) Building $<
	@$(CXX) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.c.obj: %.c
	@$(ECHO) Building $<
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
ifeq ($(OS), Windows_NT)
	@rmdir /S /Q $(BUILD_DIR)
else
	@rm -rf $(BUILD_DIR)
endif

$(BUILD_DIRS):
ifeq ($(OS), Windows_NT)
	@mkdir $(subst /,\,$(BUILD_DIRS))
else
	@mkdir -p $(BUILD_DIRS)
endif

$(BUILD_DIR):
ifeq ($(OS), Windows_NT)
	@mkdir $(subst /,\,$(BUILD_DIRS))
else
	@mkdir -p $(BUILD_DIRS)
endif

endif