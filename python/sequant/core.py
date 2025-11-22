"""
Core Expression and Tensor Manipulation
========================================

This module provides access to SeQuant's core tensor algebra functionality,
including expression types, simplification, canonicalization, and context
management.

Classes
-------
ExprPtr
    Smart pointer to an expression object
Expr
    Base expression class
DefaultTensorCanonicalizer
    Default canonicalization strategy
Vacuum
    Enum for vacuum types (Physical, SingleProduct, MultiProduct)
SPBasis
    Enum for single-particle basis types (Spinor, Spinfree)

Functions
---------
simplify(expr)
    Simplify an expression by expansion, canonicalization, and algebra
canonicalize(expr)
    Canonicalize tensor indices in an expression
expand(expr)
    Expand products of sums
flatten(expr)
    Flatten nested sums and products
rapid_simplify(expr)
    Fast simplification without canonicalization
size(expr)
    Get the number of subexpressions in an expression

Context Management
------------------
get_default_context()
    Get the default context
set_vacuum(vacuum)
    Set the vacuum type
set_spbasis(spbasis)
    Set the single-particle basis
get_vacuum()
    Get the current vacuum type
get_spbasis()
    Get the current single-particle basis
reset_default_context()
    Reset context to defaults

Examples
--------
>>> import sequant.core as core
>>> # Work with expressions
>>> expr = ...
>>> simplified = core.simplify(expr)
>>> canonical = core.canonicalize(expr)
>>> expanded = core.expand(expr)
>>>
>>> # Context management
>>> core.set_vacuum(core.Vacuum.SingleProduct)
>>> core.set_spbasis(core.SPBasis.Spinfree)
>>> current_vacuum = core.get_vacuum()
"""

# Re-export everything from the C++ module
from _sequant import (
    # Types
    ExprPtr,
    Expr,
    DefaultTensorCanonicalizer,
    # Expression manipulation
    simplify,
    canonicalize,
    expand,
    flatten,
    rapid_simplify,
    size,
    # Context enums
    Vacuum,
    SPBasis,
    # Context management
    get_default_context,
    set_vacuum,
    set_spbasis,
    get_vacuum,
    get_spbasis,
    reset_default_context,
)

__all__ = [
    # Types
    "ExprPtr",
    "Expr",
    "DefaultTensorCanonicalizer",
    # Expression manipulation
    "simplify",
    "canonicalize",
    "expand",
    "flatten",
    "rapid_simplify",
    "size",
    # Context enums
    "Vacuum",
    "SPBasis",
    # Context management
    "get_default_context",
    "set_vacuum",
    "set_spbasis",
    "get_vacuum",
    "get_spbasis",
    "reset_default_context",
]
