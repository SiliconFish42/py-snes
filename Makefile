# PySNES Makefile

.PHONY: all build test run_tests clean format lint venv install help

# Check for uv
UV := $(shell command -v uv 2> /dev/null)
ifeq ($(UV),)
$(error "uv is not installed. Please install it from https://github.com/astral-sh/uv")
endif

# Directories
BUILD_DIR=build
TEST_OUTPUT_DIR=test_output

# Python commands
PYTHON=python3
UV=uv

# Default target
all: build

# Build Python package and C++ extension using uv/scikit-build-core
build:
	$(UV) pip install -e .

# Build C++ test suite only
build_tests:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake ..
	cd $(BUILD_DIR) && make run_tests

# Run all tests (Python and C++)
test: build build_tests
	$(PYTHON) tests/scripts/run_comprehensive_tests.py
	@echo "[DEBUG] Checking for C++ test binary at $(BUILD_DIR)/run_tests"
	@if [ -f $(BUILD_DIR)/run_tests ]; then \
		echo "[DEBUG] Running C++/GTest tests..."; \
		cd $(BUILD_DIR) && ./run_tests $(ARGS) \
	else \
		echo "C++ test binary not found, skipping C++/GTest tests."; \
	fi

# Usage: make run_tests ARGS="--gtest_filter=PPUTest.BGPriorityLogic"
run_tests:
	@if [ ! -d $(BUILD_DIR) ] || [ ! -f $(BUILD_DIR)/run_tests ]; then \
		$(MAKE) build_tests; \
	fi
	cd $(BUILD_DIR) && ./run_tests $(ARGS)

# Make clean before running tests
clean_tests: clean run_tests

# Clean build and test outputs
clean:
	rm -rf $(BUILD_DIR) $(TEST_OUTPUT_DIR)

# Format code (C++ and Python)
format:
	@echo "Formatting C++ with clang-format and Python with black..."
	find src/ -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
	$(PYTHON) -m black src/ tests/ tools/

# Lint code (C++ and Python)
lint:
	@echo "Linting Python with flake8 and C++ with clang-tidy..."
	$(PYTHON) -m flake8 src/ tests/ tools/
	find src/ -name '*.cpp' | xargs -I{} clang-tidy {} --

# Set up Python virtual environment
venv:
	$(UV) venv
	$(UV) sync

# Install Python package in editable mode
install:
	$(UV) pip install -e .

# Show help
help:
	@echo "PySNES Makefile - Common development tasks:"
	@echo "  all/build     Build the C++ core and Python extension"
	@echo "  test          Run all tests (Python and C++)"
	@echo "  run_tests     Alias for test"
	@echo "  clean         Remove build and test outputs"
	@echo "  format        Format C++ and Python code"
	@echo "  lint          Lint C++ and Python code"
	@echo "  venv          Set up Python virtual environment with uv"
	@echo "  install       Install Python package in editable mode"
	@echo "  help          Show this help message" 