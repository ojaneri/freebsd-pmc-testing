# PMC Test Framework API Documentation

## Table of Contents

1. [Framework Core](#framework-core)
2. [Counter Management](#counter-management)
3. [Test Suite API](#test-suite-api)
4. [Test Case API](#test-case-api)
5. [Utility Functions](#utility-functions)
6. [Data Structures](#data-structures)

---

## Framework Core

### pmc_test_init

```c
int pmc_test_init(bool verbose, FILE *output);
```

Initialize the PMC test framework.

**Parameters:**
- `verbose` - Enable verbose output
- `output` - Output file handle (NULL for stdout)

**Returns:** 0 on success, -1 on error

**Example:**
```c
pmc_test_init(true, stdout);
```

---

### pmc_test_shutdown

```c
int pmc_test_shutdown(void);
```

Shutdown the PMC test framework and release resources.

**Returns:** 0 on success, -1 on error

---

### pmc_test_get_framework

```c
pmc_test_framework_t* pmc_test_get_framework(void);
```

Get the framework singleton instance.

**Returns:** Pointer to framework context

---

### pmc_test_is_available

```c
bool pmc_test_is_available(void);
```

Check if PMC hardware is available on the system.

**Returns:** true if available, false otherwise

---

### pmc_test_get_cpu_count

```c
int pmc_test_get_cpu_count(void);
```

Get the number of available CPUs.

**Returns:** Number of CPUs

---

### pmc_test_get_counter_count

```c
int pmc_test_get_counter_count(void);
```

Get the number of available PMC counters.

**Returns:** Number of PMC counters

---

### pmc_test_is_root

```c
bool pmc_test_is_root(void);
```

Check if running with root privileges.

**Returns:** true if running as root

---

### pmc_test_kernel_supported

```c
bool pmc_test_kernel_supported(void);
```

Check if kernel PMC support is available.

**Returns:** true if supported

---

## Counter Management

### pmc_test_counter_alloc

```c
int pmc_test_counter_alloc(int cpu, pmc_test_event_t event, 
                          pmc_test_mode_t mode, pmc_test_flags_t flags);
```

Allocate a PMC counter.

**Parameters:**
- `cpu` - CPU ID (0 for first CPU)
- `event` - PMC event type to measure
- `mode` - PMC mode (counting or sampling)
- `flags` - Counter flags (user/kernel mode)

**Returns:** Counter index on success, -1 on error

**Example:**
```c
int counter = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES,
                                     PMC_MODE_COUNT, PMC_FLAG_ALL);
```

---

### pmc_test_counter_release

```c
int pmc_test_counter_release(int counter);
```

Release a previously allocated counter.

**Parameters:**
- `counter` - Counter index

**Returns:** 0 on success, -1 on error

---

### pmc_test_counter_start

```c
int pmc_test_counter_start(int counter);
```

Start counting on a counter.

**Parameters:**
- `counter` - Counter index

**Returns:** 0 on success, -1 on error

---

### pmc_test_counter_stop

```c
int pmc_test_counter_stop(int counter);
```

Stop counting on a counter.

**Parameters:**
- `counter` - Counter index

**Returns:** 0 on success, -1 on error

---

### pmc_test_counter_read

```c
int pmc_test_counter_read(int counter, uint64_t *value);
```

Read the current value of a counter.

**Parameters:**
- `counter` - Counter index
- `value` - Pointer to store counter value

**Returns:** 0 on success, -1 on error

---

### pmc_test_counter_configure

```c
int pmc_test_counter_configure(int counter, pmc_test_counter_config_t *config);
```

Configure a counter with specific parameters.

**Parameters:**
- `counter` - Counter index
- `config` - Configuration structure

**Returns:** 0 on success, -1 on error

---

### pmc_test_counter_info

```c
int pmc_test_counter_info(int counter, pmc_test_counter_config_t *config);
```

Get counter configuration information.

**Parameters:**
- `counter` - Counter index
- `config` - Pointer to store configuration

**Returns:** 0 on success, -1 on error

---

## Test Suite API

### pmc_test_suite_create

```c
pmc_test_suite_t* pmc_test_suite_create(const char *name, const char *description);
```

Create a new test suite.

**Parameters:**
- `name` - Suite name
- `description` - Suite description

**Returns:** Pointer to test suite on success, NULL on error

---

### pmc_test_suite_destroy

```c
int pmc_test_suite_destroy(pmc_test_suite_t *suite);
```

Destroy a test suite and free resources.

**Parameters:**
- `suite` - Test suite to destroy

**Returns:** 0 on success, -1 on error

---

### pmc_test_suite_add_test

```c
int pmc_test_suite_add_test(pmc_test_suite_t *suite, pmc_test_case_t *test);
```

Add a test case to a test suite.

**Parameters:**
- `suite` - Test suite
- `test` - Test case to add

**Returns:** 0 on success, -1 on error

---

### pmc_test_suite_run

```c
int pmc_test_suite_run(pmc_test_suite_t *suite);
```

Run all tests in a test suite.

**Parameters:**
- `suite` - Test suite to run

**Returns:** Number of failed tests

---

### pmc_test_suite_get_stats

```c
int pmc_test_suite_get_stats(pmc_test_suite_t *suite, pmc_test_stats_t *stats);
```

Get test suite statistics.

**Parameters:**
- `suite` - Test suite
- `stats` - Pointer to store statistics

**Returns:** 0 on success, -1 on error

---

## Test Case API

### pmc_test_case_create

```c
pmc_test_case_t* pmc_test_case_create(const char *name, const char *description,
                                      int (*setup)(void *),
                                      int (*run)(void *),
                                      int (*teardown)(void *),
                                      void *data);
```

Create a new test case.

**Parameters:**
- `name` - Test name
- `description` - Test description
- `setup` - Setup function (can be NULL)
- `run` - Test execution function
- `teardown` - Teardown function (can be NULL)
- `data` - Test data pointer

**Returns:** Pointer to test case on success, NULL on error

---

### pmc_test_case_destroy

```c
int pmc_test_case_destroy(pmc_test_case_t *test);
```

Destroy a test case.

**Parameters:**
- `test` - Test case to destroy

**Returns:** 0 on success, -1 on error

---

### pmc_test_case_run

```c
pmc_test_result_t pmc_test_case_run(pmc_test_case_t *test);
```

Run a single test case.

**Parameters:**
- `test` - Test case to run

**Returns:** Test result status

---

### pmc_test_assert

```c
#define pmc_test_assert(condition, message)
```

Assertion macro for test cases. If condition is false, returns PMC_TEST_RESULT_FAIL.

**Parameters:**
- `condition` - Boolean condition to check
- `message` - Error message (optional)

---

## Utility Functions

### pmc_test_event_to_string

```c
const char* pmc_test_event_to_string(pmc_test_event_t event);
```

Convert PMC event to string.

**Parameters:**
- `event` - PMC event type

**Returns:** Event string

---

### pmc_test_result_to_string

```c
const char* pmc_test_result_to_string(pmc_test_result_t result);
```

Convert test result to string.

**Parameters:**
- `result` - Test result

**Returns:** Result string

---

### pmc_test_log

```c
void pmc_test_log(int level, const char *format, ...);
```

Log a message with specified level.

**Parameters:**
- `level` - Log level (LOG_LEVEL_ERROR, LOG_LEVEL_WARN, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG)
- `format` - Format string
- `...` - Format arguments

---

### pmc_test_measure_start

```c
int pmc_test_measure_start(int *counters, int num_counters);
```

Start multiple counters for performance measurement.

**Parameters:**
- `counters` - Array of counter indices
- `num_counters` - Number of counters

**Returns:** 0 on success, -1 on error

---

### pmc_test_measure_stop

```c
int pmc_test_measure_stop(int *counters, int num_counters, uint64_t *values);
```

Stop multiple counters and read values.

**Parameters:**
- `counters` - Array of counter indices
- `num_counters` - Number of counters
- `values` - Array to store counter values

**Returns:** 0 on success, -1 on error

---

### pmc_test_calculate_delta

```c
void pmc_test_calculate_delta(uint64_t *start, uint64_t *end, 
                               uint64_t *delta, int count);
```

Calculate differences between start and end values.

**Parameters:**
- `start` - Start values array
- `end` - End values array
- `delta` - Output array for deltas
- `count` - Number of values

---

### pmc_test_map_event

```c
uint32_t pmc_test_map_event(pmc_test_event_t event);
```

Map test event to PMC hardware event select value.

**Parameters:**
- `event` - Test event type

**Returns:** PMC event select value

---

## Data Structures

### pmc_test_event_t

```c
typedef enum {
    PMC_EVENT_CPU_CYCLES = 0,
    PMC_EVENT_INSTRUCTIONS,
    PMC_EVENT_CACHE_REFERENCES,
    PMC_EVENT_CACHE_MISSES,
    PMC_EVENT_BRANCH_INSTRUCTIONS,
    PMC_EVENT_BRANCH_MISSES,
    PMC_EVENT_BUS_CYCLES,
    PMC_EVENT_STALLED_CYCLES,
    PMC_EVENT_MEMORY_REQUESTS,
    PMC_EVENT_MEMORY_READS,
    PMC_EVENT_MEMORY_WRITES,
    PMC_EVENT_CONTEXT_SWITCHES,
    PMC_EVENT_CPU_MIGRATIONS,
    PMC_EVENT_PAGE_FAULTS,
    PMC_EVENT_CPU_CLOCK,
    PMC_EVENT_REF_CPU_CLOCK,
    PMC_EVENT_MAX
} pmc_test_event_t;
```

### pmc_test_mode_t

```c
typedef enum {
    PMC_MODE_COUNT = 0,       /* Count mode */
    PMC_MODE_SAMPLE,          /* Sampling mode */
    PMC_MODE_CCS,             /* Cycle counting mode */
    PMC_MODE_MAX
} pmc_test_mode_t;
```

### pmc_test_flags_t

```c
typedef enum {
    PMC_FLAG_NONE = 0,
    PMC_FLAG_USER = PMC_FM_USER,         /* Monitor user space */
    PMC_FLAG_KERNEL = PMC_FM_KERNEL,      /* Monitor kernel space */
    PMC_FLAG_EVNTSELECT = 0xFF,           /* Event selection mask */
    PMC_FLAG_ALL = PMC_FM_USER | PMC_FM_KERNEL
} pmc_test_flags_t;
```

### pmc_test_result_t

```c
typedef enum {
    PMC_TEST_RESULT_PASS = 0,
    PMC_TEST_RESULT_FAIL,
    PMC_TEST_RESULT_SKIP,
    PMC_TEST_RESULT_ERROR,
    PMC_TEST_RESULT_TIMEOUT,
    PMC_RESULT_MAX
} pmc_test_result_t;
```

### pmc_test_counter_config_t

```c
typedef struct pmc_test_counter_config {
    int                     pc_cpu;          /* CPU ID */
    enum pmc_caps           pc_cap;           /* Counter capabilities */
    enum pmc_flavor         pc_flavor;        /* PMC flavor */
    uint32_t                pc_evsel;         /* Event select register */
    uint32_t                pc_config;        /* Configuration */
    pmc_test_flags_t        pc_flags;         /* Flags */
    int                     pc_count;        /* Counter index */
} pmc_test_counter_config_t;
```

### pmc_test_stats_t

```c
typedef struct pmc_test_stats {
    int                     ts_total;
    int                     ts_passed;
    int                     ts_failed;
    int                     ts_skipped;
    int                     ts_errors;
    double                  ts_duration;
    struct timespec         ts_start;
    struct timespec         ts_end;
} pmc_test_stats_t;
```

---

## Example: Complete Test Suite

```c
#include "pmc_test_framework.h"
#include <stdio.h>
#include <stdlib.h>

static int test_example_setup(void *data) {
    return 0;
}

static int test_example_run(void *data) {
    /* Check PMC is available */
    pmc_test_assert(pmc_test_is_available() == true, 
                   "PMC should be available");
    
    /* Allocate counter */
    int counter = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES,
                                         PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(counter >= 0, "Counter allocation should succeed");
    
    /* Start counting */
    pmc_test_counter_start(counter);
    
    /* Do some work */
    volatile int sum = 0;
    for (int i = 0; i < 10000; i++) {
        sum += i;
    }
    
    /* Stop counting */
    pmc_test_counter_stop(counter);
    
    /* Read value */
    uint64_t value;
    pmc_test_counter_read(counter, &value);
    
    pmc_test_assert(value > 0, "Counter should have recorded cycles");
    
    /* Release counter */
    pmc_test_counter_release(counter);
    
    return 0;
}

static int test_example_teardown(void *data) {
    return 0;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    /* Initialize framework */
    pmc_test_init(true, stdout);
    
    /* Check permissions */
    if (!pmc_test_is_root()) {
        printf("Warning: Not running as root\n");
    }
    
    /* Create test suite */
    pmc_test_suite_t *suite = pmc_test_suite_create(
        "Example Suite",
        "Example test suite demonstrating API usage"
    );
    
    /* Create and add test case */
    pmc_test_case_t *test = pmc_test_case_create(
        "test_example",
        "Example test case",
        test_example_setup,
        test_example_run,
        test_example_teardown,
        NULL
    );
    pmc_test_suite_add_test(suite, test);
    
    /* Run tests */
    int failed = pmc_test_suite_run(suite);
    
    /* Get and print statistics */
    pmc_test_stats_t stats;
    pmc_test_suite_get_stats(suite, &stats);
    
    printf("\nResults: %d passed, %d failed, %.3f seconds\n",
           stats.ts_passed, stats.ts_failed, stats.ts_duration);
    
    /* Cleanup */
    pmc_test_suite_destroy(suite);
    pmc_test_shutdown();
    
    return (failed > 0) ? 1 : 0;
}
```
