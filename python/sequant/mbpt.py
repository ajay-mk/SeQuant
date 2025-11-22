"""
Many-Body Perturbation Theory (MBPT)
=====================================

This module provides functionality for many-body perturbation theory,
including coupled cluster methods, operators, and spin-tracing.

Classes
-------
CC
    Coupled cluster derivation engine
Ansatz
    Enum for CC ansatz types (T, oT, U, oU)
np
    Strong type for number of particles
nh
    Strong type for number of holes
OpType
    Enum for operator types (h, f, t)
BiorthogonalizationMethod
    Enum for biorthogonalization methods (V1, V2)
ClosedShellCCSpintraceOptions
    Options for closed-shell CC spin-tracing
CSV
    Enum for cluster-specific virtuals option (Yes, No)

Functions
---------
F()
    Fock operator
H(k=2)
    Hamiltonian operator with up to k-body terms
A(Bra)
    Cluster excitation operator
T(Bra)
    Cluster excitation operator T
T_(Bra)
    De-excitation operator T†
VacuumAverage(expr, [pairs])
    Compute vacuum average of an expression
closed_shell_CC_spintrace(expr, options)
    Spin-trace closed-shell CC equations
open_shell_CC_spintrace(expr)
    Spin-trace open-shell CC equations
get_csv()
    Get current CSV setting
set_csv(csv)
    Set CSV (cluster-specific virtuals) option

Examples
--------
Basic CCSD:

>>> from sequant.mbpt import CC
>>> cc = CC(2)  # CCSD
>>> t_eqs = cc.t()
>>> print(t_eqs[1].latex)  # T1 equation
>>> print(t_eqs[2].latex)  # T2 equation

EOM-CCSD for excitation energies:

>>> from sequant.mbpt import CC, np, nh
>>> cc = CC(2)
>>> r_eqs = cc.eom_r(np(2), nh(2))  # EE-EOM-CCSD

Unitary CC:

>>> from sequant.mbpt import CC, Ansatz
>>> ucc = CC(2, Ansatz.U)
>>> t_eqs = ucc.t(commutator_rank=6)

Spin-tracing:

>>> from sequant.mbpt import CC, closed_shell_CC_spintrace
>>> cc = CC(2)
>>> t_eqs = cc.t()
>>> t2_spin = closed_shell_CC_spintrace(t_eqs[2])
"""

# Re-export everything from the C++ module's mbpt submodule
from _sequant.mbpt import (
    # Enums
    OpType,
    Ansatz,
    BiorthogonalizationMethod,
    CSV,
    # Classes
    CC,
    np,
    nh,
    ClosedShellCCSpintraceOptions,
    # Operator functions
    F,
    H,
    A,
    T,
    T_,
    # Functions
    VacuumAverage,
    closed_shell_CC_spintrace,
    open_shell_CC_spintrace,
    # MBPT Context
    get_csv,
    set_csv,
)

__all__ = [
    # Enums
    "OpType",
    "Ansatz",
    "BiorthogonalizationMethod",
    "CSV",
    # Classes
    "CC",
    "np",
    "nh",
    "ClosedShellCCSpintraceOptions",
    # Operators
    "F",
    "H",
    "A",
    "T",
    "T_",
    # Functions
    "VacuumAverage",
    "closed_shell_CC_spintrace",
    "open_shell_CC_spintrace",
    # MBPT Context
    "get_csv",
    "set_csv",
]
