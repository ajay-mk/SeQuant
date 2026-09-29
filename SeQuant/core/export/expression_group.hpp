#ifndef SEQUANT_CORE_EXPORT_EXPRESSION_GROUP_HPP
#define SEQUANT_CORE_EXPORT_EXPRESSION_GROUP_HPP

#include <SeQuant/core/container.hpp>
#include <SeQuant/core/export/export_expr.hpp>
#include <SeQuant/core/export/export_node.hpp>
#include <SeQuant/core/utility/macros.hpp>

#include <optional>
#include <string>

namespace sequant {

/// Groups multiple expressions together that are supposed to be exported as a
/// single (named) group (e.g. in a function).
template <typename T = ExportExpr>
class ExpressionGroup {
 private:
  using container_type = container::svector<ExportNode<T>>;
  using node_data_type = T;

 public:
  using iterator = container_type::iterator;
  using reverse_iterator = container_type::reverse_iterator;
  using const_iterator = container_type::const_iterator;
  using const_reverse_iterator = container_type::const_reverse_iterator;

  explicit ExpressionGroup(std::optional<std::string> name = {})
      : ExpressionGroup(container_type{}, std::move(name)) {}
  explicit ExpressionGroup(ExportNode<T> expression)
      : ExpressionGroup(container_type{std::move(expression)}, {}) {}
  ExpressionGroup(ExportNode<T> expression, std::optional<std::string> name)
      : ExpressionGroup(container_type{std::move(expression)},
                        std::move(name)) {}
  explicit ExpressionGroup(container_type expressions)
      : ExpressionGroup(std::move(expressions), {}) {}
  ExpressionGroup(container_type expressions, std::optional<std::string> name)
      : expressions_(std::move(expressions)), name_(std::move(name)) {
    SEQUANT_ASSERT(!name_.has_value() || !name_.value().empty());
  }

  template <typename Range>
    requires std::ranges::range<Range> &&
             std::is_same_v<std::ranges::range_value_t<Range>,
                            typename container_type::value_type>
  explicit ExpressionGroup(const Range &rng,
                           std::optional<std::string> name = {})
      : ExpressionGroup(
            container_type(std::ranges::begin(rng), std::ranges::end(rng)),
            std::move(name)) {}

  [[nodiscard]] bool is_named() const { return name_.has_value(); }

  [[nodiscard]] const std::string &name() const { return name_.value(); }

  [[nodiscard]] iterator begin() { return expressions_.begin(); }
  [[nodiscard]] iterator end() { return expressions_.end(); }
  [[nodiscard]] const_iterator begin() const { return expressions_.begin(); }
  [[nodiscard]] const_iterator end() const { return expressions_.end(); }
  [[nodiscard]] const_iterator cbegin() const { return expressions_.cbegin(); }
  [[nodiscard]] const_iterator cend() const { return expressions_.cend(); }
  [[nodiscard]] reverse_iterator rbegin() { return expressions_.rbegin(); }
  [[nodiscard]] reverse_iterator rend() { return expressions_.rend(); }
  [[nodiscard]] const_reverse_iterator rbegin() const {
    return expressions_.rbegin();
  }
  [[nodiscard]] const_reverse_iterator rend() const {
    return expressions_.rend();
  }
  [[nodiscard]] const_reverse_iterator crbegin() const {
    return expressions_.crbegin();
  }
  [[nodiscard]] const_reverse_iterator crend() const {
    return expressions_.crend();
  }

  [[nodiscard]] ExportNode<T> &front() { return expressions_.front(); }
  [[nodiscard]] const ExportNode<T> &front() const {
    return expressions_.front();
  }
  [[nodiscard]] ExportNode<T> &back() { return expressions_.back(); }
  [[nodiscard]] const ExportNode<T> &back() const {
    return expressions_.back();
  }

  [[nodiscard]] std::size_t size() const { return expressions_.size(); }

  void add(ExportNode<T> expr) { expressions_.emplace_back(std::move(expr)); }

  iterator insert(const_iterator pos, ExportNode<T> expr) {
    return expressions_.insert(pos, std::move(expr));
  }

 private:
  container::svector<ExportNode<T>> expressions_;
  std::optional<std::string> name_;
};

}  // namespace sequant

#endif
