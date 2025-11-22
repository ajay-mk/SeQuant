#include <SeQuant/core/attr.hpp>
#include <SeQuant/core/complex.hpp>
#include <SeQuant/core/context.hpp>
#include <SeQuant/core/expressions/expr_algorithms.hpp>
#include <SeQuant/core/expressions/tensor.hpp>
#include <SeQuant/core/index.hpp>
#include <SeQuant/core/rational.hpp>
#include <SeQuant/core/space.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "mbpt.h"

namespace py = pybind11;

namespace sequant::python {

inline std::vector<Index> make_index(std::vector<std::wstring> labels) {
  std::vector<Index> index;
  for (auto l : labels) {
    index.push_back(Index{l});
  }
  return index;
}

std::shared_ptr<Tensor> make_tensor(std::wstring label,
                                    std::vector<std::wstring> b,
                                    std::vector<std::wstring> k,
                                    std::vector<std::wstring> a) {
  return std::make_shared<Tensor>(label, bra(make_index(b)), ket(make_index(k)),
                                  aux(make_index(a)));
}

std::shared_ptr<Constant> make_constant(py::float_ number) {
  return std::make_shared<Constant>(to_rational(number.cast<double>()));
}

inline ExprPtr pow(const ExprPtr &b, int n) {
  ExprPtr e = b->clone();
  // auto p = *e;
  for (int i = 1; i < n; ++i) {
    e = e * b;
  }
  return e;
}

py::object summands(ExprPtr &expr) {
  if (auto s = std::dynamic_pointer_cast<Sum>(expr)) {
    return py::cast(s->summands());
  }
  return py::none();
}

py::object factors(ExprPtr &expr) {
  if (auto p = std::dynamic_pointer_cast<Product>(expr)) {
    return py::cast(p->factors());
  }
  return py::none();
}

// disambiguates sequant::simplify
ExprPtr &simplify(ExprPtr &expr) { return sequant::simplify(expr); }

// Wrapper for canonicalize that returns a new ExprPtr
ExprPtr canonicalize(const ExprPtr &expr) {
  ExprPtr result = expr->clone();
  sequant::canonicalize(result);
  return result;
}

// Wrapper for expand that returns a new ExprPtr
ExprPtr expand_expr(const ExprPtr &expr) {
  ExprPtr result = expr->clone();
  sequant::expand(result);
  return result;
}

// Wrapper for flatten that returns a new ExprPtr
ExprPtr flatten_expr(const ExprPtr &expr) {
  ExprPtr result = expr->clone();
  sequant::flatten(result);
  return result;
}

// Wrapper for rapid_simplify that returns a new ExprPtr
ExprPtr rapid_simplify_expr(const ExprPtr &expr) {
  ExprPtr result = expr->clone();
  sequant::rapid_simplify(result);
  return result;
}

py::object rational_to_fraction(const rational &r) {
  py::object Fraction = py::module::import("fractions").attr("Fraction");
  return Fraction(numerator(r), denominator(r));
}

std::string complex_to_string(const Complex<rational> &z) {
  return z.imag() != 0 ? (to_string(z.real()) + "1j * " + to_string(z.imag()))
                       : to_string(z.real());
}

}  // namespace sequant::python

PYBIND11_NAMESPACE_BEGIN(PYBIND11_NAMESPACE)
PYBIND11_NAMESPACE_BEGIN(detail)

template <>
struct is_holder_type<sequant::Expr, sequant::ExprPtr> : std::true_type {};

template <>
struct always_construct_holder<sequant::ExprPtr>
    : always_construct_holder<void> {};
template <>
class type_caster<sequant::ExprPtr>
    : public type_caster_holder<sequant::Expr, sequant::ExprPtr> {};

PYBIND11_NAMESPACE_END(detail)
PYBIND11_NAMESPACE_END(PYBIND11_NAMESPACE)

PYBIND11_MODULE(_sequant, m) {
  using namespace sequant;
  using namespace sequant::python;

  // Context enums
  py::enum_<Vacuum>(m, "Vacuum")
      .value("Physical", Vacuum::Physical,
             "Physical vacuum (all orbitals unoccupied)")
      .value("SingleProduct", Vacuum::SingleProduct,
             "Single-product (Fermi) vacuum")
      .value("MultiProduct", Vacuum::MultiProduct, "Multi-product vacuum");

  py::enum_<SPBasis>(m, "SPBasis")
      .value("Spinor", SPBasis::Spinor, "Spin-orbital basis")
      .value("Spinfree", SPBasis::Spinfree, "Spin-free basis");

  // Context management functions
  m.def(
      "get_default_context",
      []() -> const Context & { return get_default_context(); },
      py::return_value_policy::reference,
      "Get the default context\n\n"
      "Returns\n"
      "-------\n"
      "Context\n"
      "    The current default context");

  m.def(
      "set_vacuum",
      [](Vacuum v) {
        auto ctx = get_default_context();
        ctx.set(v);
        set_default_context(ctx);
      },
      py::arg("vacuum"),
      "Set the vacuum type for the default context\n\n"
      "Parameters\n"
      "----------\n"
      "vacuum : Vacuum\n"
      "    Vacuum type (Physical, SingleProduct, or MultiProduct)\n\n"
      "Examples\n"
      "--------\n"
      ">>> import sequant as sq\n"
      ">>> sq.set_vacuum(sq.Vacuum.SingleProduct)  # Use Fermi vacuum");

  m.def(
      "set_spbasis",
      [](SPBasis basis) {
        auto ctx = get_default_context();
        ctx.set(basis);
        set_default_context(ctx);
      },
      py::arg("spbasis"),
      "Set the single-particle basis for the default context\n\n"
      "Parameters\n"
      "----------\n"
      "spbasis : SPBasis\n"
      "    Basis type (Spinor or Spinfree)\n\n"
      "Examples\n"
      "--------\n"
      ">>> import sequant as sq\n"
      ">>> sq.set_spbasis(sq.SPBasis.Spinfree)  # Use spin-free basis");

  m.def(
      "get_vacuum",
      []() { return get_default_context().vacuum(); },
      "Get the current vacuum type\n\n"
      "Returns\n"
      "-------\n"
      "Vacuum\n"
      "    Current vacuum type");

  m.def(
      "get_spbasis",
      []() { return get_default_context().spbasis(); },
      "Get the current single-particle basis\n\n"
      "Returns\n"
      "-------\n"
      "SPBasis\n"
      "    Current basis type");

  m.def(
      "reset_default_context",
      []() {
        set_default_context(Context{});
      },
      "Reset the default context to initial defaults");

#define SEQUANT_PYTHON_INDEXSPACE_TYPE_PROPERTY(TYPE, LABEL)                  \
  .def_property_static(                                                       \
      #TYPE,                                                                  \
      [](py::object /* self */) {                                             \
        return get_default_context().index_space_registry()->retrieve(LABEL); \
      },                                                                      \
      [](py::object /* self */) {})

  py::class_<IndexSpace>(m, "IndexSpace")
      SEQUANT_PYTHON_INDEXSPACE_TYPE_PROPERTY(occupied, "i")
          SEQUANT_PYTHON_INDEXSPACE_TYPE_PROPERTY(unoccupied, "a")
              SEQUANT_PYTHON_INDEXSPACE_TYPE_PROPERTY(particle, "i")
                  SEQUANT_PYTHON_INDEXSPACE_TYPE_PROPERTY(hole, "a");

  py::class_<ExprPtr>(m, "ExprPtr")
      .def_property_readonly("latex", &ExprPtr::to_latex)
      .def("size",
           static_cast<std::size_t (*)(const ExprPtr &)>(&sequant::size),
           "Returns the number of subexpressions (0 for atoms, >0 for "
           "Sum/Product)")
      .def("_repr_latex_",
           [](const ExprPtr &expr) {
             return "$$" + expr.to_latex() + "$$";
           },
           "Jupyter notebook LaTeX representation")
      .def("_repr_html_",
           [](const ExprPtr &expr) {
             return "<div style='font-size: 14px;'>$$" + expr.to_latex() +
                    "$$</div>";
           },
           "Jupyter notebook HTML representation")
      .def("__repr__",
           [](const ExprPtr &expr) {
             std::string latex = expr.to_latex();
             if (latex.length() > 100) {
               return latex.substr(0, 97) + "...";
             }
             return latex;
           },
           "String representation")
      .def("__add__", [](const ExprPtr &l, const ExprPtr &r) { return l + r; })
      .def("__sub__", [](const ExprPtr &l, const ExprPtr &r) { return l - r; })
      .def("__mul__", [](const ExprPtr &l, const ExprPtr &r) { return l * r; });

  py::class_<Expr, ExprPtr>(m, "Expr")
      .def_property_readonly("summands", &summands)
      .def_property_readonly("factors", &factors)
      .def_property_readonly("latex", &Expr::to_latex)
      .def("size", static_cast<std::size_t (*)(const Expr &)>(&sequant::size),
           "Returns the number of subexpressions (0 for atoms, >0 for "
           "Sum/Product)")
      .def("_repr_latex_",
           [](const Expr &expr) {
             return "$$" + expr.to_latex() + "$$";
           },
           "Jupyter notebook LaTeX representation")
      .def("_repr_html_",
           [](const Expr &expr) {
             return "<div style='font-size: 14px;'>$$" + expr.to_latex() +
                    "$$</div>";
           },
           "Jupyter notebook HTML representation")
      .def("__repr__",
           [](const Expr &expr) {
             std::string latex = expr.to_latex();
             if (latex.length() > 100) {
               return latex.substr(0, 97) + "...";
             }
             return latex;
           },
           "String representation")
      .def("__add__", [](const ExprPtr &l, const ExprPtr &r) { return l + r; })
      .def("__sub__", [](const ExprPtr &l, const ExprPtr &r) { return l - r; })
      .def("__mul__", [](const ExprPtr &l, const ExprPtr &r) { return l * r; })
      .def("__pow__", [](const ExprPtr &b, int n) { return pow(b, n); });

  py::class_<Index, std::shared_ptr<Index>>(m, "Index")
      .def("__str__", &Index::label)
      .def("__repr__", &Index::label)
      .def_property_readonly("space", &Index::space);

  py::class_<Tensor, std::shared_ptr<Tensor>, Expr>(m, "Tensor")
      .def(py::init(&python::make_tensor))
      .def_property_readonly("label", &Tensor::label)
      .def_property_readonly("bra", &Tensor::bra)
      .def_property_readonly("ket", &Tensor::ket)
      .def_property_readonly("aux", &Tensor::aux)
      .def_property_readonly("braket",
                             [](const Tensor &t) {
                               auto braket = t.braket();
                               return std::vector<Index>(braket.begin(),
                                                         braket.end());
                             })
      .def_property_readonly("braketaux",
                             [](const Tensor &t) {
                               auto slots = t.braketaux();
                               return std::vector<Index>(slots.begin(),
                                                         slots.end());
                             })
      .def_property_readonly("slots", [](const Tensor &t) {
        auto slots = t.slots();
        return std::vector<Index>(slots.begin(), slots.end());
      });

  py::class_<Complex<rational>>(m, "zRational")
      .def_property_readonly("real",
                             [](const Complex<rational> &r) {
                               return rational_to_fraction(r.real());
                             })
      .def_property_readonly("imag",
                             [](const Complex<rational> &r) {
                               return rational_to_fraction(r.imag());
                             })
      .def_property_readonly("latex", &Complex<rational>::to_latex)
      .def("__str__", &python::complex_to_string)
      .def("__repr__", &python::complex_to_string);

  py::class_<Constant, std::shared_ptr<Constant>, Expr>(m, "Constant")
      .def(py::init(&python::make_constant));

  py::class_<Product, std::shared_ptr<Product>, Expr>(m, "Product")
      .def_property_readonly("scalar", &Product::scalar);

  py::class_<Sum, std::shared_ptr<Sum>, Expr>(m, "Sum");

  m.def("simplify", &sequant::python::simplify,
        "Simplify an expression by expansion, canonicalization, and "
        "algebraic simplification\n\n"
        "Parameters\n"
        "----------\n"
        "expr : ExprPtr\n"
        "    Expression to simplify\n\n"
        "Returns\n"
        "-------\n"
        "ExprPtr\n"
        "    Simplified expression");

  m.def("canonicalize", &sequant::python::canonicalize, py::arg("expr"),
        "Canonicalize tensor indices in an expression\n\n"
        "Applies index canonicalization rules to put tensors in a standard "
        "form.\n\n"
        "Parameters\n"
        "----------\n"
        "expr : ExprPtr\n"
        "    Expression to canonicalize\n\n"
        "Returns\n"
        "-------\n"
        "ExprPtr\n"
        "    Canonicalized expression\n\n"
        "Examples\n"
        "--------\n"
        ">>> import sequant as sq\n"
        ">>> expr = ...  # some expression\n"
        ">>> canonical = sq.canonicalize(expr)");

  m.def("expand", &sequant::python::expand_expr, py::arg("expr"),
        "Expand products of sums\n\n"
        "Recursively expands products of sums into sums of products.\n\n"
        "Parameters\n"
        "----------\n"
        "expr : ExprPtr\n"
        "    Expression to expand\n\n"
        "Returns\n"
        "-------\n"
        "ExprPtr\n"
        "    Expanded expression");

  m.def("flatten", &sequant::python::flatten_expr, py::arg("expr"),
        "Flatten nested sums and products\n\n"
        "Recursively flattens Sum of Sum's and Product of Product's.\n\n"
        "Parameters\n"
        "----------\n"
        "expr : ExprPtr\n"
        "    Expression to flatten\n\n"
        "Returns\n"
        "-------\n"
        "ExprPtr\n"
        "    Flattened expression");

  m.def("rapid_simplify", &sequant::python::rapid_simplify_expr,
        py::arg("expr"),
        "Fast simplification without canonicalization\n\n"
        "Applies cheap transformations like eliminating trivial math and "
        "flattening,\n"
        "but does not perform canonicalization.\n\n"
        "Parameters\n"
        "----------\n"
        "expr : ExprPtr\n"
        "    Expression to simplify\n\n"
        "Returns\n"
        "-------\n"
        "ExprPtr\n"
        "    Simplified expression");

  m.def("size",
        static_cast<std::size_t (*)(const ExprPtr &)>(&sequant::size),
        "Returns the number of subexpressions in an expression.\n"
        "Returns 0 for atoms (Constant, Tensor, etc.), >0 for Sum or Product.\n"
        "For a Sum, this returns the number of summands.");

  python::mbpt::__init__(m.def_submodule("mbpt"));
}
