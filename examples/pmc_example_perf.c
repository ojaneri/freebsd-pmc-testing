/**
 * @file pmc_example_perf.c
 * @brief Performance Measurement Example
 * @version 1.0.0
 * 
 * Demonstrates advanced performance measurement using the PMC test framework.
 * Shows how to measure CPU cycles, cache performance, and branch prediction.
 * 
 * Copyright (c) 2026. All rights reserved.
 */

#include "pmc_test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* Structure to hold performance measurement results */
typedef struct {
    uint64_t cpu_cycles;
    uint64_t instructions;
    uint64_t cache_refs;
    uint64_t cache_misses;
    uint64_t branch_instructions;
    uint64_t branch_misses;
    double duration_seconds;
} perf_results_t;

/* Performance measurement context */
typedef struct {
    int cpu_cycles_counter;
    int instructions_counter;
    int cache_ref_counter;
    int cache_miss_counter;
    int branch_counter;
    int branch_miss_counter;
    struct timeval start_time;
    struct timeval end_time;
} perf_context_t;

/**
 * Initialize performance measurement context
 */
static int perf_init(perf_context_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    
    /* Allocate counters for various events */
    ctx->cpu_cycles_counter = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES,
                                                      PMC_MODE_COUNT, PMC_FLAG_ALL);
    ctx->instructions_counter = pmc_test_counter_alloc(0, PMC_EVENT_INSTRUCTIONS,
                                                        PMC_MODE_COUNT, PMC_FLAG_ALL);
    ctx->cache_ref_counter = pmc_test_counter_alloc(0, PMC_EVENT_CACHE_REFERENCES,
                                                     PMC_MODE_COUNT, PMC_FLAG_ALL);
    ctx->cache_miss_counter = pmc_test_counter_alloc(0, PMC_EVENT_CACHE_MISSES,
                                                      PMC_MODE_COUNT, PMC_FLAG_ALL);
    ctx->branch_counter = pmc_test_counter_alloc(0, PMC_EVENT_BRANCH_INSTRUCTIONS,
                                                   PMC_MODE_COUNT, PMC_FLAG_ALL);
    ctx->branch_miss_counter = pmc_test_counter_alloc(0, PMC_EVENT_BRANCH_MISSES,
                                                      PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    /* Check if all allocations succeeded */
    if (ctx->cpu_cycles_counter < 0 ||
        ctx->instructions_counter < 0 ||
        ctx->cache_ref_counter < 0 ||
        ctx->cache_miss_counter < 0 ||
        ctx->branch_counter < 0 ||
        ctx->branch_miss_counter < 0) {
        return -1;
    }
    
    return 0;
}

/**
 * Start performance measurement
 */
static void perf_start(perf_context_t *ctx)
{
    int counters[] = {
        ctx->cpu_cycles_counter,
        ctx->instructions_counter,
        ctx->cache_ref_counter,
        ctx->cache_miss_counter,
        ctx->branch_counter,
        ctx->branch_miss_counter
    };
    
    /* Start all counters */
    for (int i = 0; i < 6; i++) {
        pmc_test_counter_start(counters[i]);
    }
    
    gettimeofday(&ctx->start_time, NULL);
}

/**
 * Stop performance measurement
 */
static void perf_stop(perf_context_t *ctx, perf_results_t *results)
{
    int counters[] = {
        ctx->cpu_cycles_counter,
        ctx->instructions_counter,
        ctx->cache_ref_counter,
        ctx->cache_miss_counter,
        ctx->branch_counter,
        ctx->branch_miss_counter
    };
    
    uint64_t values[6];
    
    /* Stop and read all counters */
    for (int i = 0; i < 6; i++) {
        pmc_test_counter_stop(counters[i]);
        pmc_test_counter_read(counters[i], &values[i]);
    }
    
    gettimeofday(&ctx->end_time, NULL);
    
    /* Store results */
    results->cpu_cycles = values[0];
    results->instructions = values[1];
    results->cache_refs = values[2];
    results->cache_misses = values[3];
    results->branch_instructions = values[4];
    results->branch_misses = values[5];
    
    /* Calculate duration */
    results->duration_seconds = 
        (ctx->end_time.tv_sec - ctx->start_time.tv_sec) +
        (ctx->end_time.tv_usec - ctx->start_time.tv_usec) / 1000000.0;
}

/**
 * Cleanup performance context
 */
static void perf_cleanup(perf_context_t *ctx)
{
    pmc_test_counter_release(ctx->cpu_cycles_counter);
    pmc_test_counter_release(ctx->instructions_counter);
    pmc_test_counter_release(ctx->cache_ref_counter);
    pmc_test_counter_release(ctx->cache_miss_counter);
    pmc_test_counter_release(ctx->branch_counter);
    pmc_test_counter_release(ctx->branch_miss_counter);
}

/**
 * Print performance results
 */
static void perf_print_results(const char *name, perf_results_t *results)
{
    printf("\n=== %s Results ===\n", name);
    printf("Duration:       %.6f seconds\n", results->duration_seconds);
    printf("CPU Cycles:     %lu\n", (unsigned long)results->cpu_cycles);
    printf("Instructions:   %lu\n", (unsigned long)results->instructions);
    printf("Cache Refs:     %lu\n", (unsigned long)results->cache_refs);
    printf("Cache Misses:   %lu\n", (unsigned long)results->cache_misses);
    printf("Branches:       %lu\n", (unsigned long)results->branch_instructions);
    printf("Branch Misses:  %lu\n", (unsigned long)results->branch_misses);
    
    /* Calculate ratios */
    if (results->cpu_cycles > 0) {
        double ipc = (double)results->instructions / results->cpu_cycles;
        printf("IPC:            %.3f\n", ipc);
    }
    
    if (results->cache_refs > 0) {
        double miss_rate = (double)results->cache_misses / results->cache_refs * 100.0;
        printf("Cache Miss Rate: %.2f%%\n", miss_rate);
    }
    
    if (results->branch_instructions > 0) {
        double miss_rate = (double)results->branch_misses / results->branch_instructions * 100.0;
        printf("Branch Miss Rate: %.2f%%\n", miss_rate);
    }
    
    if (results->duration_seconds > 0) {
        double mips = results->instructions / results->duration_seconds / 1000000.0;
        printf("MIPS:           %.2f\n", mips);
    }
}

/* Benchmark functions */

/**
 * CPU-intensive benchmark
 */
static void benchmark_cpu_intensive(int iterations)
{
    volatile long long sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i * i + i * 3 - i / 2;
    }
    (void)sum;
}

/**
 * Memory-intensive benchmark
 */
static void benchmark_memory_intensive(int size)
{
    int *array = malloc(size * sizeof(int));
    if (!array) return;
    
    /* Write to all locations */
    for (int i = 0; i < size; i++) {
        array[i] = i;
    }
    
    /* Read from all locations */
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    (void)sum;
    
    free(array);
}

/**
 * Branch-intensive benchmark
 */
static void benchmark_branch_intensive(int iterations)
{
    volatile int sum = 0;
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    (void)sum;
}

/**
 * Mixed workload benchmark
 */
static void benchmark_mixed(int iterations)
{
    volatile long long sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Mix of operations */
        sum += i * 2;
        if (i % 3 == 0) {
            sum /= 2;
        }
        sum *= 1;
    }
    (void)sum;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    printf("========================================\n");
    printf("PMC Performance Measurement Example\n");
    printf("========================================\n\n");
    
    /* Initialize framework */
    if (pmc_test_init(true, stdout) != 0) {
        fprintf(stderr, "Failed to initialize PMC framework\n");
        return 1;
    }
    
    /* Check requirements */
    if (!pmc_test_is_available()) {
        fprintf(stderr, "PMC not available on this system\n");
        pmc_test_shutdown();
        return 1;
    }
    
    if (!pmc_test_is_root()) {
        printf("Warning: Not running as root. Results may be limited.\n\n");
    }
    
    /* Initialize performance context */
    perf_context_t ctx;
    if (perf_init(&ctx) != 0) {
        fprintf(stderr, "Failed to initialize performance context\n");
        pmc_test_shutdown();
        return 1;
    }
    
    perf_results_t results;
    
    /* Benchmark 1: CPU Intensive */
    printf("\n--- Running CPU Intensive Benchmark ---\n");
    perf_start(&ctx);
    benchmark_cpu_intensive(5000000);
    perf_stop(&ctx, &results);
    perf_print_results("CPU Intensive", &results);
    
    /* Benchmark 2: Memory Intensive */
    printf("\n--- Running Memory Intensive Benchmark ---\n");
    perf_start(&ctx);
    benchmark_memory_intensive(256 * 1024);  /* 1MB */
    perf_stop(&ctx, &results);
    perf_print_results("Memory Intensive", &results);
    
    /* Benchmark 3: Branch Intensive */
    printf("\n--- Running Branch Intensive Benchmark ---\n");
    perf_start(&ctx);
    benchmark_branch_intensive(5000000);
    perf_stop(&ctx, &results);
    perf_print_results("Branch Intensive", &results);
    
    /* Benchmark 4: Mixed Workload */
    printf("\n--- Running Mixed Workload Benchmark ---\n");
    perf_start(&ctx);
    benchmark_mixed(5000000);
    perf_stop(&ctx, &results);
    perf_print_results("Mixed Workload", &results);
    
    /* Cleanup */
    perf_cleanup(&ctx);
    
    printf("\n========================================\n");
    printf("Performance Measurement Complete\n");
    printf("========================================\n");
    
    /* Shutdown framework */
    pmc_test_shutdown();
    
    return 0;
}
