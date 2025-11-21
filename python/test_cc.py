#!/usr/bin/env python3
"""
Unit tests for CC (Coupled Cluster) Python bindings.
Matches C++ tests in tests/unit/test_mbpt_cc.cpp
"""

import unittest
from _sequant.mbpt import CC, Ansatz, np, nh


class TestCCSD(unittest.TestCase):
    """CCSD equations (C++ sr_tcc/t/CCSD t)"""

    def test_ccsd_t(self):
        """CCSD T amplitude equations"""
        N = 2
        t_eqs = CC(N).t()

        self.assertEqual(len(t_eqs), N + 1)
        for k in range(N + 1):
            self.assertIsNotNone(t_eqs[k])

        self.assertEqual(t_eqs[0].size(), 3)
        self.assertEqual(t_eqs[1].size(), 14)


class TestEOMCCSD(unittest.TestCase):
    """EOM-CCSD equations (C++ eom_cc/EOM-CCSD)"""

    def setUp(self):
        self.cc = CC(2)

    def test_ee_eom_r(self):
        """EE-EOM-CCSD right equations"""
        eqs = self.cc.eom_r(np(2), nh(2))
        for k in range(1, len(eqs)):
            self.assertIsNotNone(eqs[k])
        self.assertEqual(eqs[1].size(), 21)
        self.assertEqual(eqs[2].size(), 53)

    def test_ip_eom_r(self):
        """IP-EOM-CCSD right equations"""
        eqs = self.cc.eom_r(np(1), nh(2))
        for k in range(len(eqs)):
            self.assertIsNotNone(eqs[k])
        self.assertEqual(eqs[0].size(), 9)
        self.assertEqual(eqs[1].size(), 32)

    def test_ea_eom_r(self):
        """EA-EOM-CCSD right equations"""
        eqs = self.cc.eom_r(np(2), nh(1))
        for k in range(len(eqs)):
            self.assertIsNotNone(eqs[k])
        self.assertEqual(eqs[0].size(), 9)
        self.assertEqual(eqs[1].size(), 32)

    def test_ee_eom_l(self):
        """EE-EOM-CCSD left equations"""
        eqs = self.cc.eom_l(np(2), nh(2))
        for k in range(1, len(eqs)):
            self.assertIsNotNone(eqs[k])
        self.assertEqual(eqs[1].size(), 43)
        self.assertEqual(eqs[2].size(), 31)


class TestEOMCCSDT(unittest.TestCase):
    """EOM-CCSDT (C++ EOM-CCSDT, long test)"""

    def test_ee_eom_r(self):
        """EE-EOM-CCSDT right equations"""
        eqs = CC(3).eom_r(np(3), nh(3))
        for k in range(1, len(eqs)):
            self.assertIsNotNone(eqs[k])
        self.assertEqual(eqs[1].size(), 22)
        self.assertEqual(eqs[2].size(), 62)
        self.assertEqual(eqs[3].size(), 99)


class TestUCC(unittest.TestCase):
    """Unitary CC (C++ ucc section, long test)"""

    def test_ucc_t(self):
        """UCC T equations with commutator rank 3"""
        t_eqs = CC(2, Ansatz.U).t(3)
        self.assertEqual(len(t_eqs), 3)
        for k in range(3):
            self.assertIsNotNone(t_eqs[k])
        self.assertEqual(t_eqs[0].size(), 56)


if __name__ == '__main__':
    unittest.main(verbosity=2)
