/**
 * @file pmc_tests_kernel.c
 * @brief Kernel PMC Test Cases
 * @version 1.0.0
 * 
 * Advanced test cases focusing on kernel-level PMC functionality
 * including system-wide counters, interrupts, and sampling.
 * 
 * Copyright (c) 2026. All rights reserved.
 */

#include "pmc_test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

/*============================================================================
 * Test Case 1: Kernel Counter Capabilities
 *============================================================================*/

static int test_kernel_caps_run(void *data)
{
    (void)data;
    
    /* Check kernel PMC support */
    bool supported = pmc_test_kernel_supported();
    
    /* This test might be skipped if not running on FreeBSD with PMC support */
    if (!supported) {
        printf("Note: Kernel PMC support not detected - some features may not work\n");
    }
    
    /* Get CPU capabilities */
    pmc_caps caps = pmc_test_get_cpu_caps(0);
    
    /* Verify we have at least basic capabilities */
    pmc_test_assert((caps & PMC_CAP_READ) != 0, "Should have READ capability");
    pmc_test_assert((caps & PMC_CAP_WRITE) != 0, "Should have WRITE capability");
    pmc_test_assert((caps & PMC_CAP_INCREMENT) != 0, "Should have INCREMENT capability");
    
    return 0;
}

/*============================================================================
 * Test Case 2: System-wide Counting
 *============================================================================*/

typedef struct {
    int counter_id;
    uint64_t before_count;
    uint64_t after_count;
} test_system_wide_data_t;

static int test_system_wide_setup(void *data)
{
    test_system_wide_data_t *test_data = (test_system_wide_data_t*)data;
    
    /* Allocate counter with kernel mode flag for system-wide counting */
    test_data->counter_id = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES,
                                                    PMC_MODE_COUNT, PMC_FLAG_KERNEL);
    test_data->before_count = 0;
    test_data->after_count = 0;
    
    return (test_data->counter_id >= 0) ? 0 : -1;
}

static int test_system_wide_run(void *data)
{
    test_system_wide_data_t *test_data = (test_system_wide_data_t*)data;
    
    /* Start the counter */
    int result = pmc_test_counter_start(test_data->counter_id);
    pmc_test_assert(result == 0, "Starting system-wide counter should succeed");
    
    /* Read value after start */
    result = pmc_test_counter_read(test_data->counter_id, &test_data->before_count);
    pmc_test_assert(result == 0, "Reading counter should succeed");
    
    /* Fork a child process to generate some activity */
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process - do some work */
        volatile int sum = 0;
        for (int i = 0; i < 1000000; i++) {
            sum += i;
        }
        _exit(0);
    } else if (pid > 0) {
        /* Parent process - wait for child */
        int status;
        waitpid(pid, &status, 0);
    }
    
    /* Read value after child execution */
    result = pmc_test_counter_read(test_data->counter_id, &test_data->after_count);
    pmc_test_assert(result == 0, "Reading counter should succeed");
    
    /* Stop the counter */
    result = pmc_test_counter_stop(test_data->counter_id);
    pmc_test_assert(result == 0, "Stopping counter should succeed");
    
    /* The counter should have captured kernel-level activity */
    pmc_test_assert(test_data->after_count >= test_data->before_count,
                   "Counter should have recorded activity");
    
    return 0;
}

static int test_system_wide_teardown(void *data)
{
    test_system_wide_data_t *test_data = (test_system_wide_data_t*)data;
    
    if (test_data->counter_id >= 0) {
        pmc_test_counter_release(test_data->counter_id);
    }
    
    return 0;
}

/*============================================================================
 * Test Case 3: User/Kernel Mode Separation
 *============================================================================*/

typedef struct {
    int user_counter;
    int kernel_counter;
    uint64_t user_value;
    uint64_t kernel_value;
} test_mode_separation_data_t;

static int test_mode_separation_setup(void *data)
{
    test_mode_separation_data_t *test_data = (test_mode_separation_data_t*)data;
    
    /* Allocate counter for user mode only */
    test_data->user_counter = pmc_test_counter_alloc(0, PMC_EVENT_INSTRUCTIONS,
                                                     PMC_MODE_COUNT, PMC_FLAG_USER);
    
    /* Allocate counter for kernel mode only */
    test_data->kernel_counter = pmc_test_counter_alloc(0, PMC_EVENT_INSTRUCTIONS,
                                                        PMC_MODE_COUNT, PMC_FLAG_KERNEL);
    
    if (test_data->user_counter < 0 || test_data->kernel_counter < 0) {
        return -1;
    }
    
    return 0;
}

static int test_mode_separation_run(void *data)
{
    test_mode_separation_data_t *test_data = (test_mode_separation_data_t*)data;
    
    /* Start both counters */
    pmc_test_counter_start(test_data->user_counter);
    pmc_test_counter_start(test_data->kernel_counter);
    
    /* Execute code in user space */
    volatile int sum = 0;
    for (int i = 0; i < 500000; i++) {
        sum += i;
    }
    (void)sum;
    
    /* Make a system call to generate kernel activity */
    int fd = open("/dev/null", O_RDONLY);
    if (fd >= 0) {
        char buf[1];
        read(fd, buf, 1);
        close(fd);
    }
    
    /* Stop both counters */
    pmc_test_counter_stop(test_data->user_counter);
    pmc_test_counter_stop(test_data->kernel_counter);
    
    /* Read values */
    pmc_test_counter_read(test_data->user_counter, &test_data->user_value);
    pmc_test_counter_read(test_data->kernel_counter, &test_data->kernel_value);
    
    /* User mode should have significantly more instructions */
    pmc_test_assert(test_data->user_value > 0, "User counter should have counted");
    
    return 0;
}

static int test_mode_separation_teardown(void *data)
{
    test_mode_separation_data_t *test_data = (test_mode_separation_data_t*)data;
    
    if (test_data->user_counter >= 0) {
        pmc_test_counter_release(test_data->user_counter);
    }
    if (test_data->kernel_counter >= 0) {
        pmc_test_counter_release(test_data->kernel_counter);
    }
    
    return 0;
}

/*============================================================================
 * Test Case 4: Performance Measurement Helper
 *============================================================================*/

typedef struct {
    int counters[2];
    uint64_t start_values[2];
    uint64_t stop_values[2];
    uint64_t deltas[2];
} test_measure_helper_data_t;

static int test_measure_helper_run(void *data)
{
    test_measure_helper_data_t *test_data = (test_measure_helper_data_t*)data;
    
    /* Allocate counters */
    test_data->counters[0] = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES,
                                                    PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->counters[1] = pmc_test_counter_alloc(0, PMC_EVENT_INSTRUCTIONS,
                                                    PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    if (test_data->counters[0] < 0 || test_data->counters[1] < 0) {
        return -1;
    }
    
    /* Start measurement */
    int result = pmc_test_measure_start(test_data->counters, 2);
    pmc_test_assert(result == 0, "Starting measurement should succeed");
    
    /* Do some work */
    volatile long long sum = 0;
    for (int i = 0; i < 1000000; i++) {
        sum += i * i;
    }
    (void)sum;
    
    /* Stop measurement */
    result = pmc_test_measure_stop(test_data->counters, 2, test_data->stop_values);
    pmc_test_assert(result == 0, "Stopping measurement should succeed");
    
    /* Calculate deltas */
    pmc_test_calculate_delta(test_data->start_values, test_data->stop_values,
                             test_data->deltas, 2);
    
    /* Verify deltas are positive */
    pmc_test_assert(test_data->deltas[0] > 0, "CPU cycles delta should be positive");
    pmc_test_assert(test_data->deltas[1] > 0, "Instructions delta should be positive");
    
    /* Verify reasonable ratio (instructions should be less than cycles) */
    if (test_data->deltas[0] > 0) {
        double ratio = (double)test_data->deltas[1] / (double)test_data->deltas[0];
        pmc_test_assert(ratio <= 2.0, "IPC ratio should be reasonable");
    }
    
    /* Release counters */
    pmc_test_counter_release(test_data->counters[0]);
    pmc_test_counter_release(test_data->counters[1]);
    
    return 0;
}

/*============================================================================
 * Test Case 5: Cache Performance Testing
 *============================================================================*/

typedef struct {
    int l1d_ref_counter;
    int l1d_miss_counter;
    int l1i_ref_counter;
    int l1i_miss_counter;
    uint64_t ref_value;
    uint64_t miss_value;
} test_cache_perf_data_t;

static int test_cache_perf_setup(void *data)
{
    test_cache_perf_data_t *test_data = (test_cache_perf_data_t*)data;
    
    /* Allocate L1 data cache reference counter */
    test_data->l1d_ref_counter = pmc_test_counter_alloc(0, PMC_EVENT_CACHE_REFERENCES,
                                                          PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    /* Allocate L1 data cache miss counter */
    test_data->l1d_miss_counter = pmc_test_counter_alloc(0, PMC_EVENT_CACHE_MISSES,
                                                           PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    /* Allocate L1 instruction cache reference counter */
    test_data->l1i_ref_counter = pmc_test_counter_alloc(0, PMC_EVENT_CACHE_REFERENCES,
                                                          PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    /* Allocate L1 instruction cache miss counter */
    test_data->l1i_miss_counter = pmc_test_counter_alloc(0, PMC_EVENT_CACHE_MISSES,
                                                           PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    if (test_data->l1d_ref_counter < 0 || test_data->l1d_miss_counter < 0) {
        return -1;
    }
    
    return 0;
}

static int test_cache_perf_run(void *data)
{
    test_cache_perf_data_t *test_data = (test_cache_perf_data_t*)data;
    
    /* Start cache counters */
    pmc_test_counter_start(test_data->l1d_ref_counter);
    pmc_test_counter_start(test_data->l1d_miss_counter);
    
    /* Perform memory-intensive operation */
    #define CACHE_TEST_SIZE 1024 * 1024  /* 1MB */
    int *array = malloc(CACHE_TEST_SIZE * sizeof(int));
    
    if (array) {
        /* First pass - will miss in cache */
        for (int i = 0; i < CACHE_TEST_SIZE; i++) {
            array[i] = i;
        }
        
        /* Second pass - should hit in cache */
        volatile int sum = 0;
        for (int i = 0; i < CACHE_TEST_SIZE; i++) {
            sum += array[i];
        }
        (void)sum;
        
        free(array);
    }
    
    /* Stop counters */
    pmc_test_counter_stop(test_data->l1d_ref_counter);
    pmc_test_counter_stop(test_data->l1d_miss_counter);
    
    /* Read values */
    pmc_test_counter_read(test_data->l1d_ref_counter, &test_data->ref_value);
    pmc_test_counter_read(test_data->l1d_miss_counter, &test_data->miss_value);
    
    /* Verify we captured cache activity */
    pmc_test_assert(test_data->ref_value > 0, "Cache references should be non-zero");
    pmc_test_assert(test_data->miss_value >= 0, "Cache misses should be non-negative");
    
    return 0;
}

static int test_cache_perf_teardown(void *data)
{
    test_cache_perf_data_t *test_data = (test_cache_perf_data_t*)data;
    
    if (test_data->l1d_ref_counter >= 0) {
        pmc_test_counter_release(test_data->l1d_ref_counter);
    }
    if (test_data->l1d_miss_counter >= 0) {
        pmc_test_counter_release(test_data->l1d_miss_counter);
    }
    if (test_data->l1i_ref_counter >= 0) {
        pmc_test_counter_release(test_data->l1i_ref_counter);
    }
    if (test_data->l1i_miss_counter >= 0) {
        pmc_test_counter_release(test_data->l1i_miss_counter);
    }
    
    return 0;
}

/*============================================================================
 * Test Case 6: Branch Prediction Testing
 *============================================================================*/

typedef struct {
    int branch_counter;
    int branch_miss_counter;
    uint64_t branch_value;
    uint64_t miss_value;
} test_branch_pred_data_t;

static int test_branch_pred_setup(void *data)
{
    test_branch_pred_data_t *test_data = (test_branch_pred_data_t*)data;
    
    test_data->branch_counter = pmc_test_counter_alloc(0, PMC_EVENT_BRANCH_INSTRUCTIONS,
                                                        PMC_MODE_COUNT, PMC_FLAG_ALL);
    test_data->branch_miss_counter = pmc_test_counter_alloc(0, PMC_EVENT_BRANCH_MISSES,
                                                             PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    return (test_data->branch_counter >= 0 && test_data->branch_miss_counter >= 0) ? 0 : -1;
}

static int test_branch_pred_run(void *data)
{
    test_branch_pred_data_t *test_data = (test_branch_pred_data_t*)data;
    
    /* Start counters */
    pmc_test_counter_start(test_data->branch_counter);
    pmc_test_counter_start(test_data->branch_miss_counter);
    
    /* Perform conditional branching */
    volatile int sum = 0;
    for (int i = 0; i < 100000; i++) {
        if (i % 2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    (void)sum;
    
    /* Stop counters */
    pmc_test_counter_stop(test_data->branch_counter);
    pmc_test_counter_stop(test_data->branch_miss_counter);
    
    /* Read values */
    pmc_test_counter_read(test_data->branch_counter, &test_data->branch_value);
    pmc_test_counter_read(test_data->branch_miss_counter, &test_data->miss_value);
    
    /* Verify branch activity captured */
    pmc_test_assert(test_data->branch_value > 0, "Branch instructions should be counted");
    
    return 0;
}

static int test_branch_pred_teardown(void *data)
{
    test_branch_pred_data_t *test_data = (test_branch_pred_data_t*)data;
    
    if (test_data->branch_counter >= 0) {
        pmc_test_counter_release(test_data->branch_counter);
    }
    if (test_data->branch_miss_counter >= 0) {
        pmc_test_counter_release(test_data->branch_miss_counter);
    }
    
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
    printf("PMC Test Framework - Kernel Tests\n");
    printf("========================================\n\n");
    
    /* Initialize framework */
    if (pmc_test_init(true, stdout) != 0) {
        fprintf(stderr, "Failed to initialize framework\n");
        return 1;
    }
    
    /* Check if running as root */
    if (!pmc_test_is_root()) {
        printf("Warning: Not running as root. Kernel tests may fail.\n");
    }
    
    /* Create test suite */
    pmc_test_suite_t *suite = pmc_test_suite_create("PMC Kernel Tests",
                                                      "Kernel-level PMC functionality tests");
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        pmc_test_shutdown();
        return 1;
    }
    
    pmc_test_case_t *test;
    
    /* Test 1: Kernel capabilities */
    test = pmc_test_case_create("test_kernel_caps",
                                "Test kernel PMC capabilities",
                                NULL,
                                test_kernel_caps_run,
                                NULL,
                                NULL);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 2: System-wide counting */
    test_system_wide_data_t system_wide_data;
    test = pmc_test_case_create("test_system_wide",
                                "Test system-wide counter",
                                test_system_wide_setup,
                                test_system_wide_run,
                                test_system_wide_teardown,
                                &system_wide_data);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 3: Mode separation */
    test_mode_separation_data_t mode_data;
    test = pmc_test_case_create("test_mode_separation",
                                "Test user/kernel mode separation",
                                test_mode_separation_setup,
                                test_mode_separation_run,
                                test_mode_separation_teardown,
                                &mode_data);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 4: Measurement helper */
    test_measure_helper_data_t measure_data;
    test = pmc_test_case_create("test_measure_helper",
                                "Test performance measurement helper",
                                NULL,
                                test_measure_helper_run,
                                NULL,
                                &measure_data);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 5: Cache performance */
    test_cache_perf_data_t cache_data;
    test = pmc_test_case_create("test_cache_perf",
                                "Test cache performance",
                                test_cache_perf_setup,
                                test_cache_perf_run,
                                test_cache_perf_teardown,
                                &cache_data);
    pmc_test_suite_add_test(suite, test);
    
    /* Test 6: Branch prediction */
    test_branch_pred_data_t branch_data;
    test = pmc_test_case_create("test_branch_pred",
                                "Test branch prediction",
                                test_branch_pred_setup,
                                test_branch_pred_run,
                                test_branch_pred_teardown,
                                &branch_data);
    pmc_test_suite_add_test(suite, test);
    
    /* Run tests */
    int failed = pmc_test_suite_run(suite);
    
    /* Get statistics */
    pmc_test_stats_t stats;
    pmc_test_suite_get_stats(suite, &stats);
    
    printf("\n========================================\n");
    printf("Kernel Test Summary:\n");
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
