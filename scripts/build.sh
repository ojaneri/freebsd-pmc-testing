#!/bin/sh
# @file build.sh
# @brief Build script for PMC test framework
# @version 1.0.0

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Project directories
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILDDIR="$PROJECT_DIR/build"

echo "========================================"
echo "PMC Test Framework Build Script"
echo "========================================"

# Check for FreeBSD
if [ "$(uname)" != "FreeBSD" ]; then
    echo "${YELLOW}Warning: This build is designed for FreeBSD${NC}"
    echo "Continuing anyway..."
fi

# Check for root privileges
if [ "$(id -u)" -ne 0 ]; then
    echo "${YELLOW}Warning: Not running as root. Some features may not work.${NC}"
fi

# Create build directory
echo "Creating build directory..."
mkdir -p "$BUILDDIR"

# Check for required tools
echo "Checking for required tools..."

for tool in gcc make; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "${RED}Error: $tool not found${NC}"
        exit 1
    fi
done

echo "${GREEN}All required tools found${NC}"

# Build the library
echo ""
echo "Building library..."
cd "$PROJECT_DIR"
make lib

if [ $? -eq 0 ]; then
    echo "${GREEN}Library built successfully${NC}"
else
    echo "${RED}Library build failed${NC}"
    exit 1
fi

# Build tests
echo ""
echo "Building tests..."
make framework-tests

if [ $? -eq 0 ]; then
    echo "${GREEN}Tests built successfully${NC}"
else
    echo "${RED}Test build failed${NC}"
    exit 1
fi

# Build examples
echo ""
echo "Building examples..."
make examples

if [ $? -eq 0 ]; then
    echo "${GREEN}Examples built successfully${NC}"
else
    echo "${RED}Example build failed${NC}"
    exit 1
fi

echo ""
echo "========================================"
echo "Build Complete!"
echo "========================================"
echo ""
echo "Executables:"
echo "  - $BUILDDIR/pmc_tests_basic"
echo "  - $BUILDDIR/pmc_tests_kernel"
echo "  - $PROJECT_DIR/examples/pmc_example_simple"
echo "  - $PROJECT_DIR/examples/pmc_example_perf"
echo ""
echo "To run tests:     make test"
echo "To install:       sudo make install"
echo "To clean:         make clean"
echo ""
