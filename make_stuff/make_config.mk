# Aviable: DEBUG, RELEASE (case sensitive)
BUILD_TYPE := DEBUG

# Output directory.
BUILD_DIR ?= _build
TARGET_EXEC ?=  $(BUILD_DIR)/yytoolkit.dll
SOURCE_DIR ?= YYToolkit/Src

CC := gcc
CXX := g++