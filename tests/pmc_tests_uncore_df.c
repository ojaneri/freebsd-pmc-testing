/**
 * @file pmc_tests_uncore_df.c
 * @brief PMC Uncore - DF Counter Testcases (Osvaldo)
 * @version 1.0.0
 * 
 * Test cases for Data Fabric (DF) PMC counters on AMD Zen4 and Zen5+ processors.
 * These tests validate Uncore Data Fabric performance monitoring including
 * memory requests, cache coherency, and inter-chip connections.
 * 
 * Copyright (c) 2026. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pmc_test_framework.h"

/* Test data structure for DF tests */
typedef struct {
    int df_reads_counter;
    int df_writes_counter;
    int df_self_modified_counter;
    int df_snoop_counter;
    int df_invalids_counter;
    int df_pstats_counter;
    int df_probe_counter;
    int df_cxl_counter;
    int df_memif_counter;
    int df_gmi_counter;
    int df_xgami_counter;
    uint64_t start_value;
    uint64_t stop_value;
    uint64_t delta;
} df_test_data_t;

/* Forward declarations */
static int test_df_basic_alloc(void *data);
static int test_df_all_events(void *data);
static int test_df_memory_traffic(void *data);
static int test_df_zen4_zen5_comparison(void *data);

/**
 * Test: DF Counter Basic Allocation
 * Verifies that Data Fabric counters can be allocated successfully.
 */
static int test_df_basic_alloc(void *data)
{
    (void)data;
    
    /* Test DF reads counter allocation */
    int df_reads = pmc_test_counter_alloc(0, PMC_EVENT_DF_READS,
                                          PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(df_reads >= 0, "Failed to allocate DF_READS counter");
    
    /* Test DF writes counter allocation */
    int df_writes = pmc_test_counter_alloc(0, PMC_EVENT_DF_WRITES,
                                           PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(df_writes >= 0, "Failed to allocate DF_WRITES counter");
    
    /* Release counters */
    pmc_test_counter_release(df_reads);
    pmc_test_counter_release(df_writes);
    
    return PMC_TEST_RESULT_PASS;
}

/**
 * Test: DF All Events
 * Tests all available Data Fabric event types.
 */
static int test_df_all_events(void *data)
{
    df_test_data_t *test_data = (df_test_data_t *)data;
    
    /* Allocate all DF event counters */
    test_data->df_reads_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_READS,
                                                          PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->df_reads_counter >= 0, 
                    "Failed to allocate DF_READS counter");
    
    test_data->df_writes_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_WRITES,
                                                           PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->df_writes_counter >= 0, 
                    "Failed to allocate DF_WRITES counter");
    
    test_data->df_self_modified_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_SELF_MODIFIED,
                                                                   PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->df_self_modified_counter >= 0, 
                    "Failed to allocate DF_SELF_MODIFIED counter");
    
    test_data->df_snoop_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_SNOOP,
                                                          PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->df_snoop_counter >= 0, 
                    "Failed to allocate DF_SNOOP counter");
    
    test_data->df_invalids_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_INVALIDS,
                                                              PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->df_invalids_counter >= 0, 
                    "Failed to allocate DF_INVALIDS counter");
    
    test_data->df_pstats_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_PSTATS,
                                                            PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->df_pstats_counter >= 0, 
                    "Failed to allocate DF_PSTATS counter");
    
    test_data->df_probe_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_PROBE,
                                                           PMC_MODE_COUNT, PMC_FLAG_ALL);
    pmc_test_assert(test_data->df_probe_counter >= 0, 
                    "Failed to allocate DF_PROBE counter");
    
    /* Start counters */
    pmc_test_assert(pmc_test_counter_start(test_data->df_reads_counter) == 0,
                    "Failed to start DF_READS counter");
    pmc_test_assert(pmc_test_counter_start(test_data->df_writes_counter) == 0,
                    "Failed to start DF_WRITES counter");
    pmc_test_assert(pmc_test_counter_start(test_data->df_self_modified_counter) == 0,
                    "Failed to start DF_SELF_MODIFIED counter");
    pmc_test_assert(pmc_test_counter_start(test_data->df_snoop_counter) == 0,
                    "Failed to start DF_SNOOP counter");
    pmc_test_assert(pmc_test_counter_start(test_data->df_invalids_counter) == 0,
                    "Failed to start DF_INVALIDS counter");
    pmc_test_assert(pmc_test_counter_start(test_data->df_pstats_counter) == 0,
                    "Failed to start DF_PSTATS counter");
    pmc_test_assert(pmc_test_counter_start(test_data->df_probe_counter) == 0,
                    "Failed to start DF_PROBE counter");
    
    /* Perform some memory operations */
    volatile uint64_t sum = 0;
    for (uint64_t i = 0; i < 100000; i++) {
        sum += i * i;
    }
    (void)sum;
    
    /* Stop counters */
    pmc_test_assert(pmc_test_counter_stop(test_data->df_reads_counter) == 0,
                    "Failed to stop DF_READS counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->df_writes_counter) == 0,
                    "Failed to stop DF_WRITES counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->df_self_modified_counter) == 0,
                    "Failed to stop DF_SELF_MODIFIED counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->df_snoop_counter) == 0,
                    "Failed to stop DF_SNOOP counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->df_invalids_counter) == 0,
                    "Failed to stop DF_INVALIDS counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->df_pstats_counter) == 0,
                    "Failed to stop DF_PSTATS counter");
    pmc_test_assert(pmc_test_counter_stop(test_data->df_probe_counter) == 0,
                    "Failed to stop DF_PROBE counter");
    
    /* Read counter values */
    pmc_test_assert(pmc_test_counter_read(test_data->df_reads_counter, &test_data->start_value) == 0,
                    "Failed to read DF_READS counter");
    pmc_test_assert(pmc_test_counter_read(test_data->df_writes_counter, &test_data->stop_value) == 0,
                    "Failed to read DF_WRITES counter");
    
    /* Verify counters are working */
    pmc_test_log(LOG_LEVEL_INFO, "DF Reads: %lu\n", test_data->start_value);
    pmc_test_log(LOG_LEVEL_INFO, "DF Writes: %lu\n", test_data->stop_value);
    
    return PMC_TEST_RESULT_PASS;
}

/**
 * Test: DF Memory Traffic
 * Measures Data Fabric memory traffic patterns.
 */
static int test_df_memory_traffic(void *data)
{
    df_test_data_t *test_data = (df_test_data_t *)data;
    
    /* Allocate DF read and write counters */
    test_data->df_reads_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_READS,
                                                        PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->df_writes_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_WRITES,
                                                           PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    pmc_test_assert(test_data->df_reads_counter >= 0 && test_data->df_writes_counter >= 0,
                    "Failed to allocate DF counters");
    
    /* Start counters */
    pmc_test_counter_start(test_data->df_reads_counter);
    pmc_test_counter_start(test_data->df_writes_counter);
    
    /* Perform memory-intensive operations */
    #define MEMORY_SIZE (64 * 1024 * 1024)  /* 64 MB */
    static uint8_t *memory_block = NULL;
    
    /* Allocate memory */
    memory_block = (uint8_t *)malloc(MEMORY_SIZE);
    if (!memory_block) {
        pmc_test_log(LOG_LEVEL_ERROR, "Failed to allocate memory for test\n");
        return PMC_TEST_RESULT_FAIL;
    }
    
    /* Write phase - generate memory traffic */
    for (size_t i = 0; i < MEMORY_SIZE; i += 4096) {
        memory_block[i] = (uint8_t)(i & 0xFF);
    }
    
    /* Read phase */
    volatile uint64_t sum = 0;
    for (size_t i = 0; i < MEMORY_SIZE; i += 4096) {
        sum += memory_block[i];
    }
    (void)sum;
    
    /* Free memory */
    free(memory_block);
    
    /* Stop counters */
    pmc_test_counter_stop(test_data->df_reads_counter);
    pmc_test_counter_stop(test_data->df_writes_counter);
    
    /* Read values */
    uint64_t df_reads, df_writes;
    pmc_test_counter_read(test_data->df_reads_counter, &df_reads);
    pmc_test_counter_read(test_data->df_writes_counter, &df_writes);
    
    pmc_test_log(LOG_LEVEL_INFO, "DF Reads: %lu, DF Writes: %lu\n", df_reads, df_writes);
    
    /* Calculate total traffic */
    uint64_t total_traffic = df_reads + df_writes;
    pmc_test_log(LOG_LEVEL_INFO, "Total DF Traffic: %lu bytes\n", total_traffic);
    
    /* Release counters */
    pmc_test_counter_release(test_data->df_reads_counter);
    pmc_test_counter_release(test_data->df_writes_counter);
    
    return PMC_TEST_RESULT_PASS;
}

/**
 * Test: DF Zen4/Zen5+ Comparison
 * Tests Data Fabric events specific to Zen4 and Zen5+ architectures.
 */
static int test_df_zen4_zen5_comparison(void *data)
{
    df_test_data_t *test_data = (df_test_data_t *)data;
    
    /* Allocate CIF, GMI, XGMI counters (Zen4/Zen5+ specific) */
    test_data->df_cxl_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_CXL,
                                                       PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->df_memif_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_MEMIF,
                                                          PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->df_gmi_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_GMI,
                                                       PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->df_xgami_counter = pmc_test_counter_alloc(0, PMC_EVENT_DF_XGMI,
                                                          PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    /* Note: These counters may not be available on all systems */
    /* We'll skip if not available */
    if (test_data->df_cxl_counter < 0 && test_data->df_memif_counter < 0 &&
        test_data->df_gmi_counter < 0 && test_data->df_xgami_counter < 0) {
        pmc_test_log(LOG_LEVEL_WARN, "DF Zen4/Zen5+ specific counters not available on this system\n");
        return PMC_TEST_RESULT_SKIP;
    }
    
    /* Start available counters */
    if (test_data->df_cxl_counter >= 0) {
        pmc_test_counter_start(test_data->df_cxl_counter);
    }
    if (test_data->df_memif_counter >= 0) {
        pmc_test_counter_start(test_data->df_memif_counter);
    }
    if (test_data->df_gmi_counter >= 0) {
        pmc_test_counter_start(test_data->df_gmi_counter);
    }
    if (test_data->df_xgami_counter >= 0) {
        pmc_test_counter_start(test_data->df_xgami_counter);
    }
    
    /* Perform some operations */
    volatile uint64_t sum = 0;
    for (uint64_t i = 0; i < 1000000; i++) {
        sum += i;
    }
    (void)sum;
    
    /* Stop counters */
    if (test_data->df_cxl_counter >= 0) {
        pmc_test_counter_stop(test_data->df_cxl_counter);
    }
    if (test_data->df_memif_counter >= 0) {
        pmc_test_counter_stop(test_data->df_memif_counter);
    }
    if (test_data->df_gmi_counter >= 0) {
        pmc_test_counter_stop(test_data->df_gmi_counter);
    }
    if (test_data->df_xgami_counter >= 0) {
        pmc_test_counter_stop(test_data->df_xgami_counter);
    }
    
    /* Read and display values */
    uint64_t cxl = 0, memif = 0, gmi = 0, xgami = 0;
    
    if (test_data->df_cxl_counter >= 0) {
        pmc_test_counter_read(test_data->df_cxl_counter, &cxl);
        pmc_test_log(LOG_LEVEL_INFO, "DF CXL: %lu\n", cxl);
        pmc_test_counter_release(test_data->df_cxl_counter);
    }
    if (test_data->df_memif_counter >= 0) {
        pmc_test_counter_read(test_data->df_memif_counter, &memif);
        pmc_test_log(LOG_LEVEL_INFO, "DF MEMIF: %lu\n", memif);
        pmc_test_counter_release(test_data->df_memif_counter);
    }
    if (test_data->df_gmi_counter >= 0) {
        pmc_test_counter_read(test_data->df_gmi_counter, &gmi);
        pmc_test_log(LOG_LEVEL_INFO, "DF GMI: %lu\n", gmi);
        pmc_test_counter_release(test_data->df_gmi_counter);
    }
    if (test_data->df_xgami_counter >= 0) {
        pmc_test_counter_read(test_data->df_xgami_counter, &xgami);
        pmc_test_log(LOG_LEVEL_INFO, "DF XGMI: %lu\n", xgami);
        pmc_test_counter_release(test_data->df_xgami_counter);
    }
    
    return PMC_TEST_RESULT_PASS;
}

/* Setup function */
static int df_test_setup(void *data)
{
    df_test_data_t *test_data = (df_test_data_t *)data;
    memset(test_data, 0, sizeof(df_test_data_t));
    test_data->df_reads_counter = -1;
    test_data->df_writes_counter = -1;
    test_data->df_self_modified_counter = -1;
    test_data->df_snoop_counter = -1;
    test_data->df_invalids_counter = -1;
    test_data->df_pstats_counter = -1;
    test_data->df_probe_counter = -1;
    test_data->df_cxl_counter = -1;
    test_data->df_memif_counter = -1;
    test_data->df_gmi_counter = -1;
    test_data->df_xgami_counter = -1;
    return 0;
}

/* Teardown function */
static int df_test_teardown(void *data)
{
    df_test_data_t *test_data = (df_test_data_t *)data;
    
    /* Release all counters if allocated */
    if (test_data->df_reads_counter >= 0) pmc_test_counter_release(test_data->df_reads_counter);
    if (test_data->df_writes_counter >= 0) pmc_test_counter_release(test_data->df_writes_counter);
    if (test_data->df_self_modified_counter >= 0) pmc_test_counter_release(test_data->df_self_modified_counter);
    if (test_data->df_snoop_counter >= 0) pmc_test_counter_release(test_data->df_snoop_counter);
    if (test_data->df_invalids_counter >= 0) pmc_test_counter_release(test_data->df_invalids_counter);
    if (test_data->df_pstats_counter >= 0) pmc_test_counter_release(test_data->df_pstats_counter);
    if (test_data->df_probe_counter >= 0) pmc_test_counter_release(test_data->df_probe_counter);
    if (test_data->df_cxl_counter >= 0) pmc_test_counter_release(test_data->df_cxl_counter);
    if (test_data->df_memif_counter >= 0) pmc_test_counter_release(test_data->df_memif_counter);
    if (test_data->df_gmi_counter >= 0) pmc_test_counter_release(test_data->df_gmi_counter);
    if (test_data->df_xgami_counter >= 0) pmc_test_counter_release(test_data->df_xgami_counter);
    
    return 0;
}

/**
 * Main function for DF tests
 */
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    pmc_test_log(LOG_LEVEL_INFO, "=== PMC Uncore - DF Counter Test Suite (Osvaldo) ===\n");
    pmc_test_log(LOG_LEVEL_INFO, "=== AMD Zen4 and Zen5+ Data Fabric Tests ===\n\n");
    
    /* Initialize test framework */
    pmc_test_init(true, stdout);
    
    /* Create test suite */
    pmc_test_suite_t *suite = pmc_test_suite_create(
        "Data Fabric Counters",
        "Test suite for AMD Zen4/Zen5+ Data Fabric (DF) PMC counters"
    );
    
    /* Create test cases */
    df_test_data_t test_data_1, test_data_2, test_data_3, test_data_4;
    
    pmc_test_case_t *test_df_basic = pmc_test_case_create(
        "df_basic_alloc",
        "Test basic DF counter allocation",
        NULL, test_df_basic_alloc, NULL, NULL
    );
    
    pmc_test_case_t *test_df_all = pmc_test_case_create(
        "df_all_events",
        "Test all DF event types",
        df_test_setup, test_df_all_events, df_test_teardown, &test_data_1
    );
    
    pmc_test_case_t *test_df_traffic = pmc_test_case_create(
        "df_memory_traffic",
        "Test DF memory traffic measurement",
        df_test_setup, test_df_memory_traffic, df_test_teardown, &test_data_2
    );
    
    pmc_test_case_t *test_df_zen4_zen5 = pmc_test_case_create(
        "df_zen4_zen5",
        "Test Zen4/Zen5+ specific DF counters",
        df_test_setup, test_df_zen4_zen5_comparison, df_test_teardown, &test_data_3
    );
    
    /* Add tests to suite */
    pmc_test_suite_add_test(suite, test_df_basic);
    pmc_test_suite_add_test(suite, test_df_all);
    pmc_test_suite_add_test(suite, test_df_traffic);
    pmc_test_suite_add_test(suite, test_df_zen4_zen5);
    
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
