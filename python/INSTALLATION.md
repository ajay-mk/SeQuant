# SeQuant Python Package - Installation Guide

This guide explains how to build, install, and use the SeQuant Python package.

## Table of Contents

1. [Building the C++ Extension](#building-the-c-extension)
2. [Installation Options](#installation-options)
3. [Import Styles](#import-styles)
4. [Testing the Installation](#testing-the-installation)
5. [Troubleshooting](#troubleshooting)

## Building the C++ Extension

Before installing or using the Python package, you must build the C++ extension module.

### Prerequisites

- CMake 3.15+
- C++17 compatible compiler
- Python 3.8+
- pybind11 (will be fetched automatically by CMake)

### Build Steps

```bash
# Configure with Python support
cmake -B build -S . \
  -DSEQUANT_PYTHON=ON \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_BUILD_TYPE=Release

# Build the Python extension
cmake --build build --target python-sequant
```

This creates:
- `build/python/_sequant.so` (or `.pyd` on Windows) - the compiled C++ extension
- `build/python/sequant/` - the Python package with the extension copied inside

## Installation Options

After building, you have three options for using the package:

### Option 1: Direct PYTHONPATH (Development)

The simplest option for development - no installation needed:

```bash
export PYTHONPATH=/path/to/SeQuant/build/python:$PYTHONPATH
python
```

```python
>>> import sequant as sq
>>> from sequant.mbpt import CC
>>> cc = CC(2)
```

**Pros:**
- No installation step
- Changes to Python files are immediately available
- Easy to switch between branches

**Cons:**
- Need to set PYTHONPATH every time
- Only available in current shell

### Option 2: Pip Install (Editable)

Install in development/editable mode:

```bash
cd python
pip install -e .
```

**Pros:**
- Available system-wide (in current Python environment)
- No need to set PYTHONPATH
- Changes to Python files are immediately available
- Can be used with virtual environments

**Cons:**
- Need to reinstall if Python files are moved
- Requires pip

### Option 3: Pip Install (Regular)

Install as a regular package:

```bash
cd python
pip install .
```

**Pros:**
- Clean installation
- Available system-wide
- Works like any other Python package

**Cons:**
- Changes to source files not reflected until reinstall
- Need to reinstall after Python file changes

## Import Styles

The new package structure supports multiple import styles:

### Recommended Style (New)

```python
import sequant as sq
from sequant.mbpt import CC, Ansatz, np, nh

# Use the package
cc = CC(2)
t_eqs = cc.t()
```

### Alternative Styles

```python
# Import everything from sequant
from sequant import simplify, size
from sequant.mbpt import CC

# Import submodules explicitly
import sequant.core as core
import sequant.mbpt as mbpt
cc = mbpt.CC(2)

# Import specific items
from sequant.mbpt import (
    CC, Ansatz,
    np, nh,
    H, F, T, A,
    closed_shell_CC_spintrace
)
```

### Backward Compatibility

The old import style still works for backward compatibility:

```python
import _sequant as sq
from _sequant.mbpt import CC, Ansatz, np, nh
```

All test files support both import styles automatically.

## Testing the Installation

### Quick Test

```python
python -c "import sequant; from sequant.mbpt import CC; print('SeQuant imported successfully!')"
```

### Run Test Suite

Using pytest (recommended):
```bash
cd /path/to/SeQuant/python
pytest test_sequant.py test_cc.py -v
```

Using unittest:
```bash
python test_sequant.py
python test_cc.py
```

### Run Examples

```bash
python examples_cc.py
```

## Package Structure

After installation, the package structure is:

```
sequant/
├── __init__.py          # Main package, re-exports core items
├── core.py              # Core expression types and functions
├── mbpt.py              # MBPT-specific functionality
├── _sequant.so          # Compiled C++ extension (Linux/Mac)
│   or _sequant.pyd      # (Windows)
└── py.typed             # PEP 561 type marker
```

## Virtual Environments

Recommended for isolation:

```bash
# Create virtual environment
python -m venv sequant-env

# Activate it
source sequant-env/bin/activate  # Linux/Mac
# or
sequant-env\Scripts\activate  # Windows

# Install
cd /path/to/SeQuant/python
pip install -e .

# Use
python
>>> import sequant as sq
```

## Uninstallation

If installed with pip:
```bash
pip uninstall sequant
```

If using PYTHONPATH only:
```bash
# Just unset or remove from PYTHONPATH
unset PYTHONPATH
```

## Troubleshooting

### Import Error: "Cannot import _sequant module"

**Cause:** The C++ extension wasn't built or isn't in the Python path.

**Solutions:**
1. Build the extension: `cmake --build build --target python-sequant`
2. Check PYTHONPATH includes `build/python`
3. If using pip install, rebuild and reinstall

### Import Error: "No module named 'sequant'"

**Cause:** Package not installed or not in PYTHONPATH.

**Solutions:**
1. Set PYTHONPATH: `export PYTHONPATH=/path/to/SeQuant/build/python:$PYTHONPATH`
2. Or install with pip: `cd python && pip install -e .`

### CMake Error: "Could not find pybind11"

**Cause:** pybind11 not found and network access blocked.

**Solution:** Build SeQuant first with `-DSEQUANT_PYTHON=ON`, CMake will fetch pybind11 automatically.

### Wrong Python Version

**Cause:** CMake found a different Python version.

**Solution:** Specify Python explicitly:
```bash
cmake -B build -S . \
  -DSEQUANT_PYTHON=ON \
  -DPython_EXECUTABLE=/path/to/python3.10
```

### Tests Fail with Import Errors

**Cause:** Test files can't find the package.

**Solutions:**
1. For PYTHONPATH method: ensure PYTHONPATH is set
2. For pip install: make sure installation succeeded
3. Run tests from the correct directory

## Advanced: Building Wheels

To create distributable wheel packages:

```bash
cd python
pip install build
python -m build
```

This creates:
- `dist/sequant-0.1.0-py3-none-any.whl`
- `dist/sequant-0.1.0.tar.gz`

**Note:** The wheel won't include the compiled extension. Users must build it separately.

## Development Workflow

Recommended setup for SeQuant development:

```bash
# 1. Build C++ library and Python extension
cmake -B build -S . -DSEQUANT_PYTHON=ON
cmake --build build --target python-sequant

# 2. Install in editable mode
cd python
pip install -e .

# 3. Work on code
# ... make changes to Python or C++ files ...

# 4. If C++ changed, rebuild extension
cd ..
cmake --build build --target python-sequant

# 5. If only Python changed, no rebuild needed (editable install)
# Just run tests
pytest python/test_cc.py -v
```

## Next Steps

- Read the [README.md](README.md) for API overview
- Check [CC_PYTHON_BINDINGS.md](CC_PYTHON_BINDINGS.md) for detailed CC class documentation
- Explore [examples_cc.py](examples_cc.py) for usage examples
- Read the main [SeQuant documentation](https://valeevgroup.github.io/SeQuant/)
