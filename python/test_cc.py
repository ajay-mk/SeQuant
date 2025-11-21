#!/usr/bin/env python3
"""
Test script for CC (Coupled Cluster) Python bindings in SeQuant.

This tests the mbpt.CC class and all its methods.
"""

import unittest
import _sequant as sq
from _sequant.mbpt import CC, Ansatz, np, nh


class TestCC(unittest.TestCase):
    """Test cases for the CC class"""

    def test_cc_construction(self):
        """Test CC class construction with different parameters"""
        # Basic CCSD
        cc2 = CC(2)
        self.assertEqual(cc2.ansatz(), Ansatz.T)
        self.assertFalse(cc2.unitary())
        self.assertTrue(cc2.screen())
        self.assertTrue(cc2.use_topology())

        # CCSDT
        cc3 = CC(3)
        self.assertIsNotNone(cc3)

        # Unitary CCSD
        ucc2 = CC(2, Ansatz.U)
        self.assertEqual(ucc2.ansatz(), Ansatz.U)
        self.assertTrue(ucc2.unitary())

        # Orbital-optimized CCSD
        occ2 = CC(2, Ansatz.oT)
        self.assertEqual(occ2.ansatz(), Ansatz.oT)
        self.assertFalse(occ2.unitary())

        # Unitary orbital-optimized CCSD
        oucc2 = CC(2, Ansatz.oU)
        self.assertEqual(oucc2.ansatz(), Ansatz.oU)
        self.assertTrue(oucc2.unitary())

        # CC with custom options
        cc_custom = CC(2, Ansatz.T, screen=False, use_topology=False)
        self.assertFalse(cc_custom.screen())
        self.assertFalse(cc_custom.use_topology())

    def test_ansatz_enum(self):
        """Test Ansatz enum values"""
        # Test all ansatz types
        ansatzes = [Ansatz.T, Ansatz.oT, Ansatz.U, Ansatz.oU]
        for ansatz in ansatzes:
            cc = CC(2, ansatz)
            self.assertEqual(cc.ansatz(), ansatz)

    def test_strong_types(self):
        """Test np and nh strong types"""
        # Create instances
        n_particles = np(2)
        n_holes = nh(2)

        # Test conversion to int
        self.assertEqual(int(n_particles), 2)
        self.assertEqual(int(n_holes), 2)

        # Test repr
        self.assertEqual(repr(n_particles), "np(2)")
        self.assertEqual(repr(n_holes), "nh(2)")

    def test_ccsd_t_equations(self):
        """Test CCSD t amplitude equations generation"""
        cc = CC(2)
        t_eqs = cc.t()

        # Should return a list/vector
        self.assertIsNotNone(t_eqs)
        self.assertGreater(len(t_eqs), 0)

        # Check that we have equations
        # Element 0 is null, elements 1 and 2 contain T1 and T2 equations
        self.assertIsNotNone(t_eqs[1])  # T1 equation
        self.assertIsNotNone(t_eqs[2])  # T2 equation

        # Check LaTeX output works
        latex1 = t_eqs[1].latex
        latex2 = t_eqs[2].latex
        self.assertIsInstance(latex1, str)
        self.assertIsInstance(latex2, str)
        self.assertGreater(len(latex1), 0)
        self.assertGreater(len(latex2), 0)

    def test_ccsd_lambda_equations(self):
        """Test CCSD λ amplitude equations generation"""
        cc = CC(2)
        l_eqs = cc.lambda_()

        # Should return a list/vector
        self.assertIsNotNone(l_eqs)
        self.assertGreater(len(l_eqs), 0)

        # Check that we have equations
        self.assertIsNotNone(l_eqs[1])  # λ1 equation
        self.assertIsNotNone(l_eqs[2])  # λ2 equation

        # Check LaTeX output works
        latex1 = l_eqs[1].latex
        latex2 = l_eqs[2].latex
        self.assertIsInstance(latex1, str)
        self.assertIsInstance(latex2, str)

    def test_ccsdt_equations(self):
        """Test CCSDT (rank 3) equations"""
        cc = CC(3)
        t_eqs = cc.t()

        # Should have T1, T2, and T3 equations
        self.assertIsNotNone(t_eqs)
        self.assertGreaterEqual(len(t_eqs), 4)  # 0, 1, 2, 3
        self.assertIsNotNone(t_eqs[1])  # T1
        self.assertIsNotNone(t_eqs[2])  # T2
        self.assertIsNotNone(t_eqs[3])  # T3

    def test_unitary_cc(self):
        """Test unitary CC equations"""
        ucc = CC(2, Ansatz.U)
        self.assertTrue(ucc.unitary())

        # Generate equations with 4th order commutator expansion
        t_eqs = ucc.t(commutator_rank=4)
        self.assertIsNotNone(t_eqs)
        self.assertGreater(len(t_eqs), 0)

    def test_orbital_optimized_cc(self):
        """Test orbital-optimized CC (singles-free)"""
        occ = CC(2, Ansatz.oT)

        # oT should not have T1 equations
        t_eqs = occ.t()
        self.assertIsNotNone(t_eqs)
        # T2 equation should still exist
        self.assertIsNotNone(t_eqs[2])

    def test_eom_ee_ccsd(self):
        """Test EE-EOM-CCSD (Excitation Energy)"""
        cc = CC(2)

        # EE-EOM-CCSD: 2 particles, 2 holes
        r_eqs = cc.eom_r(np(2), nh(2))
        self.assertIsNotNone(r_eqs)
        self.assertGreater(len(r_eqs), 0)

        # Check R1 and R2 equations
        self.assertIsNotNone(r_eqs[1])  # R1
        self.assertIsNotNone(r_eqs[2])  # R2

        # Test left equations too
        l_eqs = cc.eom_l(np(2), nh(2))
        self.assertIsNotNone(l_eqs)
        self.assertIsNotNone(l_eqs[1])  # L1
        self.assertIsNotNone(l_eqs[2])  # L2

    def test_eom_ip_ccsd(self):
        """Test IP-EOM-CCSD (Ionization Potential)"""
        cc = CC(2)

        # IP-EOM-CCSD: 0 particles, 1 hole
        r_eqs = cc.eom_r(np(0), nh(1))
        self.assertIsNotNone(r_eqs)
        self.assertGreater(len(r_eqs), 0)

    def test_eom_ea_ccsd(self):
        """Test EA-EOM-CCSD (Electron Attachment)"""
        cc = CC(2)

        # EA-EOM-CCSD: 1 particle, 0 holes
        r_eqs = cc.eom_r(np(1), nh(0))
        self.assertIsNotNone(r_eqs)
        self.assertGreater(len(r_eqs), 0)

    def test_perturbation_theory(self):
        """Test perturbed amplitude equations"""
        cc = CC(2)

        # First-order perturbed t amplitudes
        t_pt = cc.t_pt(rank=1, order=1)
        self.assertIsNotNone(t_pt)
        self.assertGreater(len(t_pt), 0)

        # First-order perturbed λ amplitudes
        l_pt = cc.lambda_pt(rank=1, order=1)
        self.assertIsNotNone(l_pt)
        self.assertGreater(len(l_pt), 0)

    def test_commutator_rank(self):
        """Test different commutator ranks"""
        cc = CC(2)

        # Test with different commutator ranks
        for rank in [2, 4, 6]:
            t_eqs = cc.t(commutator_rank=rank)
            self.assertIsNotNone(t_eqs)
            self.assertGreater(len(t_eqs), 0)

    def test_projector_manifold(self):
        """Test pmin and pmax parameters"""
        cc = CC(2)

        # Generate only T2 equation (pmin=2, pmax=2)
        t_eqs = cc.t(pmin=2, pmax=2)
        self.assertIsNotNone(t_eqs)
        self.assertIsNotNone(t_eqs[2])  # T2 should exist

    def test_integration_example(self):
        """Integration test: Complete CCSD workflow"""
        # Create CCSD engine
        cc = CC(2, Ansatz.T, screen=True, use_topology=True)

        # Generate t equations
        t_eqs = cc.t(commutator_rank=4)
        self.assertIsNotNone(t_eqs[1])
        self.assertIsNotNone(t_eqs[2])

        # Generate λ equations
        l_eqs = cc.lambda_(commutator_rank=4)
        self.assertIsNotNone(l_eqs[1])
        self.assertIsNotNone(l_eqs[2])

        # Generate EOM-CCSD equations
        r_eqs = cc.eom_r(np(2), nh(2))
        l_eqs_eom = cc.eom_l(np(2), nh(2))
        self.assertIsNotNone(r_eqs[1])
        self.assertIsNotNone(l_eqs_eom[1])

        # Print some output (for visual verification)
        print("\n=== CCSD T1 Equation (LaTeX) ===")
        print(t_eqs[1].latex)

        print("\n=== CCSD T2 Equation (first 200 chars) ===")
        print(t_eqs[2].latex[:200] + "...")

    def test_all_ansatz_types(self):
        """Test all four ansatz types can generate equations"""
        for ansatz in [Ansatz.T, Ansatz.oT, Ansatz.U, Ansatz.oU]:
            cc = CC(2, ansatz)
            t_eqs = cc.t(commutator_rank=4)
            self.assertIsNotNone(t_eqs)

            # For non-orbital-optimized, check T1
            if ansatz in [Ansatz.T, Ansatz.U]:
                self.assertIsNotNone(t_eqs[1])

            # All should have T2
            self.assertIsNotNone(t_eqs[2])


def print_examples():
    """Print example usage of the CC class"""
    print("\n" + "=" * 60)
    print("CC Class Examples")
    print("=" * 60)

    # Example 1: Basic CCSD
    print("\n1. Basic CCSD")
    cc = CC(2)
    t_eqs = cc.t()
    print(f"   Generated {len(t_eqs)} equations")
    print(f"   T1 equation length: {len(t_eqs[1].latex)} chars")
    print(f"   T2 equation length: {len(t_eqs[2].latex)} chars")

    # Example 2: Unitary CCSD
    print("\n2. Unitary CCSD (UCC)")
    ucc = CC(2, Ansatz.U)
    ut_eqs = ucc.t(commutator_rank=6)
    print(f"   Unitary: {ucc.unitary()}")
    print(f"   Generated {len(ut_eqs)} equations with rank-6 commutators")

    # Example 3: EOM-CCSD
    print("\n3. EOM-CCSD (Excitation Energies)")
    r_eqs = cc.eom_r(np(2), nh(2))
    print(f"   Generated {len(r_eqs)} right-side equations")

    # Example 4: Different CC ranks
    print("\n4. Different CC ranks")
    for N in [2, 3]:
        cc_n = CC(N)
        t_eqs_n = cc_n.t()
        print(f"   CC rank {N}: {len(t_eqs_n)} equations")

    print("\n" + "=" * 60)


if __name__ == '__main__':
    # Run examples first
    print_examples()

    # Then run tests
    print("\n\nRunning unit tests...")
    unittest.main(verbosity=2)
