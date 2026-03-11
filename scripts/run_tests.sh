#!/bin/sh
# @file run_tests.sh
# @brief Run PMC test framework tests
# @version 1.0.0

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILDDIR="$PROJECT_DIR/build"

echo "========================================"
echo "PMC Test Framework - Test Runner"
echo "========================================"

# Check if tests are built
if [ ! -f "$BUILDDIR/pmc_tests_basic" ]; then
    echo "${RED}Error: Tests not built. Run build.sh first.${NC}"
    exit 1
fi

# Check for root
if [ "$(id -u)" -ne 0 ]; then
    echo "${YELLOW}Warning: Not running as root. Tests may fail.${NC}"
    echo ""
fi

# Run basic tests
echo ""
echo "Running Basic Tests..."
echo "----------------------------------------"
"$BUILDDIR/pmc_tests_basic"
BASIC_RESULT=$?

# Run kernel tests
echo ""
echo "Running Kernel Tests..."
echo "----------------------------------------"
"$BUILDDIR/pmc_tests_kernel"
KERNEL_RESULT=$?

# Summary
echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"

if [ $BASIC_RESULT -eq 0 ]; then
    echo "${GREEN}Basic Tests: PASSED${NC}"
else
    echo "${RED}Basic Tests: FAILED${NC}"
fi

if [ $KERNEL_RESULT -eq 0 ]; then
    echo "${GREEN}Kernel Tests: PASSED${NC}"
else
    echo "${RED}Kernel Tests: FAILED${NC}"
fi

# Exit with failure if any test failed
if [ $BASIC_RESULT -ne 0 ] || [ $KERNEL_RESULT -ne 0 ]; then
    exit 1
fi

echo ""
echo "${GREEN}All tests passed!${NC}"
exit 0
