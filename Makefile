# @file Makefile
# @brief FreeBSD PMC Kernel Test Framework - Build System
# @version 1.0.0
#
# Build system for the PMC kernel test framework.
#
# Copyright (c) 2024. All rights reserved.

#============================================================================
# Configuration
#============================================================================

# Project name
PROJECT_NAME = pmc-test-framework

# Version
VERSION = 1.0.0

# Directories
SRCDIR = src
INCDIR = include
TESTDIR = tests
EXAMPLEDIR = examples
DOCDIR = docs
BUILDDIR = build

# Compiler settings
CC ?= gcc
CXX ?= g++
CFLAGS = -Wall -Wextra -Wpedantic -g -O2
CXXFLAGS = $(CFLAGS)
LDFLAGS =

# FreeBSD-specific settings
FREEBSD_CFLAGS = -DFREEBSD -D__FreeBSD__
FREEBSD_LDFLAGS = -lpmc

# Linux compatibility (for testing/development)
LINUX_CFLAGS = -DLINUX -D__linux__ -D__USE_BSD -D_POSIX_C_SOURCE=199309L -D_GNU_SOURCE
LINUX_LDFLAGS = -lrt

# Installation directories
PREFIX ?= /usr/local
INCLUDEDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib

#============================================================================
# Build Targets
#============================================================================

# Default target
.PHONY: all
all: lib framework-tests

# Build library
.PHONY: lib
lib: $(BUILDDIR)/libpmc_test_framework.a

# Build framework tests
.PHONY: framework-tests
framework-tests: $(BUILDDIR)/pmc_tests_basic $(BUILDDIR)/pmc_tests_kernel $(BUILDDIR)/pmc_tests_l3 $(BUILDDIR)/pmc_tests_uncore_df

# Build examples
.PHONY: examples
examples: $(EXAMPLEDIR)/pmc_example_simple $(EXAMPLEDIR)/pmc_example_perf

# Build documentation
.PHONY: docs
docs:
	@echo "Documentation generation..."
	@echo "Run 'make doc' from the $(DOCDIR) directory"

#============================================================================
# Source Files
#============================================================================

LIB_SOURCES = $(SRCDIR)/pmc_test_framework.c
LIB_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/src/%.o,$(LIB_SOURCES))

TEST_SOURCES = $(TESTDIR)/pmc_tests_basic.c $(TESTDIR)/pmc_tests_kernel.c \
                 $(TESTDIR)/pmc_tests_l3.c $(TESTDIR)/pmc_tests_uncore_df.c

#============================================================================
# Build Rules
#============================================================================

# Create build directory if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/src:
	mkdir -p $(BUILDDIR)/src

# Build library objects
$(BUILDDIR)/src/%.o: $(SRCDIR)/%.c | $(BUILDDIR)/src
	$(CC) $(CFLAGS) $(LINUX_CFLAGS) -I$(INCDIR) -c $< -o $@

# Build static library
$(BUILDDIR)/libpmc_test_framework.a: $(LIB_OBJECTS)
	ar rcs $@ $^
	ranlib $@

# Build basic tests
$(BUILDDIR)/pmc_tests_basic: $(TESTDIR)/pmc_tests_basic.c $(BUILDDIR)/libpmc_test_framework.a
	$(CC) $(CFLAGS) $(LINUX_CFLAGS) -I$(INCDIR) $< -L$(BUILDDIR) -lpmc_test_framework $(LINUX_LDFLAGS) -o $@

# Build kernel tests
$(BUILDDIR)/pmc_tests_kernel: $(TESTDIR)/pmc_tests_kernel.c $(BUILDDIR)/libpmc_test_framework.a
	$(CC) $(CFLAGS) $(LINUX_CFLAGS) -I$(INCDIR) $< -L$(BUILDDIR) -lpmc_test_framework $(LINUX_LDFLAGS) -o $@

# Build L3 tests
$(BUILDDIR)/pmc_tests_l3: $(TESTDIR)/pmc_tests_l3.c $(BUILDDIR)/libpmc_test_framework.a
	$(CC) $(CFLAGS) $(LINUX_CFLAGS) -I$(INCDIR) $< -L$(BUILDDIR) -lpmc_test_framework $(LINUX_LDFLAGS) -o $@

# Build Uncore DF tests
$(BUILDDIR)/pmc_tests_uncore_df: $(TESTDIR)/pmc_tests_uncore_df.c $(BUILDDIR)/libpmc_test_framework.a
	$(CC) $(CFLAGS) $(LINUX_CFLAGS) -I$(INCDIR) $< -L$(BUILDDIR) -lpmc_test_framework $(LINUX_LDFLAGS) -o $@

# Build examples
$(EXAMPLEDIR)/pmc_example_simple: $(EXAMPLEDIR)/pmc_example_simple.c $(BUILDDIR)/libpmc_test_framework.a
	$(CC) $(CFLAGS) $(LINUX_CFLAGS) -I$(INCDIR) $< -L$(BUILDDIR) -lpmc_test_framework $(LINUX_LDFLAGS) -o $@

$(EXAMPLEDIR)/pmc_example_perf: $(EXAMPLEDIR)/pmc_example_perf.c $(BUILDDIR)/libpmc_test_framework.a
	$(CC) $(CFLAGS) $(LINUX_CFLAGS) -I$(INCDIR) $< -L$(BUILDDIR) -lpmc_test_framework $(LINUX_LDFLAGS) -o $@

#============================================================================
# Testing
#============================================================================

# Run all tests
.PHONY: test
test: framework-tests
	@echo "Running PMC basic tests..."
	@$(BUILDDIR)/pmc_tests_basic
	@echo ""
	@echo "Running PMC kernel tests..."
	@$(BUILDDIR)/pmc_tests_kernel
	@echo ""
	@echo "Running PMC L3 tests..."
	@$(BUILDDIR)/pmc_tests_l3
	@echo ""
	@echo "Running PMC Uncore DF tests..."
	@$(BUILDDIR)/pmc_tests_uncore_df

# Run basic tests only
.PHONY: test-basic
test-basic: $(BUILDDIR)/pmc_tests_basic
	$(BUILDDIR)/pmc_tests_basic

# Run kernel tests only
.PHONY: test-kernel
test-kernel: $(BUILDDIR)/pmc_tests_kernel
	$(BUILDDIR)/pmc_tests_kernel

# Run L3 tests only
.PHONY: test-l3
test-l3: $(BUILDDIR)/pmc_tests_l3
	$(BUILDDIR)/pmc_tests_l3

# Run Uncore DF tests only
.PHONY: test-df
test-df: $(BUILDDIR)/pmc_tests_uncore_df
	$(BUILDDIR)/pmc_tests_uncore_df

#============================================================================
# Installation
#============================================================================

# Install library and headers
.PHONY: install
install:
	@echo "Installing $(PROJECT_NAME) v$(VERSION)..."
	@mkdir -p $(INCLUDEDIR)
	@mkdir -p $(LIBDIR)
	@cp $(INCDIR)/*.h $(INCLUDEDIR)/
	@cp $(BUILDDIR)/libpmc_test_framework.a $(LIBDIR)/
	@ranlib $(LIBDIR)/libpmc_test_framework.a
	@echo "Installation complete."

# Uninstall
.PHONY: uninstall
uninstall:
	@echo "Uninstalling $(PROJECT_NAME)..."
	@rm -f $(INCLUDEDIR)/pmc_test_framework.h
	@rm -f $(LIBDIR)/libpmc_test_framework.a
	@echo "Uninstallation complete."

#============================================================================
# Cleanup
#============================================================================

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
	rm -f $(EXAMPLEDIR)/pmc_example_simple $(EXAMPLEDIR)/pmc_example_perf
	rm -f $(BUILDDIR)/pmc_tests_l3 $(BUILDDIR)/pmc_tests_uncore_df

# Full clean including installed files
.PHONY: distclean
distclean: clean
	@rm -f $(INCLUDEDIR)/pmc_test_framework.h
	@rm -f $(LIBDIR)/libpmc_test_framework.a

#============================================================================
# Development Targets
#============================================================================

# Format code
.PHONY: format
format:
	@echo "Formatting code..."
	@find . -name '*.c' -o -name '*.h' | xargs clang-format -i -style=file

# Static analysis
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	@scan-build $(MAKE) clean all

# Build with debug symbols
.PHONY: debug
debug: CFLAGS += -DDEBUG -g -O0
debug: clean all

# Build with sanitizers
.PHONY: sanitize
sanitize: CFLAGS += -fsanitize=address -fsanitize=undefined
sanitize: LDFLAGS += -fsanitize=address -fsanitize=undefined
sanitize: clean all

#============================================================================
# Help
#============================================================================

.PHONY: help
help:
	@echo "FreeBSD PMC Kernel Test Framework - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all           - Build everything (default)"
	@echo "  lib           - Build the PMC test framework library"
	@echo "  framework-tests - Build test executables"
	@echo "  examples      - Build example programs"
	@echo "  test          - Run all tests"
	@echo "  test-basic    - Run basic tests only"
	@echo "  test-kernel   - Run kernel tests only"
	@echo "  install       - Install library and headers"
	@echo "  uninstall     - Remove installed files"
	@echo "  clean         - Remove build artifacts"
	@echo "  distclean     - Full cleanup including installation"
	@echo "  format        - Format source code"
	@echo "  analyze       - Run static analysis"
	@echo "  debug         - Build with debug symbols"
	@echo "  sanitize      - Build with address sanitizer"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  CC            - C compiler (default: gcc)"
	@echo "  CFLAGS        - C compiler flags"
	@echo "  PREFIX        - Installation prefix (default: /usr/local)"
	@echo ""

#============================================================================
# Dependencies
#============================================================================

-include $(LIB_OBJECTS:.o=.d)
