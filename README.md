# FreeBSD PMC Kernel Test Framework

A comprehensive testing framework for FreeBSD kernel PMC (Performance Monitoring Counters) functionality.

## Overview

The FreeBSD PMC Kernel Test Framework provides a complete testing infrastructure for validating PMC hardware and kernel subsystem functionality on FreeBSD systems. This framework enables developers and system administrators to verify that PMC counters are working correctly, test different event types, and validate kernel-level performance monitoring.

## Features

- **PMC Counter Management**: Allocate, configure, start, stop, and release PMC counters
- **Event Support**: Test multiple PMC events including CPU cycles, cache operations, branch prediction
- **Test Suite Framework**: Built-in test framework for organizing and running test cases
- **User/Kernel Mode Testing**: Support for testing both user-space and kernel-space PMC counting
- **System-wide Counting**: Ability to test system-wide performance monitoring
- **Performance Measurement Helpers**: Utility functions for easy performance measurement
- **Comprehensive Test Cases**: Pre-built test cases for basic and kernel-level functionality

## System Requirements

- FreeBSD 12.0 or later
- PMC kernel module loaded (`kldload pmc` or `kldload pmc_nhm`)
- Root privileges (required for kernel PMC access)
- C compiler (gcc or clang)

## Quick Start

### Building the Framework

```bash
# Clone the repository
git clone https://github.com/example/pmc-testing.git
cd pmc-testing

# Build the library
make lib

# Build tests
make framework-tests

# Build examples
make examples
```

### Running Tests

```bash
# Run all tests
make test

# Run basic tests only
make test-basic

# Run kernel tests only
make test-kernel
```

### Installation

```bash
# Install library and headers
sudo make install

# Default installation path: /usr/local
# Custom prefix: make install PREFIX=/opt/pmc
```

## Architecture

### Directory Structure

```
pmc-testing/
├── include/           # Header files
│   └── pmc_test_framework.h
├── src/               # Framework implementation
│   └── pmc_test_framework.c
├── tests/             # Test cases
│   ├── pmc_tests_basic.c
│   └── pmc_tests_kernel.c
├── examples/          # Example programs
├── docs/              # Documentation
├── scripts/           # Utility scripts
└── Makefile          # Build system
```

### Core Components

1. **Framework Core** (`pmc_test_framework.c`)
   - Initialization and shutdown
   - Counter management
   - Test suite execution

2. **Test Framework API**
   - Test case creation and execution
   - Test suite management
   - Statistics collection

3. **Performance Measurement**
   - Counter allocation and configuration
   - Start/stop control
   - Value reading and delta calculation

## Usage Examples

### Basic Counter Usage

```c
#include "pmc_test_framework.h"

// Initialize framework
pmc_test_init(true, stdout);

// Allocate a counter
int counter = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES,
                                      PMC_MODE_COUNT, PMC_FLAG_ALL);

// Start counting
pmc_test_counter_start(counter);

// ... perform work ...

// Stop counting
pmc_test_counter_stop(counter);

// Read the value
uint64_t value;
pmc_test_counter_read(counter, &value);

printf("CPU cycles: %lu\n", value);

// Release counter
pmc_test_counter_release(counter);

// Shutdown framework
pmc_test_shutdown();
```

### Creating a Test Case

```c
static int my_test_setup(void *data) {
    // Initialize test resources
    return 0;
}

static int my_test_run(void *data) {
    // Test logic
    pmc_test_assert(some_condition, "Test failed");
    return 0;
}

static int my_test_teardown(void *data) {
    // Cleanup resources
    return 0;
}

// Create test case
pmc_test_case_t *test = pmc_test_case_create(
    "my_test",
    "Description of my test",
    my_test_setup,
    my_test_run,
    my_test_teardown,
    NULL
);

// Add to suite and run
pmc_test_suite_add_test(suite, test);
pmc_test_suite_run(suite);
```

### Using Performance Measurement Helpers

```c
// Allocate counters
int counters[2];
counters[0] = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES, ...);
counters[1] = pmc_test_counter_alloc(0, PMC_EVENT_INSTRUCTIONS, ...);

uint64_t start_values[2], stop_values[2], deltas[2];

// Start measurement
pmc_test_measure_start(counters, 2);

// ... do work ...

// Stop and read
pmc_test_measure_stop(counters, 2, stop_values);

// Calculate deltas
pmc_test_calculate_delta(start_values, stop_values, deltas, 2);

printf("CPU cycles: %lu\n", deltas[0]);
printf("Instructions: %lu\n", deltas[1]);
```

## Available PMC Events

| Event | Description |
|-------|-------------|
| `PMC_EVENT_CPU_CYCLES` | Unhalted CPU cycles |
| `PMC_EVENT_INSTRUCTIONS` | Retired instructions |
| `PMC_EVENT_CACHE_REFERENCES` | Cache references |
| `PMC_EVENT_CACHE_MISSES` | Cache misses |
| `PMC_EVENT_BRANCH_INSTRUCTIONS` | Branch instructions |
| `PMC_EVENT_BRANCH_MISSES` | Branch mispredictions |
| `PMC_EVENT_BUS_CYCLES` | Bus cycles |
| `PMC_EVENT_STALLED_CYCLES` | Resource stalls |
| `PMC_EVENT_MEMORY_REQUESTS` | Memory requests |
| `PMC_EVENT_CONTEXT_SWITCHES` | Context switches |
| `PMC_EVENT_PAGE_FAULTS` | Page faults |

## API Reference

### Framework Functions

- `pmc_test_init()` - Initialize the test framework
- `pmc_test_shutdown()` - Shutdown the test framework
- `pmc_test_get_framework()` - Get framework instance

### Counter Functions

- `pmc_test_counter_alloc()` - Allocate a PMC counter
- `pmc_test_counter_release()` - Release a counter
- `pmc_test_counter_start()` - Start counting
- `pmc_test_counter_stop()` - Stop counting
- `pmc_test_counter_read()` - Read counter value

### Test Suite Functions

- `pmc_test_suite_create()` - Create a test suite
- `pmc_test_suite_add_test()` - Add test to suite
- `pmc_test_suite_run()` - Run all tests in suite
- `pmc_test_suite_destroy()` - Free suite resources

## Troubleshooting

### "PMC not available" Error

Ensure the PMC kernel module is loaded:

```bash
sudo kldload pmc
# or for Intel Nehalem and later
sudo kldload pmc_nhm
```

### Permission Denied

Most PMC operations require root privileges:

```bash
sudo ./build/pmc_tests_basic
```

### Counter Allocation Fails

- Check available counters: `sysctl hw.pmcounters`
- Verify CPU supports PMC: `sysctl kern.pm_present`

## Development

### Building with Debug Symbols

```bash
make debug
```

### Building with Sanitizers

```bash
make sanitize
```

### Code Formatting

```bash
make format
```

## Contributing

Contributions are welcome! Please follow these guidelines:

1. Follow the existing code style
2. Add tests for new functionality
3. Update documentation
4. Ensure tests pass before submitting

## License

Copyright (c) 2026. All rights reserved.

## References

- [FreeBSD PMC Manual](https://www.freebsd.org/cgi/man.cgi?query=pmc)
- [Intel SDM - Performance Monitoring](https://software.intel.com/en-us/articles/intel-sdm)
- [AMD BIOS and Kernel Developer Guide - PMU](https://www.amd.com/)

## Contact

For issues and feature requests, please open an issue on the project repository.
