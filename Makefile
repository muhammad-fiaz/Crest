# Crest Framework Makefile
# Cross-platform build system for Crest REST API framework

# Project information
PROJECT = crest
VERSION = 1.0.0

# Compiler settings
CC = gcc
CXX = g++
AR = ar
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS = 

# Platform detection
ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    EXE_EXT = .exe
    LIB_EXT = .dll
    STATIC_EXT = .lib
    LDFLAGS += -lws2_32
    RM = del /Q
    RMDIR = rmdir /S /Q
    MKDIR = mkdir
    SEP = \\
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM = linux
        LIB_EXT = .so
        LDFLAGS += -lpthread
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM = macos
        LIB_EXT = .dylib
        LDFLAGS += -lpthread
    endif
    EXE_EXT =
    STATIC_EXT = .a
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
    SEP = /
endif

# Directories
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj
EXAMPLES_DIR = $(BUILD_DIR)/examples
INSTALL_PREFIX = /usr/local

# Source files
CORE_SOURCES = src/core/app.c src/core/config.c src/core/request.c \
               src/core/response.c src/core/server.c src/core/router.c \
               src/core/middleware.c

UTIL_SOURCES = src/utils/json.c src/utils/logger.c src/utils/string_utils.c

DASHBOARD_SOURCES = src/dashboard/dashboard.c

ALL_SOURCES = $(CORE_SOURCES) $(UTIL_SOURCES) $(DASHBOARD_SOURCES)
OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(ALL_SOURCES))

# Library names
SHARED_LIB = $(LIB_DIR)/lib$(PROJECT)$(LIB_EXT)
STATIC_LIB = $(LIB_DIR)/lib$(PROJECT)$(STATIC_EXT)

# Targets
.PHONY: all shared static examples clean install uninstall help

all: shared static examples

# Shared library
shared: $(SHARED_LIB)

$(SHARED_LIB): $(OBJECTS) | $(LIB_DIR)
	$(CC) -shared -o $@ $^ $(LDFLAGS)
	@echo "Built shared library: $(SHARED_LIB)"

# Static library
static: $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS) | $(LIB_DIR)
ifeq ($(PLATFORM),windows)
	$(AR) rcs $@ $^
else
	$(AR) rcs $@ $^
endif
	@echo "Built static library: $(STATIC_LIB)"

# Object files
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@$(MKDIR) $(dir $@) 2>nul || true
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# Examples
examples: $(EXAMPLES_DIR)/basic$(EXE_EXT) $(EXAMPLES_DIR)/advanced$(EXE_EXT)

$(EXAMPLES_DIR)/basic$(EXE_EXT): examples/basic/main.c $(STATIC_LIB) | $(EXAMPLES_DIR)
	$(CC) $(CFLAGS) $< -L$(LIB_DIR) -l$(PROJECT) $(LDFLAGS) -o $@
	@echo "Built example: $@"

$(EXAMPLES_DIR)/advanced$(EXE_EXT): examples/advanced/advanced.c $(STATIC_LIB) | $(EXAMPLES_DIR)
	$(CC) $(CFLAGS) $< -L$(LIB_DIR) -l$(PROJECT) $(LDFLAGS) -o $@
	@echo "Built example: $@"

# Create directories
$(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(OBJ_DIR) $(EXAMPLES_DIR):
	@$(MKDIR) $@ 2>nul || true

# Install
install: all
	@echo "Installing Crest framework..."
	@$(MKDIR) $(INSTALL_PREFIX)/include/crest
	@$(MKDIR) $(INSTALL_PREFIX)/lib
	@cp -r include/crest/*.h $(INSTALL_PREFIX)/include/crest/
	@cp $(SHARED_LIB) $(INSTALL_PREFIX)/lib/
	@cp $(STATIC_LIB) $(INSTALL_PREFIX)/lib/
	@echo "Installation complete!"
	@echo "Installed to: $(INSTALL_PREFIX)"

# Uninstall
uninstall:
	@echo "Uninstalling Crest framework..."
	@$(RMDIR) $(INSTALL_PREFIX)/include/crest
	@$(RM) $(INSTALL_PREFIX)/lib/lib$(PROJECT)*
	@echo "Uninstallation complete!"

# Clean
clean:
	@echo "Cleaning build artifacts..."
	@$(RMDIR) $(BUILD_DIR) 2>nul || true
	@echo "Clean complete!"

# Help
help:
	@echo "Crest Framework - Makefile Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build shared library, static library, and examples (default)"
	@echo "  shared     - Build shared library only"
	@echo "  static     - Build static library only"
	@echo "  examples   - Build example applications"
	@echo "  install    - Install libraries and headers to $(INSTALL_PREFIX)"
	@echo "  uninstall  - Remove installed files"
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Platform detected: $(PLATFORM)"
	@echo "C Compiler: $(CC)"
	@echo "Install prefix: $(INSTALL_PREFIX)"
