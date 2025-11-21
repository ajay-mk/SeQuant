//
// std::format and std::cout support for SeQuant types
//

#ifndef SEQUANT_CORE_FORMAT_HPP
#define SEQUANT_CORE_FORMAT_HPP

#include <SeQuant/core/asy_cost.hpp>
#include <SeQuant/core/complex.hpp>
#include <SeQuant/core/eval_expr.hpp>
#include <SeQuant/core/expressions/expr.hpp>
#include <SeQuant/core/expressions/expr_ptr.hpp>
#include <SeQuant/core/expressions/tensor.hpp>
#include <SeQuant/core/expressions/variable.hpp>
#include <SeQuant/core/expressions/constant.hpp>
#include <SeQuant/core/expressions/product.hpp>
#include <SeQuant/core/expressions/sum.hpp>
#include <SeQuant/core/expressions/result_expr.hpp>
#include <SeQuant/core/index.hpp>
#include <SeQuant/core/op.hpp>
#include <SeQuant/core/rational.hpp>
#include <SeQuant/core/utility/string.hpp>

#include <range/v3/core.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

// ============================================================================
// std::formatter specializations with custom format specs
// Format specs:
//   {:l} or {} - LaTeX format (default) - uses to_latex()
//   {:c}       - Code format - for code generation
// ============================================================================

namespace std {

/// std::formatter specialization for sequant::Expr base class
/// Supports {:l} for LaTeX (default) and {:c} for code format
template <>
struct formatter<sequant::Expr> : formatter<string_view> {
  char presentation = 'l';  // 'l' for latex (default), 'c' for code

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for Expr");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::Expr& expr, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    if (presentation == 'l') {
      return format_to(ctx.out(), "{}", sequant::toUtf8(expr.to_latex()));
    } else {
      // Code format - dispatch based on type
      if (expr.is<sequant::Tensor>()) {
        return format_to(ctx.out(), "{:c}", expr.as<sequant::Tensor>());
      } else if (expr.is<sequant::Constant>()) {
        return format_to(ctx.out(), "{:c}", expr.as<sequant::Constant>());
      } else if (expr.is<sequant::Variable>()) {
        return format_to(ctx.out(), "{:c}", expr.as<sequant::Variable>());
      } else if (expr.is<sequant::Sum>()) {
        return format_to(ctx.out(), "{:c}", expr.as<sequant::Sum>());
      } else if (expr.is<sequant::Product>()) {
        return format_to(ctx.out(), "{:c}", expr.as<sequant::Product>());
      } else {
        return format_to(ctx.out(), "<Unknown expression type>");
      }
    }
  }
};

/// std::formatter specialization for sequant::ExprPtr
template <>
struct formatter<sequant::ExprPtr> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for ExprPtr");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::ExprPtr& expr_ptr, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    if (!expr_ptr) {
      return format_to(ctx.out(), "<null ExprPtr>");
    }
    if (presentation == 'l') {
      return format_to(ctx.out(), "{:l}", *expr_ptr);
    } else {
      return format_to(ctx.out(), "{:c}", *expr_ptr);
    }
  }
};

/// std::formatter specialization for sequant::Index
template <>
struct formatter<sequant::Index> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for Index");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::Index& idx, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    if (presentation == 'l') {
      return format_to(ctx.out(), "{}", sequant::toUtf8(idx.to_latex()));
    } else {
      // Code format - just the label
      if (idx.has_proto_indices()) {
        return format_to(ctx.out(), "{}", sequant::toUtf8(idx.full_label()));
      }
      return format_to(ctx.out(), "{}", sequant::toUtf8(idx.label()));
    }
  }
};

/// std::formatter specialization for sequant::Tensor
template <>
struct formatter<sequant::Tensor> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for Tensor");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::Tensor& tensor, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    if (presentation == 'l') {
      return format_to(ctx.out(), "{}", sequant::toUtf8(tensor.to_latex()));
    } else {
      // Code format: "label[bra;ket;aux]"
      auto idx_to_string = [](const sequant::Index& idx) {
        return std::format("{:c}", idx);
      };
      using namespace std::literals;
      return format_to(
          ctx.out(), "{}[{};{};{}]", sequant::toUtf8(tensor.label()),
          tensor.bra() | ::ranges::views::transform(idx_to_string) |
              ::ranges::views::join(", "sv) | ::ranges::to<std::string>(),
          tensor.ket() | ::ranges::views::transform(idx_to_string) |
              ::ranges::views::join(", "sv) | ::ranges::to<std::string>(),
          tensor.aux() | ::ranges::views::transform(idx_to_string) |
              ::ranges::views::join(", "sv) | ::ranges::to<std::string>());
    }
  }
};

/// std::formatter specialization for sequant::Variable
template <>
struct formatter<sequant::Variable> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for Variable");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::Variable& variable, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    if (presentation == 'l') {
      return format_to(ctx.out(), "{}", sequant::toUtf8(variable.to_latex()));
    } else {
      // Code format - just the label
      return format_to(ctx.out(), "{}", sequant::toUtf8(variable.label()));
    }
  }
};

/// std::formatter specialization for sequant::Constant
template <>
struct formatter<sequant::Constant> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for Constant");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::Constant& constant, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    if (presentation == 'l') {
      return format_to(ctx.out(), "{}", sequant::toUtf8(constant.to_latex()));
    } else {
      // Code format - the value
      return format_to(ctx.out(), "{:c}", constant.value());
    }
  }
};

/// std::formatter specialization for sequant::Product
template <>
struct formatter<sequant::Product> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for Product");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::Product& product, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    if (presentation == 'l') {
      return format_to(ctx.out(), "{}", sequant::toUtf8(product.to_latex()));
    } else {
      // Code format - space-separated factors
      auto factors = product.factors() |
                     ::ranges::views::transform([](const sequant::ExprPtr& expr) {
                       return std::format("{:c}", *expr);
                     }) |
                     ::ranges::views::join(' ') | ::ranges::to<std::string>();

      if (product.scalar().is_identity()) {
        return format_to(ctx.out(), "{}", factors);
      } else {
        return format_to(ctx.out(), "{:c} {}", product.scalar(), factors);
      }
    }
  }
};

/// std::formatter specialization for sequant::Sum
template <>
struct formatter<sequant::Sum> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for Sum");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::Sum& sum, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    if (presentation == 'l') {
      return format_to(ctx.out(), "{}", sequant::toUtf8(sum.to_latex()));
    } else {
      // Code format - newline + separated summands
      using namespace std::literals;
      return format_to(
          ctx.out(), "{}",
          sum.summands() |
              ::ranges::views::transform([](const sequant::ExprPtr& expr) {
                return std::format("{:c}", *expr);
              }) |
              ::ranges::views::join("\n+ "sv) | ::ranges::to<std::string>());
    }
  }
};

/// std::formatter specialization for sequant::rational
template <>
struct formatter<sequant::rational> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for rational");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::rational& number, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    // For rational, both formats are the same - just the string representation
    std::stringstream sstream;
    sstream << number;
    return format_to(ctx.out(), "{}", sstream.str());
  }
};

/// std::formatter specialization for sequant::Complex<T>
template <typename T>
struct formatter<sequant::Complex<T>> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for Complex");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::Complex<T>& number, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    if (presentation == 'l') {
      return format_to(ctx.out(), "{}", sequant::toUtf8(number.to_latex()));
    } else {
      // Code format - simpler notation
      if (number.imag().is_zero()) {
        return format_to(ctx.out(), "{:c}", number.real());
      } else if (number.real().is_zero()) {
        return format_to(ctx.out(), "{:c}i", number.imag());
      } else if (number.imag() < 0) {
        decltype(number.imag()) imag = -number.imag();
        return format_to(ctx.out(), "({:c} - {:c}i)", number.real(), imag);
      } else {
        return format_to(ctx.out(), "({:c} + {:c}i)", number.real(),
                         number.imag());
      }
    }
  }
};

/// std::formatter specialization for sequant::Op<S>
template <sequant::Statistics S>
struct formatter<sequant::Op<S>> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for Op");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::Op<S>& op, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    // For Op, always use LaTeX format as there's no code-gen equivalent
    return format_to(ctx.out(), "{}", sequant::toUtf8(op.to_latex()));
  }
};

/// std::formatter specialization for sequant::EvalExpr
template <>
struct formatter<sequant::EvalExpr> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for EvalExpr");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::EvalExpr& eval_expr, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    // For EvalExpr, always use LaTeX format
    return format_to(ctx.out(), "{}", sequant::toUtf8(eval_expr.to_latex()));
  }
};

/// std::formatter specialization for sequant::AsyCost
template <>
struct formatter<sequant::AsyCost> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for AsyCost");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::AsyCost& cost, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    // For AsyCost, always use LaTeX format
    return format_to(ctx.out(), "{}", sequant::toUtf8(cost.to_latex()));
  }
};

/// std::formatter specialization for sequant::ResultExpr
template <>
struct formatter<sequant::ResultExpr> : formatter<string_view> {
  char presentation = 'l';

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && (*it == 'l' || *it == 'c')) {
      presentation = *it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format spec for ResultExpr");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const sequant::ResultExpr& result, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    std::string label =
        result.has_label() ? sequant::toUtf8(result.label()) : "?";

    if (result.bra().empty() && result.ket().empty() && result.aux().empty()) {
      if (presentation == 'l') {
        return format_to(ctx.out(), "{} =\n{:l}", label, result.expression());
      } else {
        return format_to(ctx.out(), "{} =\n{:c}", label, result.expression());
      }
    }

    auto idx_to_string = [this](const sequant::Index& idx) {
      if (presentation == 'l') {
        return std::format("{:l}", idx);
      } else {
        return std::format("{:c}", idx);
      }
    };

    using namespace std::literals;
    if (presentation == 'l') {
      return format_to(
          ctx.out(), "{}[{};{};{}] =\n {:l}", label,
          result.bra() | ::ranges::views::transform(idx_to_string) |
              ::ranges::views::join(", "sv) | ::ranges::to<std::string>(),
          result.ket() | ::ranges::views::transform(idx_to_string) |
              ::ranges::views::join(", "sv) | ::ranges::to<std::string>(),
          result.aux() | ::ranges::views::transform(idx_to_string) |
              ::ranges::views::join(", "sv) | ::ranges::to<std::string>(),
          result.expression());
    } else {
      return format_to(
          ctx.out(), "{}[{};{};{}] =\n {:c}", label,
          result.bra() | ::ranges::views::transform(idx_to_string) |
              ::ranges::views::join(", "sv) | ::ranges::to<std::string>(),
          result.ket() | ::ranges::views::transform(idx_to_string) |
              ::ranges::views::join(", "sv) | ::ranges::to<std::string>(),
          result.aux() | ::ranges::views::transform(idx_to_string) |
              ::ranges::views::join(", "sv) | ::ranges::to<std::string>(),
          result.expression());
    }
  }
};

}  // namespace std

// ============================================================================
// operator<< overloads for std::ostream (in sequant namespace for ADL)
// Always use LaTeX format for stream output
// ============================================================================

namespace sequant {

/// Stream output operator for Expr base class
inline std::ostream& operator<<(std::ostream& os, const Expr& expr) {
  return os << std::format("{:l}", expr);
}

/// Stream output operator for ExprPtr
inline std::ostream& operator<<(std::ostream& os, const ExprPtr& expr_ptr) {
  if (expr_ptr) {
    return os << std::format("{:l}", expr_ptr);
  } else {
    return os << "<null ExprPtr>";
  }
}

/// Stream output operator for Index
inline std::ostream& operator<<(std::ostream& os, const Index& idx) {
  return os << std::format("{:l}", idx);
}

/// Stream output operator for Complex<T>
template <typename T>
inline std::ostream& operator<<(std::ostream& os, const Complex<T>& complex) {
  return os << std::format("{:l}", complex);
}

/// Stream output operator for Op<S>
template <Statistics S>
inline std::ostream& operator<<(std::ostream& os, const Op<S>& op) {
  return os << std::format("{:l}", op);
}

/// Stream output operator for EvalExpr
inline std::ostream& operator<<(std::ostream& os, const EvalExpr& eval_expr) {
  return os << std::format("{:l}", eval_expr);
}

/// Stream output operator for AsyCost
inline std::ostream& operator<<(std::ostream& os, const AsyCost& cost) {
  return os << std::format("{:l}", cost);
}

/// Stream output operator for ResultExpr
inline std::ostream& operator<<(std::ostream& os, const ResultExpr& result) {
  return os << std::format("{:l}", result);
}

}  // namespace sequant

#endif  // SEQUANT_CORE_FORMAT_HPP
