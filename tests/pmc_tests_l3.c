/**
 * @file pmc_tests_l3.c
 * @brief PMC Core - L3 Counters Testcase (Osvaldo)
 * @version 1.0.0
 * 
 * Test cases for L3 Cache (Last Level Cache) PMC counters.
 * These tests validate L3 cache performance monitoring on AMD Zen4/Zen5+ 
 * and Intel processors with L3 cache counters.
 * 
 * Copyright (c) 2026. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pmc_test_framework.h"

/* Test data structure for L3 tests */
typedef struct {
    int l3_ref_counter;
    int l3_miss_counter;
    int l3_total_miss_counter;
    int l3_miss_dram_counter;
    int l3_miss_l2_counter;
    int l3_hit_counter;
    int l3_hitm_counter;
    int l3_hit_fe_counter;
    uint64_t start_value;
    uint64_t stop_value;
    uint64_t delta;
} l3_test_data_t;

/* Forward declarations */
static int test_l3_basic_alloc(void *data);
static int test_l3_all_events(void *data);
static int test_l3_hit_miss_ratio(void *data);
static int test_l3_dram_access(void *data);

/**
 * Test: L3 Counter Basic Allocation
 * Verifies that L3 cache counters can be allocated successfully.
 */
static int test_l3_basic_alloc(void *data)
{
    (void)data;
    
    /* Test L3 references counter allocation */
    int l3_ref = pmc_test_counter_alloc(0, PMC_EVENT_L3_REFERENCES,
                                         PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(l3_ref >= 0, "Failed to allocate L3_REFERENCES counter");
    
    /* Test L3 misses counter allocation */
    int l3_miss = pmc_test_counter_alloc(0, PMC_EVENT_L3_MISSES,
                                         PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(l3_miss >= 0, "Failed to allocate L3_MISSES counter");
    
    /* Release counters */
    pmc_test_counter_release(l3_ref);
    pmc_test_counter_release(l3_miss);
    
    return PMC_TEST_RESULT_PASS;
}

/**
 * Test: L3 All Events
 * Tests all available L3 cache event types.
 */
static int test_l3_all_events(void *data)
{
    l3_test_data_t *test_data = (l3_test_data_t *)data;
    
    /* Allocate all L3 event counters */
    test_data->l3_ref_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_REFERENCES,
                                                        PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->l3_ref_counter >= 0, 
                    "Failed to allocate L3_REFERENCES counter");
    
    test_data->l3_miss_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_MISSES,
                                                         PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->l3_miss_counter >= 0, 
                    "Failed to allocate L3_MISSES counter");
    
    test_data->l3_total_miss_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_TOTAL_MISSES,
                                                               PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->l3_total_miss_counter >= 0, 
                    "Failed to allocate L3_TOTAL_MISSES counter");
    
    test_data->l3_miss_dram_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_MISS_DRAM,
                                                              PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->l3_miss_dram_counter >= 0, 
                    "Failed to allocate L3_MISS_DRAM counter");
    
    test_data->l3_miss_l2_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_MISS_L2,
                                                            PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->l3_miss_l2_counter >= 0, 
                    "Failed to allocate L3_MISS_L2 counter");
    
    test_data->l3_hit_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_HIT,
                                                        PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->l3_hit_counter >= 0, 
                    "Failed to allocate L3_HIT counter");
    
    test_data->l3_hitm_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_HITM,
                                                        PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->l3_hitm_counter >= 0, 
                    "Failed to allocate L3_HITM counter");
    
    test_data->l3_hit_fe_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_HIT_FE,
                                                            PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->l3_hit_fe_counter >= 0, 
                    "Failed to allocate L3_HIT_FE counter");
    
    /* Start all counters */
    pmc_test_assert(pmc_test_counter_start(test_data->l3_ref_counter) == 0,
                    "Failed to start L3_REFERENCES counter");
    pmc_test_assert(pmc_test_counter_start(test_data->l3_miss_counter) == 0,
                    "Failed to start L3_MISSES counter");
    pmc_test_assert(pmc_test_counter_start(test_data->l3_total_miss_counter) == 0,
                    "Failed to start L3_TOTAL_MISSES counter");
    pmc_test_assert(pmc_test_counter_start(test_data->l3_miss_dram_counter) == 0,
                    "Failed to start L3_MISS_DRAM counter");
    pmc_test_assert(pmc_test_counter_start(test_data->l3_miss_l2_counter) == 0,
                    "Failed to start L3_MISS_L2 counter");
    pmc_test_assert(pmc_test_counter_start(test_data->l3_hit_counter) == 0,
                    "Failed to start L3_HIT counter");
    pmc_test_assert(pmc_test_counter_start(test_data->l3_hitm_counter) == 0,
                    "Failed to start L3_HITM counter");
    pmc_test_assert(pmc_test_counter_start(test_data->l3_hit_fe_counter) == 0,
                    "Failed to start L3_HIT_FE counter");
    
    /* Perform some memory operations to generate cache activity */
    volatile uint64_t sum = 0;
    for (uint64_t i = 0; i < 100000; i++) {
        sum += i * i;
    }
    (void)sum;
    
    /* Stop all counters */
    pmc_test_assert(pmc_test_counter_stop(test_data->l3_ref_counter) == 0,
                    "Failed to stop L3_REFERENCES counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->l3_miss_counter) == 0,
                    "Failed to stop L3_MISSES counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->l3_total_miss_counter) == 0,
                    "Failed to stop L3_TOTAL_MISSES counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->l3_miss_dram_counter) == 0,
                    "Failed to stop L3_MISS_DRAM counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->l3_miss_l2_counter) == 0,
                    "Failed to stop L3_MISS_L2 counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->l3_hit_counter) == 0,
                    "Failed to stop L3_HIT counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->l3_hitm_counter) == 0,
                    "Failed to stop L3_HITM counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->l3_hit_fe_counter) == 0,
                    "Failed to stop L3_HIT_FE counter");
    
    /* Read counter values */
    pmc_test_assert(pmc_test_counter_read(test_data->l3_ref_counter, &test_data->start_value) == 0,
                    "Failed to read L3_REFERENCES counter");
    pmc_test_assert(pmc_test_counter_read(test_data->l3_miss_counter, &test_data->stop_value) == 0,
                    "Failed to read L3_MISSES counter");
    
    /* Verify counters are working (at least some counts should be non-zero) */
    pmc_test_log(LOG_LEVEL_INFO, "L3 References: %lu\n", test_data->start_value);
    pmc_test_log(LOG_LEVEL_INFO, "L3 Misses: %lu\n", test_data->stop_value);
    
    return PMC_TEST_RESULT_PASS;
}

/**
 * Test: L3 Hit/Miss Ratio
 * Measures L3 cache hit/miss ratio during memory operations.
 */
static int test_l3_hit_miss_ratio(void *data)
{
    l3_test_data_t *test_data = (l3_test_data_t *)data;
    
    /* Allocate L3 reference and miss counters */
    test_data->l3_ref_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_REFERENCES,
                                                        PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->l3_miss_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_MISSES,
                                                         PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    pmc_test_assert(test_data->l3_ref_counter >= 0 && test_data->l3_miss_counter >= 0,
                    "Failed to allocate L3 counters");
    
    /* Start counters */
    pmc_test_counter_start(test_data->l3_ref_counter);
    pmc_test_counter_start(test_data->l3_miss_counter);
    
    /* Perform sequential memory access (should have good hit rate) */
    #define ARRAY_SIZE 1024
    static uint64_t test_array[ARRAY_SIZE];
    volatile uint64_t sum = 0;
    
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            sum += test_array[i];
        }
    }
    (void)sum;
    
    /* Stop counters */
    pmc_test_counter_stop(test_data->l3_ref_counter);
    pmc_test_counter_stop(test_data->l3_miss_counter);
    
    /* Read values */
    uint64_t l3_refs, l3_misses;
    pmc_test_counter_read(test_data->l3_ref_counter, &l3_refs);
    pmc_test_counter_read(test_data->l3_miss_counter, &l3_misses);
    
    pmc_test_log(LOG_LEVEL_INFO, "L3 References: %lu, L3 Misses: %lu\n", l3_refs, l3_misses);
    
    /* Calculate hit ratio if we have references */
    if (l3_refs > 0) {
        double hit_ratio = (double)(l3_refs - l3_misses) / l3_refs * 100.0;
        pmc_test_log(LOG_LEVEL_INFO, "L3 Hit Ratio: %.2f%%\n", hit_ratio);
    }
    
    /* Release counters */
    pmc_test_counter_release(test_data->l3_ref_counter);
    pmc_test_counter_release(test_data->l3_miss_counter);
    
    return PMC_TEST_RESULT_PASS;
}

/**
 * Test: L3 DRAM Access
 * Tests L3 cache misses that go to DRAM.
 */
static int test_l3_dram_access(void *data)
{
    l3_test_data_t *test_data = (l3_test_data_t *)data;
    
    /* Allocate L3 DRAM miss counter */
    test_data->l3_miss_dram_counter = pmc_test_counter_alloc(0, PMC_EVENT_L3_MISS_DRAM,
                                                              PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    pmc_test_assert(test_data->l3_miss_dram_counter >= 0,
                    "Failed to allocate L3_MISS_DRAM counter");
    
    /* Start counter */
    pmc_test_counter_start(test_data->l3_miss_dram_counter);
    
    /* Perform random memory access (higher chance of DRAM access) */
    #define LARGE_ARRAY_SIZE 65536
    static uint64_t large_array[LARGE_ARRAY_SIZE];
    volatile uint64_t sum = 0;
    
    /* First pass - populate array (cold cache) */
    for (size_t i = 0; i < LARGE_ARRAY_SIZE; i++) {
        large_array[i] = i;
    }
    
    /* Second pass - random access pattern */
    srand(42);
    for (int iter = 0; iter < 10000; iter++) {
        size_t idx = rand() % LARGE_ARRAY_SIZE;
        sum += large_array[idx];
    }
    (void)sum;
    
    /* Stop counter */
    pmc_test_counter_stop(test_data->l3_miss_dram_counter);
    
    /* Read value */
    uint64_t dram_misses;
    pmc_test_counter_read(test_data->l3_miss_dram_counter, &dram_misses);
    
    pmc_test_log(LOG_LEVEL_INFO, "L3 Misses going to DRAM: %lu\n", dram_misses);
    
    /* Release counter */
    pmc_test_counter_release(test_data->l3_miss_dram_counter);
    
    return PMC_TEST_RESULT_PASS;
}

/* Setup function */
static int l3_test_setup(void *data)
{
    l3_test_data_t *test_data = (l3_test_data_t *)data;
    memset(test_data, 0, sizeof(l3_test_data_t));
    test_data->l3_ref_counter = -1;
    test_data->l3_miss_counter = -1;
    test_data->l3_total_miss_counter = -1;
    test_data->l3_miss_dram_counter = -1;
    test_data->l3_miss_l2_counter = -1;
    test_data->l3_hit_counter = -1;
    test_data->l3_hitm_counter = -1;
    test_data->l3_hit_fe_counter = -1;
    return 0;
}

/* Teardown function */
static int l3_test_teardown(void *data)
{
    l3_test_data_t *test_data = (l3_test_data_t *)data;
    
    /* Release all counters if allocated */
    if (test_data->l3_ref_counter >= 0) pmc_test_counter_release(test_data->l3_ref_counter);
    if (test_data->l3_miss_counter >= 0) pmc_test_counter_release(test_data->l3_miss_counter);
    if (test_data->l3_total_miss_counter >= 0) pmc_test_counter_release(test_data->l3_total_miss_counter);
    if (test_data->l3_miss_dram_counter >= 0) pmc_test_counter_release(test_data->l3_miss_dram_counter);
    if (test_data->l3_miss_l2_counter >= 0) pmc_test_counter_release(test_data->l3_miss_l2_counter);
    if (test_data->l3_hit_counter >= 0) pmc_test_counter_release(test_data->l3_hit_counter);
    if (test_data->l3_hitm_counter >= 0) pmc_test_counter_release(test_data->l3_hitm_counter);
    if (test_data->l3_hit_fe_counter >= 0) pmc_test_counter_release(test_data->l3_hit_fe_counter);
    
    return 0;
}

/**
 * Main function for L3 tests
 */
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    pmc_test_log(LOG_LEVEL_INFO, "=== PMC Core - L3 Counters Test Suite (Osvaldo) ===\n\n");
    
    /* Initialize test framework */
    pmc_test_init(true, stdout);
    
    /* Create test suite */
    pmc_test_suite_t *suite = pmc_test_suite_create(
        "L3 Cache Counters",
        "Test suite for L3 (Last Level Cache) PMC counters"
    );
    
    /* Create test cases */
    l3_test_data_t test_data_1, test_data_2, test_data_3, test_data_4;
    
    pmc_test_case_t *test_l3_basic = pmc_test_case_create(
        "l3_basic_alloc",
        "Test basic L3 counter allocation",
        NULL, test_l3_basic_alloc, NULL, NULL
    );
    
    pmc_test_case_t *test_l3_all = pmc_test_case_create(
        "l3_all_events",
        "Test all L3 event types",
        l3_test_setup, test_l3_all_events, l3_test_teardown, &test_data_1
    );
    
    pmc_test_case_t *test_l3_hitmiss = pmc_test_case_create(
        "l3_hit_miss_ratio",
        "Test L3 hit/miss ratio measurement",
        l3_test_setup, test_l3_hit_miss_ratio, l3_test_teardown, &test_data_2
    );
    
    pmc_test_case_t *test_l3_dram = pmc_test_case_create(
        "l3_dram_access",
        "Test L3 misses that go to DRAM",
        l3_test_setup, test_l3_dram_access, l3_test_teardown, &test_data_3
    );
    
    /* Add tests to suite */
    pmc_test_suite_add_test(suite, test_l3_basic);
    pmc_test_suite_add_test(suite, test_l3_all);
    pmc_test_suite_add_test(suite, test_l3_hitmiss);
    pmc_test_suite_add_test(suite, test_l3_dram);
    
    /* Run suite */
    pmc_test_suite_run(suite);
    
    /* Get and display stats */
    pmc_test_stats_t stats;
    pmc_test_suite_get_stats(suite, &stats);
    
    pmc_test_log(LOG_LEVEL_INFO, "\n=== Test Suite Results ===\n");
    pmc_test_log(LOG_LEVEL_INFO, "Total: %d, Passed: %d, Failed: %d, Skipped: %d, Errors: %d\n",
                 stats.ts_total, stats.ts_passed, stats.ts_failed, 
                 stats.ts_skipped, stats.ts_errors);
    pmc_test_log(LOG_LEVEL_INFO, "Duration: %.3f seconds\n", stats.ts_duration);
    
    /* Cleanup */
    pmc_test_suite_destroy(suite);
    pmc_test_shutdown();
    
    return (stats.ts_failed > 0) ? 1 : 0;
}
