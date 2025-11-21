# SeQuant Python Package Structure

This document describes the improved package structure for the SeQuant Python interface.

## Overview

The SeQuant Python interface has been reorganized into a proper Python package with a clean API, supporting both modern import styles and backward compatibility.

## Package Structure

```
python/
├── sequant/                    # Main Python package
│   ├── __init__.py            # Package initialization, re-exports core items
│   ├── core.py                # Core expression types and functions
│   ├── mbpt.py                # MBPT-specific functionality
│   └── py.typed               # PEP 561 type marker
├── src/                        # C++ source (unchanged)
│   └── sequant/
│       ├── _sequant.cc        # Main pybind11 module
│       ├── mbpt.h             # MBPT bindings
│       └── python.h           # Common utilities
├── test_sequant.py            # Core tests (updated for new imports)
├── test_cc.py                 # CC-specific tests (updated)
├── examples_cc.py             # Usage examples (updated)
├── setup.py                   # Package installation script (NEW)
├── pyproject.toml             # Modern Python package metadata (NEW)
├── MANIFEST.in                # Package file inclusion rules (NEW)
├── README.md                  # Package overview (NEW)
├── INSTALLATION.md            # Installation guide (NEW)
├── PACKAGE_STRUCTURE.md       # This file (NEW)
├── CC_PYTHON_BINDINGS.md      # CC class documentation (updated)
└── CMakeLists.txt             # Build configuration (updated)
```

## Key Improvements

### 1. Proper Package Organization

**Before:**
```python
import _sequant as sq
from _sequant.mbpt import CC
```

**After:**
```python
import sequant as sq
from sequant.mbpt import CC
```

The compiled C++ module `_sequant` is now an internal implementation detail, wrapped by a clean Python package.

### 2. Submodule Organization

The package is organized into logical submodules:

- **`sequant`**: Top-level package with commonly used items
- **`sequant.core`**: Core expression types and manipulation functions
- **`sequant.mbpt`**: Many-body perturbation theory functionality

### 3. Installation Support

Multiple installation methods are now supported:

1. **PYTHONPATH** (development): Add `build/python` to PYTHONPATH
2. **Pip editable** (development): `pip install -e .`
3. **Pip regular** (deployment): `pip install .`

### 4. Backward Compatibility

All old import styles continue to work:
```python
# Old style still works
import _sequant as sq
from _sequant.mbpt import CC

# New style (recommended)
import sequant as sq
from sequant.mbpt import CC
```

### 5. Modern Python Packaging

- **setup.py**: Traditional installation script
- **pyproject.toml**: Modern build system configuration (PEP 518)
- **MANIFEST.in**: Control what gets included in distributions
- **py.typed**: Type checking support marker (PEP 561)

## Import Styles

### Recommended (New)

```python
# Import the package
import sequant as sq
from sequant.mbpt import CC, Ansatz, np, nh

# Use submodules explicitly
import sequant.core as core
import sequant.mbpt as mbpt

# Import specific items
from sequant import simplify, size
from sequant.mbpt import CC, closed_shell_CC_spintrace
```

### Backward Compatible (Old)

```python
# Still works
import _sequant as sq
from _sequant.mbpt import CC, Ansatz, np, nh
```

## Build System Changes

### CMakeLists.txt

Added commands to:
1. Copy Python package files to build directory
2. Copy compiled extension into the package
3. Maintain backward compatibility with old PYTHONPATH usage

```cmake
# Copy Python package files to build directory
add_custom_command(TARGET python-sequant POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/sequant
    ${CMAKE_CURRENT_BINARY_DIR}/sequant
)

# Copy the compiled extension into the package
add_custom_command(TARGET python-sequant POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:python-sequant>
    ${CMAKE_CURRENT_BINARY_DIR}/sequant/_sequant$<TARGET_FILE_SUFFIX:python-sequant>
)
```

After building, both import styles work:
- Old: `build/python/_sequant.so` (backward compatibility)
- New: `build/python/sequant/_sequant.so` (package structure)

## API Organization

### Top Level (`sequant`)

Re-exports commonly used items for convenience:
- `ExprPtr`, `Expr`: Expression types
- `simplify()`, `size()`: Utility functions
- `DefaultTensorCanonicalizer`: Canonicalization

Provides access to submodules:
- `sequant.core`: Core functionality
- `sequant.mbpt`: MBPT functionality

### Core Module (`sequant.core`)

- Expression types: `ExprPtr`, `Expr`
- Functions: `simplify()`, `size()`
- Canonicalization: `DefaultTensorCanonicalizer`

### MBPT Module (`sequant.mbpt`)

- Classes: `CC`, `np`, `nh`, `ClosedShellCCSpintraceOptions`
- Enums: `Ansatz`, `OpType`, `BiorthogonalizationMethod`
- Operators: `H()`, `F()`, `T()`, `A()`, `T_()`
- Functions: `VacuumAverage()`, `closed_shell_CC_spintrace()`, `open_shell_CC_spintrace()`

## Testing

All test files support both import styles automatically:

```python
# Tests try new style first, fall back to old
try:
    from sequant.mbpt import CC
except ImportError:
    from _sequant.mbpt import CC
```

Run tests with:
```bash
pytest test_sequant.py test_cc.py -v
# or
python test_cc.py
```

## Documentation

- **README.md**: Quick start and API overview
- **INSTALLATION.md**: Detailed installation instructions
- **CC_PYTHON_BINDINGS.md**: Comprehensive CC class documentation
- **PACKAGE_STRUCTURE.md**: This file - package organization

## Migration Guide

### For Users

**No changes required!** Old code continues to work.

To use the new style:
```python
# Change:
from _sequant.mbpt import CC
# To:
from sequant.mbpt import CC
```

### For Developers

When adding new functionality:

1. Add C++ bindings in `src/sequant/_sequant.cc` or `src/sequant/mbpt.h`
2. Re-export in appropriate Python module:
   - Core functionality: `sequant/core.py`
   - MBPT functionality: `sequant/mbpt.py`
3. Update `__all__` lists for clean namespace
4. Update documentation

## Benefits

1. **Professional**: Follows Python packaging best practices
2. **Installable**: Can be installed with `pip install`
3. **Discoverable**: Works with IDEs, type checkers, and documentation tools
4. **Organized**: Clear separation between core and domain-specific code
5. **Compatible**: Existing code continues to work without changes
6. **Maintainable**: Easy to extend with new submodules

## Future Enhancements

Potential additions:
- Type stubs (`.pyi` files) for better IDE support
- Sphinx documentation generation
- NumPy integration
- Additional submodules (e.g., `sequant.spaces`, `sequant.io`)
- Wheel distribution on PyPI

## Technical Notes

### Why Both `_sequant` Locations?

The build system creates the compiled module in two places:

1. **`build/python/_sequant.so`**: For backward compatibility with existing scripts
2. **`build/python/sequant/_sequant.so`**: For the new package structure

This allows both import styles to work without requiring users to change their code.

### PEP Compliance

- **PEP 518**: Build system requirements in `pyproject.toml`
- **PEP 561**: Type information marker with `py.typed`
- **PEP 621**: Project metadata in `pyproject.toml`

### Virtual Environment Support

The package works seamlessly with:
- `venv`
- `virtualenv`
- `conda`
- `pipenv`
- `poetry`

## Related Files

- [README.md](README.md) - Package overview and quick start
- [INSTALLATION.md](INSTALLATION.md) - Installation guide
- [CC_PYTHON_BINDINGS.md](CC_PYTHON_BINDINGS.md) - CC class documentation
- [setup.py](setup.py) - Installation script
- [pyproject.toml](pyproject.toml) - Package metadata
