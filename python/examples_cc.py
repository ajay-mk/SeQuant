#!/usr/bin/env python3
"""
Examples for using the CC (Coupled Cluster) class in SeQuant Python bindings.

These examples demonstrate how to use the CC class to derive various
coupled cluster equations programmatically.
"""

# Support both old and new import styles
try:
    import sequant as sq
    from sequant.mbpt import CC, Ansatz, np, nh, H, T, A, VacuumAverage
except ImportError:
    # Fall back to direct _sequant import for backward compatibility
    import _sequant as sq
    from _sequant.mbpt import CC, Ansatz, np, nh, H, T, A, VacuumAverage


def example_basic_ccsd():
    """Example 1: Basic CCSD equations"""
    print("\n" + "="*60)
    print("Example 1: Basic CCSD T Amplitude Equations")
    print("="*60)

    # Create CCSD engine (rank 2)
    cc = CC(2)

    # Derive t amplitude equations
    # Returns a vector where element k contains the equation for T_k
    t_eqs = cc.t(commutator_rank=4)

    print(f"\nGenerated {len(t_eqs)} equation slots")
    print(f"\nT1 equation (LaTeX):")
    print(t_eqs[1].latex)
    print(f"\nT2 equation (first 300 chars):")
    print(t_eqs[2].latex[:300] + "...")


def example_ccsd_lambda():
    """Example 2: CCSD Lambda equations"""
    print("\n" + "="*60)
    print("Example 2: CCSD Lambda Amplitude Equations")
    print("="*60)

    cc = CC(2)

    # Derive λ amplitude equations
    l_eqs = cc.lambda_(commutator_rank=4)

    print(f"\nGenerated {len(l_eqs)} lambda equations")
    print(f"\nλ1 equation (LaTeX):")
    print(l_eqs[1].latex)
    print(f"\nλ2 equation (first 300 chars):")
    print(l_eqs[2].latex[:300] + "...")


def example_ccsdt():
    """Example 3: CCSDT equations (rank 3)"""
    print("\n" + "="*60)
    print("Example 3: CCSDT T Amplitude Equations (Rank 3)")
    print("="*60)

    # CCSDT: rank 3
    cc = CC(3)

    # This will generate T1, T2, and T3 equations
    t_eqs = cc.t(commutator_rank=4)

    print(f"\nGenerated {len(t_eqs)} equation slots")
    print(f"T1 equation length: {len(t_eqs[1].latex)} chars")
    print(f"T2 equation length: {len(t_eqs[2].latex)} chars")
    print(f"T3 equation length: {len(t_eqs[3].latex)} chars")


def example_unitary_cc():
    """Example 4: Unitary Coupled Cluster (UCC)"""
    print("\n" + "="*60)
    print("Example 4: Unitary Coupled Cluster (UCC)")
    print("="*60)

    # Unitary CC uses exp(T - T†) instead of exp(T)
    ucc = CC(2, Ansatz.U)

    print(f"Is unitary ansatz: {ucc.unitary()}")

    # For unitary CC, higher commutator ranks are often needed
    t_eqs = ucc.t(commutator_rank=6)

    print(f"\nUnitary CCSD T equations with 6th-order commutators")
    print(f"T1 equation length: {len(t_eqs[1].latex)} chars")
    print(f"T2 equation length: {len(t_eqs[2].latex)} chars")


def example_orbital_optimized():
    """Example 5: Orbital-Optimized CC"""
    print("\n" + "="*60)
    print("Example 5: Orbital-Optimized CCSD (OO-CCSD)")
    print("="*60)

    # Orbital-optimized ansatz (singles-free)
    occ = CC(2, Ansatz.oT)

    print(f"Ansatz type: {occ.ansatz()}")

    # OO-CC doesn't have T1 amplitudes
    t_eqs = occ.t()

    print(f"\nGenerated {len(t_eqs)} equation slots")
    print(f"T2 equation (orbital-optimized) length: {len(t_eqs[2].latex)} chars")

    # All four ansatz types
    print("\n\nAll available ansatz types:")
    for ansatz, name in [(Ansatz.T, "Traditional"),
                         (Ansatz.oT, "Orbital-optimized"),
                         (Ansatz.U, "Unitary"),
                         (Ansatz.oU, "Unitary orbital-optimized")]:
        cc_test = CC(2, ansatz)
        print(f"  {name}: unitary={cc_test.unitary()}")


def example_eom_excitation_energy():
    """Example 6: EOM-CCSD for Excitation Energies"""
    print("\n" + "="*60)
    print("Example 6: EE-EOM-CCSD (Excitation Energy)")
    print("="*60)

    cc = CC(2)

    # EOM-CCSD for excitation energies (EE-EOM-CCSD)
    # Right eigenvector: 2 particles, 2 holes
    r_eqs = cc.eom_r(np(2), nh(2))

    print(f"\nGenerated {len(r_eqs)} right-side sigma equations")
    print(f"R1 equation length: {len(r_eqs[1].latex)} chars")
    print(f"R2 equation length: {len(r_eqs[2].latex)} chars")

    # Left eigenvector
    l_eqs = cc.eom_l(np(2), nh(2))

    print(f"\nGenerated {len(l_eqs)} left-side sigma equations")
    print(f"L1 equation length: {len(l_eqs[1].latex)} chars")
    print(f"L2 equation length: {len(l_eqs[2].latex)} chars")


def example_eom_ionization():
    """Example 7: IP-EOM-CCSD and EA-EOM-CCSD"""
    print("\n" + "="*60)
    print("Example 7: IP-EOM and EA-EOM-CCSD")
    print("="*60)

    cc = CC(2)

    # Ionization Potential (IP-EOM-CCSD): 0 particles, 1 hole
    print("\nIP-EOM-CCSD (Ionization Potential):")
    ip_eqs = cc.eom_r(np(0), nh(1))
    print(f"  Generated {len(ip_eqs)} equations")
    print(f"  R(0h,1p) equation length: {len(ip_eqs[1].latex)} chars")

    # Electron Attachment (EA-EOM-CCSD): 1 particle, 0 holes
    print("\nEA-EOM-CCSD (Electron Attachment):")
    ea_eqs = cc.eom_r(np(1), nh(0))
    print(f"  Generated {len(ea_eqs)} equations")
    print(f"  R(1h,0p) equation length: {len(ea_eqs[1].latex)} chars")

    # Double Ionization Potential (DIP-EOM-CCSD): 0 particles, 2 holes
    print("\nDIP-EOM-CCSD (Double Ionization Potential):")
    dip_eqs = cc.eom_r(np(0), nh(2))
    print(f"  Generated {len(dip_eqs)} equations")


def example_perturbation_theory():
    """Example 8: Perturbed amplitude equations"""
    print("\n" + "="*60)
    print("Example 8: First-Order Perturbed Amplitudes")
    print("="*60)

    cc = CC(2)

    # First-order perturbed t amplitudes
    # (useful for response properties)
    t_pt = cc.t_pt(rank=1, order=1)

    print(f"\nPerturbed T amplitudes:")
    print(f"  Generated {len(t_pt)} equations")
    print(f"  T1(1) equation length: {len(t_pt[1].latex)} chars")
    print(f"  T2(1) equation length: {len(t_pt[2].latex)} chars")

    # First-order perturbed λ amplitudes
    l_pt = cc.lambda_pt(rank=1, order=1)

    print(f"\nPerturbed λ amplitudes:")
    print(f"  Generated {len(l_pt)} equations")
    print(f"  λ1(1) equation length: {len(l_pt[1].latex)} chars")
    print(f"  λ2(1) equation length: {len(l_pt[2].latex)} chars")


def example_different_settings():
    """Example 9: CC with different settings"""
    print("\n" + "="*60)
    print("Example 9: CC Configuration Options")
    print("="*60)

    # Default: screening and topology optimization enabled
    cc_default = CC(2)
    print(f"Default settings:")
    print(f"  Screening: {cc_default.screen()}")
    print(f"  Topology optimization: {cc_default.use_topology()}")

    # Disable screening
    cc_no_screen = CC(2, screen=False)
    print(f"\nNo screening:")
    print(f"  Screening: {cc_no_screen.screen()}")

    # Disable topology optimization
    cc_no_topo = CC(2, use_topology=False)
    print(f"\nNo topology optimization:")
    print(f"  Topology optimization: {cc_no_topo.use_topology()}")

    # Disable both
    cc_minimal = CC(2, screen=False, use_topology=False)
    print(f"\nMinimal optimizations:")
    print(f"  Screening: {cc_minimal.screen()}")
    print(f"  Topology optimization: {cc_minimal.use_topology()}")


def example_projector_manifold():
    """Example 10: Custom projector manifolds"""
    print("\n" + "="*60)
    print("Example 10: Custom Projector Manifolds")
    print("="*60)

    cc = CC(3)  # CCSDT

    # Generate only T2 and T3 equations (skip T1)
    print("\nGenerate only T2 and T3 (pmin=2, pmax=3):")
    t_eqs_23 = cc.t(pmin=2, pmax=3)
    print(f"  T2 equation exists: {t_eqs_23[2] is not None}")
    print(f"  T3 equation exists: {t_eqs_23[3] is not None}")

    # Generate only T1 and T2 (from CCSDT, skip T3)
    print("\nGenerate only T1 and T2 (pmin=1, pmax=2):")
    t_eqs_12 = cc.t(pmin=1, pmax=2)
    print(f"  T1 equation exists: {t_eqs_12[1] is not None}")
    print(f"  T2 equation exists: {t_eqs_12[2] is not None}")


def example_comparison():
    """Example 11: Compare different CC methods"""
    print("\n" + "="*60)
    print("Example 11: Comparison of CC Methods")
    print("="*60)

    methods = [
        (CC(2, Ansatz.T), "CCSD"),
        (CC(3, Ansatz.T), "CCSDT"),
        (CC(2, Ansatz.U), "UCC-SD"),
        (CC(2, Ansatz.oT), "OO-CCSD"),
    ]

    print("\nEquation sizes (LaTeX string length):")
    print(f"{'Method':<12} {'T1':>8} {'T2':>10} {'T3':>10}")
    print("-" * 44)

    for cc, name in methods:
        t_eqs = cc.t(commutator_rank=4)
        t1_len = len(t_eqs[1].latex) if t_eqs[1] else 0
        t2_len = len(t_eqs[2].latex) if t_eqs[2] else 0
        t3_len = len(t_eqs[3].latex) if len(t_eqs) > 3 and t_eqs[3] else 0

        print(f"{name:<12} {t1_len:>8} {t2_len:>10} {t3_len:>10}")


def main():
    """Run all examples"""
    print("\n" + "="*60)
    print("SeQuant Python CC Class Examples")
    print("="*60)
    print("\nThese examples demonstrate the CC (Coupled Cluster) class")
    print("for deriving various coupled cluster equations.")

    # Run all examples
    example_basic_ccsd()
    example_ccsd_lambda()
    example_ccsdt()
    example_unitary_cc()
    example_orbital_optimized()
    example_eom_excitation_energy()
    example_eom_ionization()
    example_perturbation_theory()
    example_different_settings()
    example_projector_manifold()
    example_comparison()

    print("\n" + "="*60)
    print("Examples completed!")
    print("="*60)


if __name__ == "__main__":
    main()
