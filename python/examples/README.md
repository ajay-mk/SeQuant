# SeQuant Python Examples

This directory contains examples demonstrating the SeQuant Python interface.

## Contents

### Jupyter Notebooks

- **[SeQuant_Tutorial.ipynb](SeQuant_Tutorial.ipynb)** - Comprehensive tutorial covering:
  - Context configuration (vacuum, basis)
  - CCSD, CCSDT equation derivation
  - Lambda and EOM-CCSD equations
  - Unitary coupled cluster
  - Expression manipulation and simplification
  - Spin-tracing
  - Jupyter-specific display features

### Python Scripts

- **[examples_cc.py](../examples_cc.py)** - Standalone examples for coupled cluster methods

## Running the Notebooks

### Prerequisites

1. Build SeQuant with Python support:
   ```bash
   cmake -B build -S . -DSEQUANT_PYTHON=ON
   cmake --build build --target python-sequant
   ```

2. Install Jupyter (if not already installed):
   ```bash
   pip install jupyter notebook
   ```

### Launch

From the SeQuant root directory:

```bash
# Set PYTHONPATH to include build directory
export PYTHONPATH=$(pwd)/build/python:$PYTHONPATH

# Launch Jupyter
cd python/examples
jupyter notebook
```

Then open `SeQuant_Tutorial.ipynb` in the browser.

## Features in Notebooks

### Automatic LaTeX Rendering

Expressions automatically render as LaTeX in Jupyter:

```python
from sequant.mbpt import CC
cc = CC(2)
t_eqs = cc.t()
t_eqs[1]  # Displays as formatted LaTeX equation
```

### Display Helpers

The `sequant.jupyter` module provides utilities for enhanced display:

```python
from sequant.jupyter import display_eqs, compare_eqs, show_context

# Display multiple equations with labels
display_eqs({
    "T₁ Equation": t_eqs[1],
    "T₂ Equation": t_eqs[2]
})

# Compare two expressions side-by-side
from sequant import simplify
compare_eqs(expr, simplify(expr), "Original", "Simplified")

# Show current context settings
show_context()
```

## Jupyter-Specific Features

### Rich Display

All `ExprPtr` and `Expr` objects have:
- `_repr_latex_()` - Automatic LaTeX rendering
- `_repr_html_()` - HTML fallback with styling
- `__repr__()` - Plain text representation (truncated if long)

### Context Visualization

Use `show_context()` to see current SeQuant settings in a formatted table.

## Creating Your Own Notebooks

Template for a SeQuant notebook:

```python
import sequant as sq
from sequant.mbpt import CC, Ansatz, np, nh
from sequant.jupyter import display_eqs, show_context

# Configure context
sq.set_vacuum(sq.Vacuum.SingleProduct)
sq.set_spbasis(sq.SPBasis.Spinfree)
show_context()

# Your calculations here...
cc = CC(2)
t_eqs = cc.t()
display_eqs({"T1": t_eqs[1], "T2": t_eqs[2]})
```

## Troubleshooting

### "Cannot import _sequant"

Make sure `PYTHONPATH` includes the build directory:
```bash
export PYTHONPATH=/path/to/SeQuant/build/python:$PYTHONPATH
```

### LaTeX not rendering

Install `notebook` (not just `jupyter-core`):
```bash
pip install notebook
```

Make sure MathJax is enabled in your notebook settings.

### IPython not available

The `sequant.jupyter` module requires IPython:
```bash
pip install ipython
```

## Additional Resources

- [SeQuant Documentation](https://valeevgroup.github.io/SeQuant/)
- [Python Interface README](../README.md)
- [Installation Guide](../INSTALLATION.md)
- [CC Bindings Documentation](../CC_PYTHON_BINDINGS.md)
