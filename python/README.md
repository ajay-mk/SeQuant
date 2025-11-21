# SeQuant Python Interface

Python bindings for [SeQuant](https://github.com/ValeevGroup/SeQuant), a symbolic tensor algebra library for quantum many-body theory.

## Features

- **Core Expression Manipulation**: Work with symbolic tensor expressions
- **Coupled Cluster Methods**: Derive CC equations (CCSD, CCSDT, UCC, etc.)
- **EOM-CC**: Equation-of-motion methods for excited states
- **Spin-Tracing**: Transform spin-free equations for closed/open-shell systems
- **LaTeX Output**: Generate publication-ready equations

## Installation

### Prerequisites

1. Build SeQuant with Python support:
   ```bash
   cmake -B build -S . \
     -DSEQUANT_PYTHON=ON \
     -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
     -DCMAKE_BUILD_TYPE=Release

   cmake --build build --target python-sequant
   ```

2. Set PYTHONPATH to include the build directory:
   ```bash
   export PYTHONPATH=/path/to/SeQuant/build:$PYTHONPATH
   ```

### Installing the Package (Optional)

After building, you can install the Python package:
```bash
cd python
pip install .
```

Or for development:
```bash
pip install -e .
```

## Quick Start

### Basic CCSD

```python
import sequant as sq
from sequant.mbpt import CC

# Create CCSD engine
cc = CC(2)

# Derive T amplitude equations
t_eqs = cc.t()

# Access equations
t1_eq = t_eqs[1]  # T1 equation
t2_eq = t_eqs[2]  # T2 equation

# Get LaTeX output
print(t1_eq.latex)
```

### EOM-CCSD for Excitation Energies

```python
from sequant.mbpt import CC, np, nh

cc = CC(2)

# EE-EOM-CCSD: 2 particles, 2 holes
r_eqs = cc.eom_r(np(2), nh(2))
l_eqs = cc.eom_l(np(2), nh(2))

# IP-EOM-CCSD: 0 particles, 1 hole
ip_eqs = cc.eom_r(np(0), nh(1))

# EA-EOM-CCSD: 1 particle, 0 holes
ea_eqs = cc.eom_r(np(1), nh(0))
```

### Unitary Coupled Cluster

```python
from sequant.mbpt import CC, Ansatz

# Unitary CC: exp(T - T†)
ucc = CC(2, Ansatz.U)

# Higher commutator ranks often needed for UCC
t_eqs = ucc.t(commutator_rank=6)
```

### Spin-Tracing

```python
from sequant.mbpt import CC, closed_shell_CC_spintrace

cc = CC(2)
t_eqs = cc.t()

# Transform to spin-orbital form
t2_spin = closed_shell_CC_spintrace(t_eqs[2])
```

## Package Structure

```
sequant/
├── __init__.py          # Main package interface
├── core.py              # Core expression types and functions
└── mbpt.py              # Many-body perturbation theory
```

### Import Styles

Recommended:
```python
import sequant as sq
from sequant.mbpt import CC, Ansatz, np, nh
```

Alternative:
```python
from sequant import simplify, size
import sequant.mbpt as mbpt
```

## API Overview

### Core Module (`sequant.core`)

- **ExprPtr**: Smart pointer to expression objects
- **Expr**: Base expression class
- **simplify(expr)**: Simplify expressions
- **size(expr)**: Count terms in expression
- **DefaultTensorCanonicalizer**: Canonicalization engine

### MBPT Module (`sequant.mbpt`)

#### Classes

- **CC**: Coupled cluster derivation engine
  - `t()`: T amplitude equations
  - `lambda_()`: λ amplitude equations
  - `eom_r()`, `eom_l()`: EOM-CC equations
  - `t_pt()`, `lambda_pt()`: Perturbed amplitudes

- **Ansatz**: Enum for CC ansatz types
  - `T`: Traditional exp(T)
  - `oT`: Orbital-optimized (singles-free)
  - `U`: Unitary exp(T - T†)
  - `oU`: Unitary orbital-optimized

- **np**, **nh**: Strong types for particle/hole counts

#### Functions

- **H(k)**: Hamiltonian operator
- **F()**: Fock operator
- **T(rank)**, **A(rank)**: Cluster operators
- **VacuumAverage(expr)**: Compute vacuum average
- **closed_shell_CC_spintrace(expr)**: Closed-shell spin-tracing
- **open_shell_CC_spintrace(expr)**: Open-shell spin-tracing

## Examples

See the `examples_cc.py` file for comprehensive examples covering:
- CCSD, CCSDT equations
- Lambda equations
- EOM-CC (EE, IP, EA)
- Unitary and orbital-optimized CC
- Perturbation theory
- Custom configurations

## Testing

Run the test suite:
```bash
python -m pytest test_sequant.py test_cc.py -v
```

Or using unittest:
```bash
python test_cc.py
python test_sequant.py
```

## Documentation

For detailed API documentation, see:
- [CC_PYTHON_BINDINGS.md](CC_PYTHON_BINDINGS.md) - Comprehensive CC class guide
- [SeQuant Documentation](https://valeevgroup.github.io/SeQuant/)

## Requirements

- Python 3.8+
- SeQuant C++ library
- pybind11 (included in SeQuant build)

## License

LGPL-3.0 (same as SeQuant)

## Contributing

Contributions are welcome! Please see the main [SeQuant repository](https://github.com/ValeevGroup/SeQuant) for contribution guidelines.

## Support

- **Issues**: https://github.com/ValeevGroup/SeQuant/issues
- **Documentation**: https://valeevgroup.github.io/SeQuant/
