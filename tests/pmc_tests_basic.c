/**
 * @file pmc_tests_basic.c
 * @brief Basic PMC Test Cases
 * @version 1.0.0
 * 
 * Basic test cases for the PMC kernel test framework.
 * 
 * Copyright (c) 2026. All rights reserved.
 */

#include "pmc_test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================
 * Test Case 1: Framework Initialization
 *============================================================================*/

static int test_framework_init_setup(void *data)
{
    (void)data;
    /* No setup needed */
    return 0;
}

static int test_framework_init_run(void *data)
{
    (void)data;
    
    pmc_test_framework_t *fw = pmc_test_get_framework();
    pmc_test_assert(fw != NULL, "Framework instance should not be NULL");
    pmc_test_assert(fw->tf_initialized == true, "Framework should be initialized");
    
    return 0;
}

static int test_framework_init_teardown(void *data)
{
    (void)data;
    /* No teardown needed */
    return 0;
}

/*============================================================================
 * Test Case 2: PMC Availability
 *============================================================================*/

static int test_pmc_available_setup(void *data)
{
    (void)data;
    return 0;
}

static int test_pmc_available_run(void *data)
{
    (void)data;
    
    bool available = pmc_test_is_available();
    pmc_test_assert(available == true, "PMC should be available on this system");
    
    int num_cpus = pmc_test_get_cpu_count();
    pmc_test_assert(num_cpus > 0, "Should have at least one CPU");
    
    int num_counters = pmc_test_get_counter_count();
    pmc_test_assert(num_counters > 0, "Should have at least one PMC counter");
    
    return 0;
}

static int test_pmc_available_teardown(void *data)
{
    (void)data;
    return 0;
}

/*============================================================================
 * Test Case 3: Counter Allocation
 *============================================================================*/

typedef struct {
    int counter_id;
    int cpu;
    pmc_test_event_t event;
} test_counter_alloc_data_t;

static int test_counter_alloc_setup(void *data)
{
    test_counter_alloc_data_t *test_data = (test_counter_alloc_data_t*)data;
    test_data->cpu = 0;
    test_data->event = PMC_EVENT_CPU_CYCLES;
    test_data->counter_id = -1;
    return 0;
}

static int test_counter_alloc_run(void *data)
{
    test_counter_alloc_data_t *test_data = (test_counter_alloc_data_t*)data;
    
    int counter = pmc_test_counter_alloc(test_data->cpu, test_data->event,
                                         PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    pmc_test_assert(counter >= 0, "Counter allocation should succeed");
    
    test_data->counter_id = counter;
    
    pmc_test_counter_config_t config;
    int result = pmc_test_counter_info(counter, &config);
    pmc_test_assert(result == 0, "Getting counter info should succeed");
    
    return 0;
}

static int test_counter_alloc_teardown(void *data)
{
    test_counter_alloc_data_t *test_data = (test_counter_alloc_data_t*)data;
    
    if (test_data->counter_id >= 0) {
        pmc_test_counter_release(test_data->counter_id);
    }
    
    return 0;
}

/*============================================================================
 * Test Case 4: Counter Start/Stop
 *============================================================================*/

typedef struct {
    int counter_id;
    uint64_t start_value;
    uint64_t stop_value;
} test_counter_control_data_t;

static int test_counter_control_setup(void *data)
{
    test_counter_control_data_t *test_data = (test_counter_control_data_t*)data;
    
    test_data->counter_id = pmc_test_counter_alloc(0, PMC_EVENT_INSTRUCTIONS,
                                                   PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    if (test_data->counter_id < 0) {
        return -1;
    }
    
    return 0;
}

static int test_counter_control_run(void *data)
{
    test_counter_control_data_t *test_data = (test_counter_control_data_t*)data;
    
    /* Start counter */
    int result = pmc_test_counter_start(test_data->counter_id);
    pmc_test_assert(result == 0, "Starting counter should succeed");
    
    /* Read value */
    result = pmc_test_counter_read(test_data->counter_id, &test_data->start_value);
    pmc_test_assert(result == 0, "Reading counter should succeed");
    
    /* Do some work */
    volatile int sum = 0;
    for (int i = 0; i < 10000; i++) {
        sum += i;
    }
    (void)sum;
    
    /* Stop counter */
    result = pmc_test_counter_stop(test_data->counter_id);
    pmc_test_assert(result == 0, "Stopping counter should succeed");
    
    /* Read value after stop */
    result = pmc_test_counter_read(test_data->counter_id, &test_data->stop_value);
    pmc_test_assert(result == 0, "Reading stopped counter should succeed");
    
    /* Counter should have incremented */
    pmc_test_assert(test_data->stop_value >= test_data->start_value, 
                   "Counter value should have increased");
    
    return 0;
}

static int test_counter_control_teardown(void *data)
{
    test_counter_control_data_t *test_data = (test_counter_control_data_t*)data;
    
    if (test_data->counter_id >= 0) {
        pmc_test_counter_release(test_data->counter_id);
    }
    
    return 0;
}

/*============================================================================
 * Test Case 5: Multiple Counters
 *============================================================================*/

typedef struct {
    int counters[4];
    int num_counters;
} test_multiple_counters_data_t;

static int test_multiple_counters_setup(void *data)
{
    test_multiple_counters_data_t *test_data = (test_multiple_counters_data_t*)data;
    
    test_data->num_counters = 4;
    
    /* Allocate multiple counters with different events */
    test_data->counters[0] = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES,
                                                    PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->counters[1] = pmc_test_counter_alloc(0, PMC_EVENT_INSTRUCTIONS,
                                                    PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->counters[2] = pmc_test_counter_alloc(0, PMC_EVENT_CACHE_REFERENCES,
                                                    PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->counters[3] = pmc_test_counter_alloc(0, PMC_EVENT_BRANCH_INSTRUCTIONS,
                                                    PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    for (int i = 0; i < test_data->num_counters; i++) {
        if (test_data->counters[i] < 0) {
            return -1;
        }
    }
    
    return 0;
}

static int test_multiple_counters_run(void *data)
{
    test_multiple_counters_data_t *test_data = (test_multiple_counters_data_t*)data;
    
    /* Start all counters */
    for (int i = 0; i < test_data->num_counters; i++) {
        int result = pmc_test_counter_start(test_data->counters[i]);
        pmc_test_assert(result == 0, "Counter start failed");
    }
    
    /* Do some work */
    volatile int sum = 0;
    for (int i = 0; i < 100000; i++) {
        sum += i;
    }
    (void)sum;
    
    /* Stop all counters */
    for (int i = 0; i < test_data->num_counters; i++) {
        int result = pmc_test_counter_stop(test_data->counters[i]);
        pmc_test_assert(result == 0, "Counter stop failed");
    }
    
    /* Read all counter values */
    uint64_t values[4];
    for (int i = 0; i < test_data->num_counters; i++) {
        int result = pmc_test_counter_read(test_data->counters[i], &values[i]);
        pmc_test_assert(result == 0, "Counter read failed");
    }
    
    /* Verify that counters have values */
    for (int i = 0; i < test_data->num_counters; i++) {
        pmc_test_assert(values[i] > 0, "Counter value is zero");
    }
    
    return 0;
}

static int test_multiple_counters_teardown(void *data)
{
    test_multiple_counters_data_t *test_data = (test_multiple_counters_data_t*)data;
    
    for (int i = 0; i < test_data->num_counters; i++) {
        if (test_data->counters[i] >= 0) {
            pmc_test_counter_release(test_data->counters[i]);
        }
    }
    
    return 0;
}

/*============================================================================
 * Test Case 6: Event Mapping
 *============================================================================*/

static int test_event_mapping_run(void *data)
{
    (void)data;
    
    /* Test all event types */
    for (pmc_test_event_t event = 0; event < PMC_EVENT_MAX; event++) {
        const char *event_str = pmc_test_event_to_string(event);
        pmc_test_assert(event_str != NULL, "Event string is NULL");
        pmc_test_assert(strlen(event_str) > 0, "Event string is empty");
        
        /* Test mapping to PMC value */
        uint32_t evsel = pmc_test_map_event(event);
        pmc_test_assert(evsel != 0 || event == PMC_EVENT_CPU_CLOCK || 
                       event == PMC_EVENT_REF_CPU_CLOCK,
                       "Event mapping failed");
    }
    
    return 0;
}

/*============================================================================
 * Test Case 7: Test Suite Creation
 *============================================================================*/

static int test_suite_creation_run(void *data)
{
    (void)data;
    
    pmc_test_suite_t *suite = pmc_test_suite_create("Test Suite", 
                                                       "Test suite for verification");
    pmc_test_assert(suite != NULL, "Suite creation should succeed");
    
    pmc_test_assert(strcmp(suite->ts_name, "Test Suite") == 0, 
                   "Suite name should match");
    pmc_test_assert(strcmp(suite->ts_description, "Test suite for verification") == 0,
                   "Suite description should match");
    pmc_test_assert(suite->ts_count == 0, "Initial count should be zero");
    
    pmc_test_suite_destroy(suite);
    
    return 0;
}

/*============================================================================
 * Main Function
 *============================================================================*/

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    printf("========================================\n");
    printf("PMC Test Framework - Basic Tests\n");
    printf("========================================\n\n");
    
    /* Initialize framework */
    if (pmc_test_init(true, stdout) != 0) {
        fprintf(stderr, "Failed to initialize framework\n");
        return 1;
    }
    
    /* Check if running as root */
    if (!pmc_test_is_root()) {
        printf("Warning: Not running as root. Some tests may fail.\n");
    }
    
    /* Check PMC availability */
    if (!pmc_test_is_available()) {
        printf("Warning: PMC not available on this system\n");
    }
    
    /* Create test suite */
    pmc_test_suite_t *suite = pmc_test_suite_create("PMC Basic Tests",
                                                      "Basic tests for PMC functionality");
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        pmc_test_shutdown();
        return 1;
    }
    
    /* Add test cases */
    pmc_test_case_t *test;
    
    /* Test 1: Framework initialization */
    test = pmc_test_case_create("test_framework_init",
                                "Test framework initialization",
                                test_framework_init_setup,
                                test_framework_init_run,
                                test_framework_init_teardown,
                                NULL);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 2: PMC availability */
    test = pmc_test_case_create("test_pmc_available",
                                "Test PMC availability",
                                test_pmc_available_setup,
                                test_pmc_available_run,
                                test_pmc_available_teardown,
                                NULL);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 3: Counter allocation */
    test_counter_alloc_data_t alloc_data;
    test = pmc_test_case_create("test_counter_alloc",
                                "Test PMC counter allocation",
                                test_counter_alloc_setup,
                                test_counter_alloc_run,
                                test_counter_alloc_teardown,
                                &alloc_data);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 4: Counter control */
    test_counter_control_data_t control_data;
    test = pmc_test_case_create("test_counter_control",
                                "Test PMC counter start/stop",
                                test_counter_control_setup,
                                test_counter_control_run,
                                test_counter_control_teardown,
                                &control_data);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 5: Multiple counters */
    test_multiple_counters_data_t multi_data;
    test = pmc_test_case_create("test_multiple_counters",
                                "Test multiple PMC counters",
                                test_multiple_counters_setup,
                                test_multiple_counters_run,
                                test_multiple_counters_teardown,
                                &multi_data);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 6: Event mapping */
    test = pmc_test_case_create("test_event_mapping",
                                "Test PMC event mapping",
                                NULL,
                                test_event_mapping_run,
                                NULL,
                                NULL);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 7: Suite creation */
    test = pmc_test_case_create("test_suite_creation",
                                "Test suite creation",
                                NULL,
                                test_suite_creation_run,
                                NULL,
                                NULL);
    pmc_test_suite_add_test(suite, test);
    
    /* Run tests */
    int failed = pmc_test_suite_run(suite);
    
    /* Get statistics */
    pmc_test_stats_t stats;
    pmc_test_suite_get_stats(suite, &stats);
    
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total:    %d\n", stats.ts_total);
    printf("  Passed:   %d\n", stats.ts_passed);
    printf("  Failed:   %d\n", stats.ts_failed);
    printf("  Skipped:  %d\n", stats.ts_skipped);
    printf("  Errors:   %d\n", stats.ts_errors);
    printf("  Duration: %.3f seconds\n", stats.ts_duration);
    printf("========================================\n");
    
    /* Cleanup */
    pmc_test_suite_destroy(suite);
    pmc_test_shutdown();
    
    return (failed > 0) ? 1 : 0;
}
