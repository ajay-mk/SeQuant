#ifndef SEQUANT_PYTHON_MBPT_H
#define SEQUANT_PYTHON_MBPT_H

#include <SeQuant/domain/mbpt/convention.hpp>
#include <SeQuant/domain/mbpt/models/cc.hpp>
#include <SeQuant/domain/mbpt/op.hpp>

#include <SeQuant/core/expr.hpp>
#include <SeQuant/core/tensor_canonicalizer.hpp>

#include "python.h"

#include <cstdint>

namespace sequant::python::mbpt {

template <class F>
auto make_sr_op(F f) {
  auto op = [f](std::int64_t Rank) { return f(Rank); };
  return op;
}

template <class... Args>
ExprPtr VacuumAverage(const ExprPtr& e, const Args&... args) {
  return sequant::mbpt::op::vac_av(e, args...);
}

#define SR_OP(OP) \
  #OP, [](std::int64_t Rank) { return sequant::mbpt::OP(Rank); }, py::arg("Bra")

inline void __init__(py::module m) {
  sequant::mbpt::load(sequant::mbpt::Convention::Minimal);
  sequant::TensorCanonicalizer::register_instance(
      std::make_shared<DefaultTensorCanonicalizer>());

  py::enum_<sequant::mbpt::OpType>(m, "OpType")
      .value("h", sequant::mbpt::OpType::h)
      .value("f", sequant::mbpt::OpType::f)
      .value("t", sequant::mbpt::OpType::t)
      .export_values();

  m.def("F", &sequant::mbpt::F);
  m.def("H", &sequant::mbpt::H,
        "H(k = 2) returns a Hamiltonian operator with up to k-body terms",
        py::arg("k") = 2);

  m.def(SR_OP(A));
  m.def(SR_OP(T));
  m.def(SR_OP(T_));

  m.def("VacuumAverage", &VacuumAverage<>);
  m.def("VacuumAverage",
        &VacuumAverage<std::vector<
            std::pair<sequant::mbpt::OpType, sequant::mbpt::OpType> > >);

  // Strong types for particle and hole counts
  py::class_<sequant::mbpt::nₚ>(m, "np")
      .def(py::init<std::int64_t>())
      .def("__int__", [](const sequant::mbpt::nₚ& n) { return static_cast<std::int64_t>(n); })
      .def("__repr__", [](const sequant::mbpt::nₚ& n) {
        return "np(" + std::to_string(static_cast<std::int64_t>(n)) + ")";
      });

  py::class_<sequant::mbpt::nₕ>(m, "nh")
      .def(py::init<std::int64_t>())
      .def("__int__", [](const sequant::mbpt::nₕ& n) { return static_cast<std::int64_t>(n); })
      .def("__repr__", [](const sequant::mbpt::nₕ& n) {
        return "nh(" + std::to_string(static_cast<std::int64_t>(n)) + ")";
      });

  // CC Ansatz enum
  py::enum_<sequant::mbpt::CC::Ansatz>(m, "Ansatz")
      .value("T", sequant::mbpt::CC::Ansatz::T, "Traditional ansatz")
      .value("oT", sequant::mbpt::CC::Ansatz::oT, "Traditional orbital-optimized (singles-free) ansatz")
      .value("U", sequant::mbpt::CC::Ansatz::U, "Unitary ansatz")
      .value("oU", sequant::mbpt::CC::Ansatz::oU, "Unitary orbital-optimized (singles-free) ansatz")
      .export_values();

  // CC class
  py::class_<sequant::mbpt::CC>(m, "CC")
      .def(py::init<size_t, sequant::mbpt::CC::Ansatz, bool, bool>(),
           py::arg("N"),
           py::arg("ansatz") = sequant::mbpt::CC::Ansatz::T,
           py::arg("screen") = true,
           py::arg("use_topology") = true,
           "Constructs CC engine\n\n"
           "Parameters\n"
           "----------\n"
           "N : int\n"
           "    Coupled cluster excitation rank (e.g., 2 for CCSD, 3 for CCSDT)\n"
           "ansatz : Ansatz, optional\n"
           "    Type of CC ansatz (T, oT, U, oU), default is T\n"
           "screen : bool, optional\n"
           "    If True, uses operator level screening before applying Wick's theorem, default is True\n"
           "use_topology : bool, optional\n"
           "    If True, uses topological optimizations in Wick's theorem, default is True")
      .def("ansatz", &sequant::mbpt::CC::ansatz,
           "Returns the type of ansatz")
      .def("unitary", &sequant::mbpt::CC::unitary,
           "Returns True if the ansatz is unitary (U or oU)")
      .def("screen", &sequant::mbpt::CC::screen,
           "Returns whether screening is enabled")
      .def("use_topology", &sequant::mbpt::CC::use_topology,
           "Returns whether topological optimization is used")
      .def("t", &sequant::mbpt::CC::t,
           py::arg("commutator_rank") = 4,
           py::arg("pmax") = std::numeric_limits<size_t>::max(),
           py::arg("pmin") = 0,
           "Derives t amplitude equations: <P|H̄|0> = 0\n\n"
           "Parameters\n"
           "----------\n"
           "commutator_rank : int, optional\n"
           "    Rank of commutators included in H̄, default is 4\n"
           "pmax : int, optional\n"
           "    Highest particle rank of the projector manifold <P|, default is cluster rank\n"
           "pmin : int, optional\n"
           "    Lowest particle rank of the projector manifold <P|, default is 0\n\n"
           "Returns\n"
           "-------\n"
           "list of ExprPtr\n"
           "    Vector of t amplitude equations, element k contains equation for rank k")
      .def("lambda_", &sequant::mbpt::CC::λ,
           py::arg("commutator_rank") = 4,
           "Derives λ amplitude equations: <0|(1+Λ̂) dH̄/dT̂_P|0> = 0\n\n"
           "Parameters\n"
           "----------\n"
           "commutator_rank : int, optional\n"
           "    Rank of commutators included in H̄, default is 4\n\n"
           "Returns\n"
           "-------\n"
           "list of ExprPtr\n"
           "    Vector of λ amplitude equations, element k contains equation for rank k")
      .def("t_pt", &sequant::mbpt::CC::t_pt,
           py::arg("rank") = 1,
           py::arg("order") = 1,
           py::arg("nbatch") = std::nullopt,
           "Derives perturbed t amplitude equations\n\n"
           "Parameters\n"
           "----------\n"
           "rank : int, optional\n"
           "    Rank of perturbation operator (1 = one-body), default is 1\n"
           "order : int, optional\n"
           "    Order of perturbation, default is 1\n"
           "nbatch : int or None, optional\n"
           "    Optional batching index rank for perturbation operators\n\n"
           "Returns\n"
           "-------\n"
           "list of ExprPtr\n"
           "    Vector of perturbed t amplitude equations")
      .def("lambda_pt", &sequant::mbpt::CC::λ_pt,
           py::arg("rank") = 1,
           py::arg("order") = 1,
           py::arg("nbatch") = std::nullopt,
           "Derives perturbed λ amplitude equations\n\n"
           "Parameters\n"
           "----------\n"
           "rank : int, optional\n"
           "    Rank of perturbation operator (1 = one-body), default is 1\n"
           "order : int, optional\n"
           "    Order of perturbation, default is 1\n"
           "nbatch : int or None, optional\n"
           "    Optional batching index rank for perturbation operators\n\n"
           "Returns\n"
           "-------\n"
           "list of ExprPtr\n"
           "    Vector of perturbed λ amplitude equations")
      .def("eom_r", &sequant::mbpt::CC::eom_r,
           py::arg("np"),
           py::arg("nh"),
           "Derives right-side sigma equations for EOM-CC\n\n"
           "Parameters\n"
           "----------\n"
           "np : np\n"
           "    Number of particle creators in R operator\n"
           "nh : nh\n"
           "    Number of hole creators in R operator\n\n"
           "Returns\n"
           "-------\n"
           "list of ExprPtr\n"
           "    Vector of right-side sigma equations\n\n"
           "Examples\n"
           "--------\n"
           "EE-EOM-CCSD: eom_r(np(2), nh(2))\n"
           "IP-EOM-CCSD: eom_r(np(0), nh(1))\n"
           "EA-EOM-CCSD: eom_r(np(1), nh(0))")
      .def("eom_l", &sequant::mbpt::CC::eom_l,
           py::arg("np"),
           py::arg("nh"),
           "Derives left-side sigma equations for EOM-CC\n\n"
           "Parameters\n"
           "----------\n"
           "np : np\n"
           "    Number of particle annihilators in L operator\n"
           "nh : nh\n"
           "    Number of hole annihilators in L operator\n\n"
           "Returns\n"
           "-------\n"
           "list of ExprPtr\n"
           "    Vector of left-side sigma equations");
}

}  // namespace sequant::python::mbpt

#endif /* SEQUANT_PYTHON_MBPT_H */
