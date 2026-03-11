/**
 * @file pmc_example_simple.c
 * @brief Simple PMC Usage Example
 * @version 1.0.0
 * 
 * Demonstrates basic usage of the PMC test framework.
 * 
 * Copyright (c) 2026. All rights reserved.
 */

#include "pmc_test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Simple work function to generate CPU activity
 */
static void do_work(int iterations)
{
    volatile long long sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i * i;
    }
    (void)sum;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    printf("========================================\n");
    printf("PMC Simple Example\n");
    printf("========================================\n\n");
    
    /* Initialize framework with verbose output */
    if (pmc_test_init(true, stdout) != 0) {
        fprintf(stderr, "Failed to initialize PMC framework\n");
        return 1;
    }
    
    /* Print system information */
    printf("\n--- System Information ---\n");
    printf("CPUs: %d\n", pmc_test_get_cpu_count());
    printf("PMC Counters: %d\n", pmc_test_get_counter_count());
    printf("PMC Available: %s\n", pmc_test_is_available() ? "Yes" : "No");
    printf("Running as root: %s\n", pmc_test_is_root() ? "Yes" : "No");
    printf("Kernel PMC support: %s\n", pmc_test_kernel_supported() ? "Yes" : "No");
    
    /* Example 1: Basic counter usage */
    printf("\n--- Example 1: Basic Counter ---\n");
    
    /* Allocate a counter for CPU cycles */
    int cpu_cycles_counter = pmc_test_counter_alloc(
        0,                      /* CPU 0 */
        PMC_EVENT_CPU_CYCLES,   /* Event: CPU cycles */
        PMC_MODE_COUNT,         /* Count mode */
        PMC_FLAG_ALL            /* Count both user and kernel */
    );
    
    if (cpu_cycles_counter < 0) {
        fprintf(stderr, "Failed to allocate counter\n");
        pmc_test_shutdown();
        return 1;
    }
    
    printf("Allocated counter: %d\n", cpu_cycles_counter);
    
    /* Start counting */
    pmc_test_counter_start(cpu_cycles_counter);
    printf("Counter started\n");
    
    /* Do some work */
    printf("Performing work...\n");
    do_work(1000000);
    
    /* Stop counting */
    pmc_test_counter_stop(cpu_cycles_counter);
    printf("Counter stopped\n");
    
    /* Read the value */
    uint64_t cpu_cycles;
    pmc_test_counter_read(cpu_cycles_counter, &cpu_cycles);
    printf("CPU Cycles: %lu\n", (unsigned long)cpu_cycles);
    
    /* Release counter */
    pmc_test_counter_release(cpu_cycles_counter);
    printf("Counter released\n");
    
    /* Example 2: Multiple counters */
    printf("\n--- Example 2: Multiple Counters ---\n");
    
    /* Allocate multiple counters */
    int counters[3];
    uint64_t values[3];
    
    counters[0] = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES, PMC_MODE_COUNT, PMC_FLAG_ALL);
    counters[1] = pmc_test_counter_alloc(0, PMC_EVENT_INSTRUCTIONS, PMC_MODE_COUNT, PMC_FLAG_ALL);
    counters[2] = pmc_test_counter_alloc(0, PMC_EVENT_CACHE_REFERENCES, PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    /* Check allocation success */
    for (int i = 0; i < 3; i++) {
        if (counters[i] < 0) {
            fprintf(stderr, "Failed to allocate counter %d\n", i);
            pmc_test_shutdown();
            return 1;
        }
    }
    
    /* Start all counters */
    printf("Starting 3 counters...\n");
    pmc_test_counter_start(counters[0]);
    pmc_test_counter_start(counters[1]);
    pmc_test_counter_start(counters[2]);
    
    /* Do work */
    printf("Performing work...\n");
    do_work(2000000);
    
    /* Stop all counters */
    pmc_test_counter_stop(counters[0]);
    pmc_test_counter_stop(counters[1]);
    pmc_test_counter_stop(counters[2]);
    
    /* Read all values */
    for (int i = 0; i < 3; i++) {
        pmc_test_counter_read(counters[i], &values[i]);
    }
    
    printf("CPU Cycles: %lu\n", (unsigned long)values[0]);
    printf("Instructions: %lu\n", (unsigned long)values[1]);
    printf("Cache References: %lu\n", (unsigned long)values[2]);
    
    /* Calculate IPC */
    if (values[0] > 0) {
        double ipc = (double)values[1] / (double)values[0];
        printf("IPC: %.3f\n", ipc);
    }
    
    /* Release all counters */
    for (int i = 0; i < 3; i++) {
        pmc_test_counter_release(counters[i]);
    }
    
    /* Example 3: Using measurement helpers */
    printf("\n--- Example 3: Measurement Helpers ---\n");
    
    /* Allocate counters using helper function */
    int measure_counters[2];
    measure_counters[0] = pmc_test_counter_alloc(0, PMC_EVENT_CPU_CYCLES, PMC_MODE_COUNT, PMC_FLAG_ALL);
    measure_counters[1] = pmc_test_counter_alloc(0, PMC_EVENT_INSTRUCTIONS, PMC_MODE_COUNT, PMC_FLAG_ALL);
    
    uint64_t start_values[2], stop_values[2], deltas[2];
    
    /* Start measurement */
    printf("Starting measurement...\n");
    pmc_test_measure_start(measure_counters, 2);
    
    /* Do work */
    printf("Performing work...\n");
    do_work(3000000);
    
    /* Stop measurement */
    pmc_test_measure_stop(measure_counters, 2, stop_values);
    
    /* Calculate deltas */
    pmc_test_calculate_delta(start_values, stop_values, deltas, 2);
    
    printf("Delta - CPU Cycles: %lu\n", (unsigned long)deltas[0]);
    printf("Delta - Instructions: %lu\n", (unsigned long)deltas[1]);
    
    /* Cleanup */
    pmc_test_counter_release(measure_counters[0]);
    pmc_test_counter_release(measure_counters[1]);
    
    printf("\n--- Examples Complete ---\n");
    
    /* Shutdown framework */
    pmc_test_shutdown();
    
    return 0;
}
