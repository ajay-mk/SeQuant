"""
Core Expression and Tensor Manipulation
========================================

This module provides access to SeQuant's core tensor algebra functionality,
including expression types, simplification, and canonicalization.

Classes
-------
ExprPtr
    Smart pointer to an expression object
Expr
    Base expression class
DefaultTensorCanonicalizer
    Default canonicalization strategy

Functions
---------
simplify(expr)
    Simplify an expression
size(expr)
    Get the number of subexpressions in an expression

Examples
--------
>>> import sequant.core as core
>>> # Work with expressions
>>> expr = ...
>>> simplified = core.simplify(expr)
>>> num_terms = expr.size()  # or core.size(expr)
"""

# Re-export everything from the C++ module
from _sequant import (
    ExprPtr,
    Expr,
    DefaultTensorCanonicalizer,
    simplify,
    size,
)

__all__ = [
    "ExprPtr",
    "Expr",
    "DefaultTensorCanonicalizer",
    "simplify",
    "size",
]
