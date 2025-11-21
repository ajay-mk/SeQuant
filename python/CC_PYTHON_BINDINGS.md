# CC Class Python Bindings for SeQuant

## Overview

This document describes the Python bindings for the `mbpt::CC` class, which provides a derivation engine for coupled-cluster methods in SeQuant.

## What Was Added

### 1. Strong Types for Particle/Hole Counts

Added Python bindings for `nₚ` (number of particles) and `nₕ` (number of holes):

```python
from _sequant.mbpt import np, nh

n_particles = np(2)  # 2 particles
n_holes = nh(2)      # 2 holes
```

These strong types are required for EOM-CC methods to specify the operator manifold.

### 2. CC Ansatz Enum

Added the `Ansatz` enum with four types:

```python
from _sequant.mbpt import Ansatz

# Traditional ansatz: exp(T)
Ansatz.T

# Orbital-optimized (singles-free): exp(T₂ + T₃ + ...)
Ansatz.oT

# Unitary ansatz: exp(T - T†)
Ansatz.U

# Unitary orbital-optimized: exp(T₂ - T₂† + T₃ - T₃† + ...)
Ansatz.oU
```

### 3. Expression Utility Functions

Added the `size()` function for analyzing expressions:

```python
import _sequant as sq

# Get number of terms in an expression
num_terms = sq.size(expr)
```

The `size()` function returns:
- `0` for atomic expressions (Constant, Tensor, etc.)
- `>0` for composite expressions (Sum, Product)
- For a Sum, returns the number of summands
- For a Product, returns the number of factors

This is useful for testing and validating equation derivations.

### 4. CC Class

The main `CC` class for deriving coupled cluster equations:

#### Constructor

```python
CC(N, ansatz=Ansatz.T, screen=True, use_topology=True)
```

**Parameters:**
- `N` (int): Coupled cluster excitation rank (2=CCSD, 3=CCSDT, etc.)
- `ansatz` (Ansatz): Type of CC ansatz (default: T)
- `screen` (bool): Enable operator-level screening (default: True)
- `use_topology` (bool): Enable topological optimizations (default: True)

#### Methods

**Query Methods:**
- `ansatz()` → Returns the ansatz type
- `unitary()` → Returns True if ansatz is unitary (U or oU)
- `screen()` → Returns whether screening is enabled
- `use_topology()` → Returns whether topology optimization is enabled

**T Amplitude Equations:**
```python
t(commutator_rank=4, pmax=∞, pmin=0) → list of ExprPtr
```
Derives t amplitude equations: ⟨P|H̄|0⟩ = 0

**Lambda Amplitude Equations:**
```python
lambda_(commutator_rank=4) → list of ExprPtr
```
Derives λ amplitude equations: ⟨0|(1+Λ̂) dH̄/dT̂_P|0⟩ = 0

**Perturbed Amplitudes:**
```python
t_pt(rank=1, order=1, nbatch=None) → list of ExprPtr
lambda_pt(rank=1, order=1, nbatch=None) → list of ExprPtr
```
Derives first-order perturbed amplitude equations (for response properties)

**EOM-CC Equations:**
```python
eom_r(np, nh) → list of ExprPtr  # Right-side sigma equations
eom_l(np, nh) → list of ExprPtr  # Left-side sigma equations
```
Derives equation-of-motion CC equations for excited states

## Usage Examples

### Basic CCSD

```python
from _sequant.mbpt import CC

# Create CCSD engine
cc = CC(2)

# Derive T amplitude equations
t_eqs = cc.t()

# Access specific equations (element 0 is null)
t1_eq = t_eqs[1]  # T1 equation
t2_eq = t_eqs[2]  # T2 equation

# Get LaTeX representation
print(t1_eq.latex)
print(t2_eq.latex)
```

### Lambda Equations

```python
cc = CC(2)

# Derive λ equations
l_eqs = cc.lambda_()

lambda1_eq = l_eqs[1]
lambda2_eq = l_eqs[2]

print(lambda1_eq.latex)
```

### CCSDT (Rank 3)

```python
# CCSDT: rank 3
cc = CC(3)

t_eqs = cc.t()

# Now have T1, T2, and T3
t1_eq = t_eqs[1]
t2_eq = t_eqs[2]
t3_eq = t_eqs[3]
```

### Unitary Coupled Cluster

```python
from _sequant.mbpt import Ansatz

# Unitary CC (exp(T - T†))
ucc = CC(2, Ansatz.U)

# Often need higher commutator ranks for UCC
t_eqs = ucc.t(commutator_rank=6)

print(f"Is unitary: {ucc.unitary()}")  # True
```

### Orbital-Optimized CC

```python
# Orbital-optimized (singles-free)
occ = CC(2, Ansatz.oT)

t_eqs = occ.t()

# No T1 equation in orbital-optimized ansatz
# but T2 still exists
t2_eq = t_eqs[2]
```

### EOM-CCSD for Excitation Energies

```python
from _sequant.mbpt import np, nh

cc = CC(2)

# EE-EOM-CCSD: 2 particles, 2 holes
r_eqs = cc.eom_r(np(2), nh(2))
l_eqs = cc.eom_l(np(2), nh(2))

r1_eq = r_eqs[1]  # R1 sigma equation
r2_eq = r_eqs[2]  # R2 sigma equation
```

### IP-EOM and EA-EOM

```python
cc = CC(2)

# Ionization Potential: 0 particles, 1 hole
ip_eqs = cc.eom_r(np(0), nh(1))

# Electron Attachment: 1 particle, 0 holes
ea_eqs = cc.eom_r(np(1), nh(0))

# Double Ionization: 0 particles, 2 holes
dip_eqs = cc.eom_r(np(0), nh(2))
```

### Perturbed Amplitudes (Response Properties)

```python
cc = CC(2)

# First-order perturbed t amplitudes
t_pt = cc.t_pt(rank=1, order=1)

# First-order perturbed λ amplitudes
l_pt = cc.lambda_pt(rank=1, order=1)
```

### Custom Configuration

```python
# Disable screening
cc_no_screen = CC(2, screen=False)

# Disable topology optimization
cc_no_topo = CC(2, use_topology=False)

# Disable both optimizations
cc_minimal = CC(2, screen=False, use_topology=False)
```

### Custom Projector Manifolds

```python
cc = CC(3)  # CCSDT

# Generate only T2 and T3 equations (skip T1)
t_eqs = cc.t(pmin=2, pmax=3)

# Generate only T1 and T2 (skip T3)
t_eqs = cc.t(pmin=1, pmax=2)
```

## Return Value Format

All equation-generating methods return a `list` (or `vector`) of `ExprPtr` objects:
- Element 0 is always null/empty
- Element k contains the equation for rank k
- For example, `t_eqs[1]` is the T1 equation, `t_eqs[2]` is T2, etc.

## Working with Expressions

The returned expressions are `ExprPtr` objects that support:

```python
# Get LaTeX representation
latex_str = expr.latex

# Simplify expression
simplified = sq.simplify(expr)

# Get the number of terms/subexpressions
num_terms = sq.size(expr)  # Returns 0 for atoms, >0 for Sum/Product

# Arithmetic operations
result = expr1 + expr2
result = expr1 * expr2
result = expr ** 2
```

## Building and Testing

### Build Configuration

To build SeQuant with Python support:

```bash
cmake -B build -S . \
  -DSEQUANT_PYTHON=ON \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --target python-sequant
```

### Running Tests

```bash
# Set PYTHONPATH to find the module
export PYTHONPATH=/path/to/SeQuant/build:$PYTHONPATH

# Run the CC-specific tests
python python/test_cc.py

# Run the basic SeQuant tests (now includes CC imports)
python python/test_sequant.py

# Run the examples
python python/examples_cc.py
```

## Files Modified/Created

### Modified Files:
1. `/home/user/SeQuant/python/src/sequant/mbpt.h`
   - Added include for `cc.hpp`
   - Added `np` and `nh` strong type bindings
   - Added `Ansatz` enum bindings
   - Added `CC` class bindings with all methods
   - Added comprehensive docstrings

2. `/home/user/SeQuant/python/src/sequant/_sequant.cc`
   - Added include for `expr_algorithms.hpp`
   - Added `size()` function binding for counting terms in expressions

3. `/home/user/SeQuant/python/test_sequant.py`
   - Updated imports to include CC, Ansatz, np, nh

### Created Files:
1. `/home/user/SeQuant/python/test_cc.py`
   - Unit tests matching C++ tests in `tests/unit/test_mbpt_cc.cpp`
   - Tests CCSD T amplitude equations with exact term count validation
   - Tests EOM-CC (EE, IP, EA) equations with size assertions
   - Tests UCC equations
   - 5 test classes with 10+ test methods

2. `/home/user/SeQuant/python/examples_cc.py`
   - 11 detailed examples covering:
     - Basic CCSD
     - Lambda equations
     - CCSDT
     - Unitary CC
     - Orbital-optimized CC
     - EOM-CCSD (EE, IP, EA)
     - Perturbation theory
     - Configuration options
     - Custom projector manifolds
     - Method comparisons

3. `/home/user/SeQuant/python/CC_PYTHON_BINDINGS.md`
   - This documentation file

## Implementation Notes

### Memory Management
- Uses pybind11's holder type system for `ExprPtr`
- Automatic reference counting and garbage collection
- No manual memory management required from Python

### String Conversion
- Strong types (`np`, `nh`) have proper `__repr__` methods
- Expressions have `.latex` property for LaTeX output

### Error Handling
- C++ exceptions are automatically translated to Python exceptions
- Invalid parameters will raise appropriate errors

## Compatibility

- **Python Version**: 3.8+
- **pybind11 Version**: 3.0.1+
- **C++ Standard**: C++17+
- **Platforms**: Linux, macOS, Windows (where SeQuant builds)

## Future Enhancements

Potential additions to the Python interface:
1. Spin-tracing functions (`closed_shell_CC_spintrace`, `open_shell_CC_spintrace`)
2. Export functions (to Wolfram, JSON, etc.)
3. Canonicalization functions
4. Custom index space creation
5. Context managers for default settings
6. NumPy integration for numerical evaluation

## Support

For issues or questions:
- GitHub Issues: https://github.com/ValeevGroup/SeQuant/issues
- Documentation: https://valeevgroup.github.io/SeQuant/

## License

Same as SeQuant: LGPL v3
