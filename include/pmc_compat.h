/**
 * @file pmc_compat.h
 * @brief Linux compatibility layer for PMC test framework
 * 
 * This header provides compatibility definitions for building on Linux
 * while targeting FreeBSD. For actual PMC functionality, use on FreeBSD.
 */

#ifndef _PMC_COMPAT_H
#define _PMC_COMPAT_H

#ifdef LINUX

#include <stdint.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

/* Define BSD types for Linux compatibility */
typedef uint8_t u_char;
typedef uint16_t u_short;
typedef uint32_t u_int;
typedef uint64_t u_long;

/* Define FreeBSD-specific enums */
enum pmc_caps {
    PMC_CAP_READ = 0x01,
    PMC_CAP_WRITE = 0x02,
    PMC_CAP_INCREMENT = 0x04,
    PMC_CAP_INTERRUPT = 0x08,
    PMC_CAP_SYSTEM = 0x10,
    PMC_CAP_USER = 0x20
};

enum pmc_flavor {
    PMC_FLAVOR_INVALID = 0,
    PMC_FLAVOR_INTEL_P6,
    PMC_FLAVOR_INTEL_PII,
    PMC_FLAVOR_INTEL_PIII,
    PMC_FLAVOR_INTEL_P4,
    PMC_FLAVOR_INTEL_NEHALEM,
    PMC_FLAVOR_AMD_K7,
    PMC_FLAVOR_AMD_K8,
    PMC_FLAVOR_AMD_K10
};

enum pmc_mode {
    PMC_MODE_COUNT = 0,
    PMC_MODE_SAMPLE,
    PMC_MODE_CCS,
    PMC_MODE_MAX
};

/* Define PMC flags */
#define PMC_FM_USER      0x01
#define PMC_FM_KERNEL    0x02

/* Define pmc_info structure */
struct pmc_info {
    int         pi_ncpu;
    int         pi_npmc;
    char        pi_cpu[256];
};

/* Stub functions for Linux */
static inline int pmc_init(void) { return 0; }
static inline int pmc_shutdown(void) { return 0; }

#else /* !LINUX - FreeBSD or other */

/* On FreeBSD, include the actual headers */
#include <sys/pmc.h>

#endif /* LINUX */

#endif /* _PMC_COMPAT_H */
