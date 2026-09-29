#include <SeQuant/core/expressions/result_expr.hpp>
#include <SeQuant/core/expressions/tensor.hpp>
#include <SeQuant/core/expressions/variable.hpp>
#include <SeQuant/core/utility/indices.hpp>

namespace sequant {

bool ResultExpr::ResultCmp::operator()(const ResultExpr &lhs,
                                       const ResultExpr &rhs) const {
  if (lhs.has_label() != rhs.has_label()) {
    return false;
  }

  if (lhs.has_label() && lhs.label() != rhs.label()) {
    return false;
  }

  return lhs.symmetry() == rhs.symmetry() &&
         lhs.braket_symmetry() == rhs.braket_symmetry() &&
         lhs.column_symmetry() == rhs.column_symmetry() &&
         lhs.bra() == rhs.bra() && lhs.ket() == rhs.ket() &&
         lhs.aux() == rhs.aux();
}

ResultExpr::ResultExpr(const Tensor &tensor, ExprPtr expression)
    : expr_(std::move(expression)),
      symm_(tensor.symmetry()),
      bksymm_(tensor.braket_symmetry()),
      csymm_(tensor.column_symmetry()),
      bra_indices_(tensor.bra().begin(), tensor.bra().end()),
      ket_indices_(tensor.ket().begin(), tensor.ket().end()),
      aux_indices_(tensor.aux().begin(), tensor.aux().end()),
      label_(tensor.label()) {}

ResultExpr::ResultExpr(const Variable &variable, ExprPtr expression)
    : expr_(std::move(expression)), label_(variable.label()) {}

ResultExpr::ResultExpr(IndexContainer bra, IndexContainer ket,
                       IndexContainer aux, Symmetry symm,
                       BraKetSymmetry braket_symm, ColumnSymmetry column_symm,
                       std::optional<std::wstring> label, ExprPtr expression)
    : expr_(std::move(expression)),
      symm_(symm),
      bksymm_(braket_symm),
      csymm_(column_symm),
      bra_indices_(std::move(bra)),
      ket_indices_(std::move(ket)),
      aux_indices_(std::move(aux)),
      label_(std::move(label)) {}

ResultExpr &ResultExpr::operator=(ExprPtr expression) {
  expr_ = std::move(expression);

  return *this;
}

bool ResultExpr::has_label() const { return label_.has_value(); }

const std::wstring &ResultExpr::label() const { return label_.value(); }

void ResultExpr::set_label(std::wstring label) { label_ = std::move(label); }

Symmetry ResultExpr::symmetry() const { return symm_; }

void ResultExpr::set_symmetry(Symmetry symm) { symm_ = symm; }

BraKetSymmetry ResultExpr::braket_symmetry() const { return bksymm_; }

void ResultExpr::set_braket_symmetry(BraKetSymmetry symm) { bksymm_ = symm; }

ColumnSymmetry ResultExpr::column_symmetry() const { return csymm_; }

void ResultExpr::set_column_symmetry(ColumnSymmetry symm) { csymm_ = symm; }

const ResultExpr::IndexContainer &ResultExpr::bra() const {
  return bra_indices_;
}

const ResultExpr::IndexContainer &ResultExpr::ket() const {
  return ket_indices_;
}

const ResultExpr::IndexContainer &ResultExpr::aux() const {
  return aux_indices_;
}

const ExprPtr &ResultExpr::expression() const { return expr_; }

ExprPtr &ResultExpr::expression() { return expr_; }

ResultExpr ResultExpr::clone() const {
  return ResultExpr(bra_indices_, ket_indices_, aux_indices_, symm_, bksymm_,
                    csymm_, label_, expr_->clone());
}

}  // namespace sequant
