# Utilities + progress indication, please ignore.

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
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

# Makefile starts here.
include make_stuff/make_config.mk

SRC := $(call rwildcard,$(SOURCE_DIR),*.c) $(call rwildcard,$(SOURCE_DIR),*.cpp)
OBJ := $(SRC:%=$(BUILD_DIR)/%.obj)

ifeq ($(BUILD_TYPE),DEBUG)
CPPFLAGS ?= -m32 -Og -g3 -ggdb -std=c++14 -Wall -Wextra -Wpedantic
CFLAGS ?= -m32 -std=c11 -Og -g3 -ggdb -Wall -Wextra -Wpedantic
LDFLAGS ?= -m32 -Og -g3 -ggdb -shared -lkernel32 -lpsapi
else ifeq ($(BUILD_TYPE),RELEASE)
CPPFLAGS ?= -m32 -std=c++14 -march=native -mtune=native -Ofast -fno-pie -flto -ffunction-sections -fdata-sections
CFLAGS ?= -m32 -std=c11 -march=native -mtune=native -Ofast -fno-pie -flto -ffunction-sections -fdata-sections
LDFLAGS ?= -m32 -march=native -mtune=native -Ofast -static-libstdc++ -static-libgcc -fno-pie -flto -ffunction-sections -fdata-sections -shared -lkernel32 -lpsapi -Wl,--as-needed -Wl,--gc-sections -s
endif

BUILD_DIRS ?= $(sort $(dir $(OBJ)))

dll: $(TARGET_EXEC)
all: dll

$(TARGET_EXEC): $(BUILD_DIR) $(OBJ)
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

$(BUILD_DIR):
ifeq ($(OS), Windows_NT)
	@mkdir $(subst /,\,$(BUILD_DIRS))
else
	@mkdir -p $(BUILD_DIRS)
endif

# No, this is not a mistake. Leave this here, it's for progress bar
endif