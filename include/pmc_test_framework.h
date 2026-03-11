/**
 * @file pmc_test_framework.h
 * @brief FreeBSD PMC Kernel Test Framework - Main Header
 * @version 1.0.0
 * 
 * This header file defines the core API for the PMC (Performance Monitoring
 * Counters) kernel test framework on FreeBSD systems.
 * 
 * Copyright (c) 2026. All rights reserved.
 */

#ifndef _PMC_TEST_FRAMEWORK_H
#define _PMC_TEST_FRAMEWORK_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>

#ifdef FREEBSD
#include <sys/pmc.h>
#else
/* Define FreeBSD-compatible types for Linux build */
typedef enum pmc_caps_enum {
    PMC_CAP_READ = 0x01,
    PMC_CAP_WRITE = 0x02,
    PMC_CAP_INCREMENT = 0x04,
    PMC_CAP_INTERRUPT = 0x08,
    PMC_CAP_SYSTEM = 0x10,
    PMC_CAP_USER = 0x20
} pmc_caps;

typedef enum pmc_flavor_enum {
    PMC_FLAVOR_INVALID = 0,
    PMC_FLAVOR_INTEL_P6,
    PMC_FLAVOR_INTEL_PII,
    PMC_FLAVOR_INTEL_PIII,
    PMC_FLAVOR_INTEL_P4,
    PMC_FLAVOR_INTEL_NEHALEM,
    PMC_FLAVOR_AMD_K7,
    PMC_FLAVOR_AMD_K8,
    PMC_FLAVOR_AMD_K10
} pmc_flavor;

#define PMC_FM_USER    0x01
#define PMC_FM_KERNEL  0x02

#define LOG_LEVEL_ERROR   0
#define LOG_LEVEL_WARN    1
#define LOG_LEVEL_INFO    2
#define LOG_LEVEL_DEBUG   3

struct pmc_info {
    int         pi_ncpu;
    int         pi_npmc;
    char        pi_cpu[256];
};
#endif

#define PMC_TEST_MAX_COUNTERS     8
#define PMC_TEST_MAX_CPUS         64
#define PMC_TEST_DEFAULT_TIMEOUT  30
#define PMC_TEST_OUTPUT_BUFFER    4096
#define PMC_TEST_NAME_MAX         128
#define PMC_TEST_DESC_MAX         512

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
    /* L3 Cache Events (Core) */
    PMC_EVENT_L3_REFERENCES,
    PMC_EVENT_L3_MISSES,
    PMC_EVENT_L3_TOTAL_MISSES,
    PMC_EVENT_L3_MISS_DRAM,
    PMC_EVENT_L3_MISS_L2,
    PMC_EVENT_L3_HIT,
    PMC_EVENT_L3_HITM,
    PMC_EVENT_L3_HIT_FE,
    /* Data Fabric Events (Uncore - AMD Zen4/Zen5+) */
    PMC_EVENT_DF_COUNTERS,
    PMC_EVENT_DF_READS,
    PMC_EVENT_DF_WRITES,
    PMC_EVENT_DF_SELF_MODIFIED,
    PMC_EVENT_DF_SNOOP,
    PMC_EVENT_DF_INVALIDS,
    PMC_EVENT_DF_PSTATS,
    PMC_EVENT_DF_PROBE,
    PMC_EVENT_DF_CXL,
    PMC_EVENT_DF_MEMIF,
    PMC_EVENT_DF_GMI,
    PMC_EVENT_DF_XGMI,
    PMC_EVENT_MAX
} pmc_test_event_t;

typedef enum {
    PMC_MODE_COUNT = 0,
    PMC_MODE_SAMPLE,
    PMC_MODE_CCS,
    PMC_MODE_MAX
} pmc_test_mode_t;

typedef enum {
    PMC_FLAG_NONE = 0,
    PMC_FLAG_USER = PMC_FM_USER,
    PMC_FLAG_KERNEL = PMC_FM_KERNEL,
    PMC_FLAG_EVNTSELECT = 0xFF,
    PMC_FLAG_ALL = PMC_FM_USER | PMC_FM_KERNEL
} pmc_test_flags_t;

typedef enum {
    PMC_TEST_RESULT_PASS = 0,
    PMC_TEST_RESULT_FAIL,
    PMC_TEST_RESULT_SKIP,
    PMC_TEST_RESULT_ERROR,
    PMC_TEST_RESULT_TIMEOUT,
    PMC_RESULT_MAX
} pmc_test_result_t;

typedef struct pmc_test_counter_config {
    int                     pc_cpu;
    pmc_caps                pc_cap;
    pmc_flavor              pc_flavor;
    uint32_t                pc_evsel;
    uint32_t                pc_config;
    pmc_test_flags_t        pc_flags;
    int                     pc_count;
} pmc_test_counter_config_t;

typedef struct pmc_test_counter_data {
    int                     cd_cpu;
    uint64_t                cd_value;
    uint64_t                cd_prev_value;
    struct timespec         cd_timestamp;
    pmc_test_event_t        cd_event;
} pmc_test_counter_data_t;

typedef struct pmc_test_case {
    char                    tc_name[PMC_TEST_NAME_MAX];
    char                    tc_description[PMC_TEST_DESC_MAX];
    pmc_test_result_t       tc_result;
    struct timespec         tc_start_time;
    struct timespec         tc_end_time;
    char                    tc_message[PMC_TEST_OUTPUT_BUFFER];
    void                    *tc_data;
    int                     (*tc_setup)(void *);
    int                     (*tc_run)(void *);
    int                     (*tc_teardown)(void *);
} pmc_test_case_t;

typedef struct pmc_test_suite {
    char                    ts_name[PMC_TEST_NAME_MAX];
    char                    ts_description[PMC_TEST_DESC_MAX];
    int                     ts_count;
    int                     ts_passed;
    int                     ts_failed;
    int                     ts_skipped;
    int                     ts_errors;
    pmc_test_case_t         **ts_tests;
} pmc_test_suite_t;

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

typedef struct pmc_test_framework {
    int                     tf_num_counters;
    int                     tf_num_cpus;
    pmc_test_counter_config_t tf_counters[PMC_TEST_MAX_COUNTERS];
    pmc_test_counter_data_t tf_counter_data[PMC_TEST_MAX_COUNTERS];
    pmc_test_suite_t        *tf_current_suite;
    pmc_test_stats_t        tf_stats;
    bool                    tf_initialized;
    bool                    tf_verbose;
    FILE                    *tf_output;
} pmc_test_framework_t;

int pmc_test_init(bool verbose, FILE *output);
int pmc_test_shutdown(void);
pmc_test_framework_t* pmc_test_get_framework(void);
int pmc_test_get_version(int *major, int *minor);
bool pmc_test_is_available(void);
int pmc_test_get_cpu_count(void);
int pmc_test_get_counter_count(void);

int pmc_test_counter_alloc(int cpu, pmc_test_event_t event, 
                           pmc_test_mode_t mode, pmc_test_flags_t flags);
int pmc_test_counter_release(int counter);
int pmc_test_counter_start(int counter);
int pmc_test_counter_stop(int counter);
int pmc_test_counter_read(int counter, uint64_t *value);
int pmc_test_counter_configure(int counter, pmc_test_counter_config_t *config);
int pmc_test_counter_info(int counter, pmc_test_counter_config_t *config);

pmc_test_suite_t* pmc_test_suite_create(const char *name, const char *description);
int pmc_test_suite_destroy(pmc_test_suite_t *suite);
int pmc_test_suite_add_test(pmc_test_suite_t *suite, pmc_test_case_t *test);
int pmc_test_suite_run(pmc_test_suite_t *suite);
int pmc_test_suite_get_stats(pmc_test_suite_t *suite, pmc_test_stats_t *stats);

pmc_test_case_t* pmc_test_case_create(const char *name, const char *description,
                                       int (*setup)(void *),
                                       int (*run)(void *),
                                       int (*teardown)(void *),
                                       void *data);
int pmc_test_case_destroy(pmc_test_case_t *test);
pmc_test_result_t pmc_test_case_run(pmc_test_case_t *test);

#define pmc_test_assert(condition, message) \
    do { \
        if (!(condition)) { \
            return PMC_TEST_RESULT_FAIL; \
        } \
    } while (0)

#define pmc_test_assert_msg(condition, fmt, ...) \
    do { \
        if (!(condition)) { \
            snprintf(test->tc_message, PMC_TEST_OUTPUT_BUFFER, fmt, ##__VA_ARGS__); \
            return PMC_TEST_RESULT_FAIL; \
        } \
    } while (0)

void pmc_test_get_timestamp(struct timespec *ts);
double pmc_test_time_diff(struct timespec *start, struct timespec *end);
const char* pmc_test_event_to_string(pmc_test_event_t event);
const char* pmc_test_result_to_string(pmc_test_result_t result);
void pmc_test_log(int level, const char *format, ...);
void pmc_test_sleep(int ms);

uint32_t pmc_test_map_event(pmc_test_event_t event);
pmc_caps pmc_test_get_cpu_caps(int cpu);

int pmc_test_measure_start(int *counters, int num_counters);
int pmc_test_measure_stop(int *counters, int num_counters, uint64_t *values);
void pmc_test_calculate_delta(uint64_t *start, uint64_t *end, 
                               uint64_t *delta, int count);

int pmc_test_get_system_info(struct pmc_info *info);
bool pmc_test_is_root(void);
bool pmc_test_kernel_supported(void);

#endif
