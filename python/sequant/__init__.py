"""
SeQuant Python Interface
=========================

A Python interface to the SeQuant library for symbolic tensor algebra
in quantum many-body theory.

Submodules
----------
core
    Core expression and tensor manipulation
mbpt
    Many-body perturbation theory specific functionality

Examples
--------
>>> import sequant as sq
>>> from sequant.mbpt import CC, Ansatz, np, nh
>>> cc = CC(2)  # CCSD
>>> t_eqs = cc.t()
>>> print(t_eqs[1].latex)
"""

from typing import TYPE_CHECKING

# Import the compiled C++ module
try:
    import _sequant
except ImportError as e:
    raise ImportError(
        "Cannot import _sequant module. Make sure SeQuant is built with "
        "Python support enabled (SEQUANT_PYTHON=ON) and that the build "
        "directory is in your PYTHONPATH."
    ) from e

# Version information
__version__ = "0.1.0"
__author__ = "The SeQuant Developers"

# Re-export core functionality at package level for convenience
from _sequant import (
    # Expression types
    ExprPtr,
    Expr,
    # Canonicalization
    DefaultTensorCanonicalizer,
    # Utility functions
    simplify,
    size,
)

# Create submodule aliases
from . import core
from . import mbpt

# Define what gets imported with "from sequant import *"
__all__ = [
    # Submodules
    "core",
    "mbpt",
    # Core types
    "ExprPtr",
    "Expr",
    # Functions
    "simplify",
    "size",
    # Canonicalization
    "DefaultTensorCanonicalizer",
    # Version
    "__version__",
]


def __dir__():
    """Custom dir() to show available symbols."""
    return __all__
