/**
 * @file pmc_test_framework.c
 * @brief FreeBSD PMC Kernel Test Framework - Implementation
 * @version 1.0.0
 * 
 * Copyright (c) 2026. All rights reserved.
 */

#include "pmc_test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <dlfcn.h>
#include <stdarg.h>

static pmc_test_framework_t g_framework = {0};
static bool g_initialized = false;
static int g_log_level = LOG_LEVEL_INFO;

#define LOG_LEVEL_ERROR   0
#define LOG_LEVEL_WARN    1
#define LOG_LEVEL_INFO    2
#define LOG_LEVEL_DEBUG   3

static void pmc_test_log_internal(int level, const char *format, va_list args)
{
    if (level > g_log_level) return;
    
    const char *prefix = "";
    switch (level) {
        case LOG_LEVEL_ERROR:   prefix = "[ERROR] "; break;
        case LOG_LEVEL_WARN:    prefix = "[WARN] "; break;
        case LOG_LEVEL_INFO:    prefix = "[INFO] "; break;
        case LOG_LEVEL_DEBUG:   prefix = "[DEBUG] "; break;
    }
    
    if (g_framework.tf_output) {
        fprintf(g_framework.tf_output, "%s", prefix);
        vfprintf(g_framework.tf_output, format, args);
        fprintf(g_framework.tf_output, "\n");
        fflush(g_framework.tf_output);
    }
}

int pmc_test_init(bool verbose, FILE *output)
{
    if (g_initialized) return 0;
    
    memset(&g_framework, 0, sizeof(g_framework));
    g_framework.tf_output = output ? output : stdout;
    g_framework.tf_verbose = verbose;
    
    if (verbose) g_log_level = LOG_LEVEL_DEBUG;
    
    g_framework.tf_num_cpus = pmc_test_get_cpu_count();
    g_framework.tf_num_counters = pmc_test_get_counter_count();
    
    for (int i = 0; i < PMC_TEST_MAX_COUNTERS; i++) {
        g_framework.tf_counters[i].pc_count = -1;
    }
    
    pmc_test_log(LOG_LEVEL_INFO, "PMC Test Framework initialized");
    pmc_test_log(LOG_LEVEL_INFO, "CPUs: %d, Counters: %d", 
                 g_framework.tf_num_cpus, g_framework.tf_num_counters);
    
    g_framework.tf_initialized = true;
    g_initialized = true;
    
    return 0;
}

int pmc_test_shutdown(void)
{
    if (!g_initialized) return 0;
    pmc_test_log(LOG_LEVEL_INFO, "Shutting down PMC Test Framework");
    g_framework.tf_initialized = false;
    g_initialized = false;
    return 0;
}

pmc_test_framework_t* pmc_test_get_framework(void) { return &g_framework; }

int pmc_test_get_version(int *major, int *minor)
{
    if (major) *major = 1;
    if (minor) *minor = 0;
    return 0;
}

bool pmc_test_is_available(void)
{
    return pmc_test_get_cpu_count() > 0 && pmc_test_get_counter_count() > 0;
}

int pmc_test_get_cpu_count(void)
{
    int num_cpus;
#ifdef FREEBSD
    size_t len = sizeof(num_cpus);
    sysctlbyname("hw.ncpu", &num_cpus, &len, NULL, 0);
#else
    num_cpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpus <= 0) num_cpus = 1;
#endif
    return num_cpus;
}

int pmc_test_get_counter_count(void)
{
    return PMC_TEST_MAX_COUNTERS;
}

int pmc_test_counter_alloc(int cpu, pmc_test_event_t event,
                           pmc_test_mode_t mode, pmc_test_flags_t flags)
{
    if (!g_initialized) return -1;
    if (cpu < 0 || cpu >= g_framework.tf_num_cpus) return -1;
    if (event >= PMC_EVENT_MAX) return -1;
    
    int counter = -1;
    for (int i = 0; i < PMC_TEST_MAX_COUNTERS; i++) {
        if (g_framework.tf_counters[i].pc_count < 0) {
            counter = i;
            break;
        }
    }
    
    if (counter < 0) {
        pmc_test_log(LOG_LEVEL_ERROR, "No free counter slots");
        return -1;
    }
    
    pmc_test_counter_config_t *config = &g_framework.tf_counters[counter];
    memset(config, 0, sizeof(*config));
    
    config->pc_cpu = cpu;
    config->pc_evsel = pmc_test_map_event(event);
    config->pc_flags = flags;
    config->pc_flavor = PMC_FLAVOR_INTEL_P6;
    config->pc_cap = PMC_CAP_READ | PMC_CAP_WRITE | PMC_CAP_INCREMENT;
    config->pc_count = counter;
    
    pmc_test_log(LOG_LEVEL_DEBUG, "Allocated counter %d for CPU %d, event %s",
                 counter, cpu, pmc_test_event_to_string(event));
    
    return counter;
}

int pmc_test_counter_release(int counter)
{
    if (!g_initialized) return -1;
    if (counter < 0 || counter >= PMC_TEST_MAX_COUNTERS) return -1;
    if (g_framework.tf_counters[counter].pc_count < 0) return -1;
    
    memset(&g_framework.tf_counters[counter], 0, sizeof(pmc_test_counter_config_t));
    g_framework.tf_counters[counter].pc_count = -1;
    
    pmc_test_log(LOG_LEVEL_DEBUG, "Released counter %d", counter);
    return 0;
}

int pmc_test_counter_start(int counter)
{
    if (!g_initialized) return -1;
    if (counter < 0 || counter >= PMC_TEST_MAX_COUNTERS) return -1;
    
    pmc_test_counter_data_t *data = &g_framework.tf_counter_data[counter];
    pmc_test_get_timestamp(&data->cd_timestamp);
    data->cd_prev_value = data->cd_value;
    
    pmc_test_log(LOG_LEVEL_DEBUG, "Started counter %d", counter);
    return 0;
}

int pmc_test_counter_stop(int counter)
{
    if (!g_initialized) return -1;
    if (counter < 0 || counter >= PMC_TEST_MAX_COUNTERS) return -1;
    
    pmc_test_log(LOG_LEVEL_DEBUG, "Stopped counter %d", counter);
    return 0;
}

int pmc_test_counter_read(int counter, uint64_t *value)
{
    if (!g_initialized) return -1;
    if (counter < 0 || counter >= PMC_TEST_MAX_COUNTERS) return -1;
    if (!value) return -1;
    
    pmc_test_counter_data_t *data = &g_framework.tf_counter_data[counter];
    *value = data->cd_value;
    
    return 0;
}

int pmc_test_counter_configure(int counter, pmc_test_counter_config_t *config)
{
    if (!g_initialized) return -1;
    if (counter < 0 || counter >= PMC_TEST_MAX_COUNTERS) return -1;
    if (!config) return -1;
    
    memcpy(&g_framework.tf_counters[counter], config, sizeof(*config));
    return 0;
}

int pmc_test_counter_info(int counter, pmc_test_counter_config_t *config)
{
    if (!g_initialized) return -1;
    if (counter < 0 || counter >= PMC_TEST_MAX_COUNTERS) return -1;
    if (!config) return -1;
    
    memcpy(config, &g_framework.tf_counters[counter], sizeof(*config));
    return 0;
}

pmc_test_suite_t* pmc_test_suite_create(const char *name, const char *description)
{
    pmc_test_suite_t *suite = calloc(1, sizeof(pmc_test_suite_t));
    if (!suite) return NULL;
    
    if (name) strncpy(suite->ts_name, name, PMC_TEST_NAME_MAX - 1);
    if (description) strncpy(suite->ts_description, description, PMC_TEST_DESC_MAX - 1);
    
    return suite;
}

int pmc_test_suite_destroy(pmc_test_suite_t *suite)
{
    if (!suite) return -1;
    
    if (suite->ts_tests) {
        for (int i = 0; i < suite->ts_count; i++) {
            if (suite->ts_tests[i]) pmc_test_case_destroy(suite->ts_tests[i]);
        }
        free(suite->ts_tests);
    }
    
    free(suite);
    return 0;
}

int pmc_test_suite_add_test(pmc_test_suite_t *suite, pmc_test_case_t *test)
{
    if (!suite || !test) return -1;
    
    pmc_test_case_t **new_tests = realloc(suite->ts_tests,
                                          (suite->ts_count + 1) * sizeof(pmc_test_case_t*));
    if (!new_tests) return -1;
    
    suite->ts_tests = new_tests;
    suite->ts_tests[suite->ts_count] = test;
    suite->ts_count++;
    
    return 0;
}

int pmc_test_suite_run(pmc_test_suite_t *suite)
{
    if (!suite) return -1;
    if (!g_initialized) pmc_test_init(false, NULL);
    
    pmc_test_log(LOG_LEVEL_INFO, "Running test suite: %s", suite->ts_name);
    pmc_test_log(LOG_LEVEL_INFO, "Tests: %d", suite->ts_count);
    
    g_framework.tf_current_suite = suite;
    pmc_test_get_timestamp(&g_framework.tf_stats.ts_start);
    
    for (int i = 0; i < suite->ts_count; i++) {
        pmc_test_case_t *test = suite->ts_tests[i];
        if (!test) continue;
        
        pmc_test_log(LOG_LEVEL_INFO, "[%d/%d] Running: %s", i + 1, suite->ts_count, test->tc_name);
        
        pmc_test_result_t result = pmc_test_case_run(test);
        
        switch (result) {
            case PMC_TEST_RESULT_PASS:
                suite->ts_passed++;
                pmc_test_log(LOG_LEVEL_INFO, "  Result: PASS");
                break;
            case PMC_TEST_RESULT_FAIL:
                suite->ts_failed++;
                pmc_test_log(LOG_LEVEL_INFO, "  Result: FAIL");
                break;
            case PMC_TEST_RESULT_SKIP:
                suite->ts_skipped++;
                pmc_test_log(LOG_LEVEL_INFO, "  Result: SKIP");
                break;
            default:
                suite->ts_errors++;
                pmc_test_log(LOG_LEVEL_ERROR, "  Result: ERROR");
                break;
        }
    }
    
    pmc_test_get_timestamp(&g_framework.tf_stats.ts_end);
    g_framework.tf_stats.ts_duration = pmc_test_time_diff(
        &g_framework.tf_stats.ts_start, &g_framework.tf_stats.ts_end);
    
    pmc_test_log(LOG_LEVEL_INFO, "Results: %d passed, %d failed, %d skipped, %d errors",
                 suite->ts_passed, suite->ts_failed, suite->ts_skipped, suite->ts_errors);
    
    g_framework.tf_current_suite = NULL;
    
    return suite->ts_failed;
}

int pmc_test_suite_get_stats(pmc_test_suite_t *suite, pmc_test_stats_t *stats)
{
    if (!suite || !stats) return -1;
    
    stats->ts_total = suite->ts_count;
    stats->ts_passed = suite->ts_passed;
    stats->ts_failed = suite->ts_failed;
    stats->ts_skipped = suite->ts_skipped;
    stats->ts_errors = suite->ts_errors;
    stats->ts_duration = g_framework.tf_stats.ts_duration;
    
    return 0;
}

pmc_test_case_t* pmc_test_case_create(const char *name, const char *description,
                                       int (*setup)(void *), int (*run)(void *),
                                       int (*teardown)(void *), void *data)
{
    pmc_test_case_t *test = calloc(1, sizeof(pmc_test_case_t));
    if (!test) return NULL;
    
    if (name) strncpy(test->tc_name, name, PMC_TEST_NAME_MAX - 1);
    if (description) strncpy(test->tc_description, description, PMC_TEST_DESC_MAX - 1);
    
    test->tc_setup = setup;
    test->tc_run = run;
    test->tc_teardown = teardown;
    test->tc_data = data;
    test->tc_result = PMC_TEST_RESULT_SKIP;
    
    return test;
}

int pmc_test_case_destroy(pmc_test_case_t *test)
{
    if (!test) return -1;
    free(test);
    return 0;
}

pmc_test_result_t pmc_test_case_run(pmc_test_case_t *test)
{
    if (!test) return PMC_TEST_RESULT_ERROR;
    if (!test->tc_run) {
        test->tc_result = PMC_TEST_RESULT_ERROR;
        return test->tc_result;
    }
    
    pmc_test_get_timestamp(&test->tc_start_time);
    
    if (test->tc_setup) {
        int result = test->tc_setup(test->tc_data);
        if (result != 0) {
            test->tc_result = PMC_TEST_RESULT_ERROR;
            pmc_test_get_timestamp(&test->tc_end_time);
            return test->tc_result;
        }
    }
    
    int run_result = test->tc_run(test->tc_data);
    
    if (test->tc_teardown) test->tc_teardown(test->tc_data);
    
    pmc_test_get_timestamp(&test->tc_end_time);
    
    if (run_result == 0) test->tc_result = PMC_TEST_RESULT_PASS;
    else if (run_result < 0) test->tc_result = PMC_TEST_RESULT_ERROR;
    else test->tc_result = PMC_TEST_RESULT_FAIL;
    
    return test->tc_result;
}

void pmc_test_get_timestamp(struct timespec *ts)
{
    if (ts) clock_gettime(CLOCK_MONOTONIC, ts);
}

double pmc_test_time_diff(struct timespec *start, struct timespec *end)
{
    if (!start || !end) return 0.0;
    double diff = (end->tv_sec - start->tv_sec) * 1.0;
    diff += (end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return diff;
}

const char* pmc_test_event_to_string(pmc_test_event_t event)
{
    static const char *event_names[] = {
        "CPU_CYCLES", "INSTRUCTIONS", "CACHE_REFERENCES", "CACHE_MISSES",
        "BRANCH_INSTRUCTIONS", "BRANCH_MISSES", "BUS_CYCLES", "STALLED_CYCLES",
        "MEMORY_REQUESTS", "MEMORY_READS", "MEMORY_WRITES", "CONTEXT_SWITCHES",
        "CPU_MIGRATIONS", "PAGE_FAULTS", "CPU_CLOCK", "REF_CPU_CLOCK",
        /* L3 Cache Events (Core) */
        "L3_REFERENCES", "L3_MISSES", "L3_TOTAL_MISSES", "L3_MISS_DRAM",
        "L3_MISS_L2", "L3_HIT", "L3_HITM", "L3_HIT_FE",
        /* Data Fabric Events (Uncore - AMD Zen4/Zen5+) */
        "DF_COUNTERS", "DF_READS", "DF_WRITES", "DF_SELF_MODIFIED",
        "DF_SNOOP", "DF_INVALIDS", "DF_PSTATS", "DF_PROBE",
        "DF_CXL", "DF_MEMIF", "DF_GMI", "DF_XGMI"
    };
    if (event >= PMC_EVENT_MAX) return "UNKNOWN";
    return event_names[event];
}

const char* pmc_test_result_to_string(pmc_test_result_t result)
{
    static const char *result_names[] = {"PASS", "FAIL", "SKIP", "ERROR", "TIMEOUT"};
    if (result >= PMC_RESULT_MAX) return "UNKNOWN";
    return result_names[result];
}

void pmc_test_log(int level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    pmc_test_log_internal(level, format, args);
    va_end(args);
}

void pmc_test_sleep(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

uint32_t pmc_test_map_event(pmc_test_event_t event)
{
    /* Event mapping for Intel/AMD PMC counters */
    /* Core events */
    static const uint32_t event_map[] = {
        0x3C, 0xC0, 0x4F, 0x4E, 0xC4, 0xC5, 0x3C, 0xA2,  /* 0-7 */
        0x0, 0x0, 0x0, 0x5C, 0x5D, 0x6F, 0x0, 0x0,         /* 8-15 */
        /* L3 Cache Events (Core) - AMD Zen4/5+ */
        0x04, 0x26, 0x27, 0x28, 0x29, 0x04, 0x04, 0x04,  /* 16-23 */
        /* Data Fabric Events (Uncore - AMD Zen4/Zen5+) */
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,           /* 24-31 */
        0x0, 0x0, 0x0, 0x0                                /* 32-35 */
    };
    if (event >= PMC_EVENT_MAX) return 0;
    return event_map[event];
}

pmc_caps pmc_test_get_cpu_caps(int cpu)
{
    (void)cpu;
    return (pmc_caps)(PMC_CAP_READ | PMC_CAP_WRITE | PMC_CAP_INCREMENT | 
                      PMC_CAP_INTERRUPT | PMC_CAP_SYSTEM);
}

int pmc_test_measure_start(int *counters, int num_counters)
{
    if (!counters || num_counters <= 0) return -1;
    for (int i = 0; i < num_counters; i++) {
        if (pmc_test_counter_start(counters[i]) != 0) {
            for (int j = 0; j < i; j++) pmc_test_counter_stop(counters[j]);
            return -1;
        }
    }
    return 0;
}

int pmc_test_measure_stop(int *counters, int num_counters, uint64_t *values)
{
    if (!counters || num_counters <= 0 || !values) return -1;
    for (int i = 0; i < num_counters; i++) {
        if (pmc_test_counter_read(counters[i], &values[i]) != 0) return -1;
        pmc_test_counter_stop(counters[i]);
    }
    return 0;
}

void pmc_test_calculate_delta(uint64_t *start, uint64_t *end, uint64_t *delta, int count)
{
    if (!start || !end || !delta) return;
    for (int i = 0; i < count; i++) delta[i] = end[i] - start[i];
}

int pmc_test_get_system_info(struct pmc_info *info)
{
    if (!info) return -1;
    memset(info, 0, sizeof(*info));
    info->pi_npmc = pmc_test_get_counter_count();
    info->pi_ncpu = pmc_test_get_cpu_count();
#ifdef FREEBSD
    size_t len = sizeof(info->pi_cpu);
    sysctlbyname("hw.model", &info->pi_cpu, &len, NULL, 0);
#else
    strncpy(info->pi_cpu, "Linux-simulated", sizeof(info->pi_cpu) - 1);
#endif
    return 0;
}

bool pmc_test_is_root(void) { return (geteuid() == 0); }

bool pmc_test_kernel_supported(void)
{
#ifdef FREEBSD
    int pmc_present;
    size_t len = sizeof(pmc_present);
    if (sysctlbyname("kern.pm_present", &pmc_present, &len, NULL, 0) == 0)
        return (pmc_present != 0);
#endif
    return false;
}
