#!/usr/bin/env python3
"""
Unit tests for CC (Coupled Cluster) Python bindings in SeQuant.

These tests match the C++ unit tests in tests/unit/test_mbpt_cc.cpp
to ensure the Python bindings produce equivalent results.
"""

import unittest
import _sequant as sq
from _sequant.mbpt import CC, Ansatz, np, nh


class TestMBPT_CC(unittest.TestCase):
    """Test cases matching test_mbpt_cc.cpp"""

    def test_sr_tcc_ccsd_t(self):
        """Test CCSD T amplitude equations (matches C++ SECTION sr_tcc/t/CCSD t)"""
        N = 2
        t_eqs = CC(N).t()

        # Check size of result vector
        self.assertEqual(len(t_eqs), N + 1)

        # Check all equations exist
        for k in range(N + 1):
            self.assertIsNotNone(t_eqs[k], f"t_eqs[{k}] should not be None")

        # Check equation sizes (number of terms)
        if N == 2:
            self.assertEqual(sq.size(t_eqs[0]), 3, "T0 equation should have 3 terms")
            self.assertEqual(sq.size(t_eqs[1]), 14, "T1 equation should have 14 terms")


class TestEOM_CC(unittest.TestCase):
    """Test EOM-CC equations (matches C++ SECTION eom_cc)"""

    def test_ee_eom_ccsd_r(self):
        """Test EE-EOM-CCSD right equations (matches C++ SECTION EE-EOM-CCSD R)"""
        N = 2
        cc = CC(N)

        # EE-EOM-CCSD: 2 particles, 2 holes
        n_p = 2
        n_h = 2
        eqs = cc.eom_r(np(n_p), nh(n_h))

        # Check all equations from k=1 onwards exist
        for k in range(1, len(eqs)):
            self.assertIsNotNone(eqs[k], f"R equation [{k}] should exist")

        # Check equation sizes
        self.assertEqual(sq.size(eqs[1]), 21, "R1 equation should have 21 terms")
        self.assertEqual(sq.size(eqs[2]), 53, "R2 equation should have 53 terms")

    def test_ip_eom_ccsd_r(self):
        """Test IP-EOM-CCSD right equations (matches C++ SECTION IP-EOM-CCSD R)"""
        N = 2
        cc = CC(N)

        # IP-EOM-CCSD: 1 particle, 2 holes
        n_p = 1
        n_h = 2
        eqs = cc.eom_r(np(n_p), nh(n_h))

        # Check all equations exist (including index 0 for IP)
        for k in range(len(eqs)):
            self.assertIsNotNone(eqs[k], f"IP-R equation [{k}] should exist")

        # Check equation sizes
        self.assertEqual(sq.size(eqs[0]), 9, "IP-R0 equation should have 9 terms")
        self.assertEqual(sq.size(eqs[1]), 32, "IP-R1 equation should have 32 terms")

    def test_ea_eom_ccsd_r(self):
        """Test EA-EOM-CCSD right equations (matches C++ SECTION EA-EOM-CCSD R)"""
        N = 2
        cc = CC(N)

        # EA-EOM-CCSD: 2 particles, 1 hole
        n_p = 2
        n_h = 1
        eqs = cc.eom_r(np(n_p), nh(n_h))

        # Check all equations exist (including index 0 for EA)
        for k in range(len(eqs)):
            self.assertIsNotNone(eqs[k], f"EA-R equation [{k}] should exist")

        # Check equation sizes
        self.assertEqual(sq.size(eqs[0]), 9, "EA-R0 equation should have 9 terms")
        self.assertEqual(sq.size(eqs[1]), 32, "EA-R1 equation should have 32 terms")

    def test_ee_eom_ccsd_l(self):
        """Test EE-EOM-CCSD left equations (matches C++ SECTION EE-EOM-CCSD L)"""
        N = 2
        cc = CC(N)

        # EE-EOM-CCSD: 2 particles, 2 holes
        n_p = 2
        n_h = 2
        eqs = cc.eom_l(np(n_p), nh(n_h))

        # Check all equations from k=1 onwards exist
        for k in range(1, len(eqs)):
            self.assertIsNotNone(eqs[k], f"L equation [{k}] should exist")

        # Check equation sizes
        self.assertEqual(sq.size(eqs[1]), 43, "L1 equation should have 43 terms")
        self.assertEqual(sq.size(eqs[2]), 31, "L2 equation should have 31 terms")


class TestEOM_CC_Long(unittest.TestCase):
    """Long tests for EOM-CC (matches C++ SEQUANT_SKIP_LONG_TESTS sections)"""

    def test_ee_eom_ccsdt_r(self):
        """Test EE-EOM-CCSDT right equations (matches C++ SECTION EE-EOM-CCSDT R)"""
        N = 3
        cc = CC(N)

        # EE-EOM-CCSDT: 3 particles, 3 holes
        n_p = 3
        n_h = 3
        eqs = cc.eom_r(np(n_p), nh(n_h))

        # Check all equations from k=1 onwards exist
        for k in range(1, len(eqs)):
            self.assertIsNotNone(eqs[k], f"CCSDT R equation [{k}] should exist")

        # Check equation sizes
        self.assertEqual(sq.size(eqs[1]), 22, "CCSDT R1 should have 22 terms")
        self.assertEqual(sq.size(eqs[2]), 62, "CCSDT R2 should have 62 terms")
        self.assertEqual(sq.size(eqs[3]), 99, "CCSDT R3 should have 99 terms")


class TestUCC_Long(unittest.TestCase):
    """Long tests for Unitary CC (matches C++ ucc section)"""

    def test_ucc_t(self):
        """Test Unitary CC T equations (matches C++ SECTION ucc/t)"""
        N = 2
        C = 3  # commutator rank
        ansatz = Ansatz.U

        # Unitary CC energy, up to third commutator
        t_eqs = CC(N, ansatz).t(C)

        # Check size of result vector
        self.assertEqual(len(t_eqs), N + 1)

        # Check all equations exist
        for k in range(N + 1):
            self.assertIsNotNone(t_eqs[k], f"UCC t_eqs[{k}] should exist")

        # Check equation size for specific case
        if C == 3 and ansatz == Ansatz.U:
            self.assertEqual(sq.size(t_eqs[0]), 56, "UCC T0 with C=3 should have 56 terms")


class TestCCBasic(unittest.TestCase):
    """Additional basic tests for CC class"""

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

    def test_ansatz_enum(self):
        """Test Ansatz enum values"""
        ansatzes = [Ansatz.T, Ansatz.oT, Ansatz.U, Ansatz.oU]
        for ansatz in ansatzes:
            cc = CC(2, ansatz)
            self.assertEqual(cc.ansatz(), ansatz)

    def test_strong_types(self):
        """Test np and nh strong types"""
        n_particles = np(2)
        n_holes = nh(2)

        # Test conversion to int
        self.assertEqual(int(n_particles), 2)
        self.assertEqual(int(n_holes), 2)

        # Test repr
        self.assertEqual(repr(n_particles), "np(2)")
        self.assertEqual(repr(n_holes), "nh(2)")

    def test_lambda_equations(self):
        """Test lambda amplitude equations"""
        cc = CC(2)
        l_eqs = cc.lambda_()

        self.assertIsNotNone(l_eqs)
        self.assertGreater(len(l_eqs), 0)
        self.assertIsNotNone(l_eqs[1])  # λ1
        self.assertIsNotNone(l_eqs[2])  # λ2

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

    def test_size_function(self):
        """Test that the size() function works"""
        cc = CC(2)
        t_eqs = cc.t()

        # size() should return an integer
        size_t0 = sq.size(t_eqs[0])
        size_t1 = sq.size(t_eqs[1])

        self.assertIsInstance(size_t0, int)
        self.assertIsInstance(size_t1, int)
        self.assertGreater(size_t0, 0)
        self.assertGreater(size_t1, 0)


def print_test_summary():
    """Print a summary of what tests are available"""
    print("\n" + "="*70)
    print("CC Python Bindings Unit Tests")
    print("="*70)
    print("\nThese tests match the C++ unit tests in tests/unit/test_mbpt_cc.cpp")
    print("\nTest Classes:")
    print("  1. TestMBPT_CC - CCSD T amplitude equations")
    print("  2. TestEOM_CC - EOM-CCSD equations (EE, IP, EA)")
    print("  3. TestEOM_CC_Long - EOM-CCSDT equations (may be slow)")
    print("  4. TestUCC_Long - Unitary CC equations (may be slow)")
    print("  5. TestCCBasic - Additional basic functionality tests")
    print("\nTo run specific test classes:")
    print("  python test_cc.py TestMBPT_CC")
    print("  python test_cc.py TestEOM_CC")
    print("\nTo skip long tests:")
    print("  python test_cc.py TestMBPT_CC TestEOM_CC TestCCBasic")
    print("="*70 + "\n")


if __name__ == '__main__':
    import sys

    # If no arguments, show summary
    if len(sys.argv) == 1:
        print_test_summary()

    # Run tests
    unittest.main(verbosity=2)
