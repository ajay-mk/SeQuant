//
// Created for SeQuant issue #296 - std::format and std::cout support
//

#include <catch2/catch_test_macros.hpp>

#include "catch2_sequant.hpp"

#include <SeQuant/core/asy_cost.hpp>
#include <SeQuant/core/complex.hpp>
#include <SeQuant/core/eval_expr.hpp>
#include <SeQuant/core/expr.hpp>
#include <SeQuant/core/format.hpp>
#include <SeQuant/core/index.hpp>
#include <SeQuant/core/op.hpp>

#include <format>
#include <sstream>
#include <string>

using namespace sequant;

TEST_CASE("std::format support for Expr types", "[format][expr]") {
  SECTION("Constant") {
    auto c = Constant(2);
    auto result = std::format("{}", c);
    REQUIRE_FALSE(result.empty());
    // The output should contain "2" somewhere
    REQUIRE(result.find("2") != std::string::npos);
  }

  SECTION("Variable") {
    auto v = Variable(L"x");
    auto result = std::format("{}", v);
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.find("x") != std::string::npos);
  }

  SECTION("Product") {
    auto c1 = ex<Constant>(2);
    auto c2 = ex<Constant>(3);
    auto p = c1 * c2;
    auto result = std::format("{}", *p);
    REQUIRE_FALSE(result.empty());
  }

  SECTION("Sum") {
    auto c1 = ex<Constant>(2);
    auto c2 = ex<Constant>(3);
    auto s = c1 + c2;
    auto result = std::format("{}", *s);
    REQUIRE_FALSE(result.empty());
  }

  SECTION("Tensor") {
    auto i1 = Index(L"i_1");
    auto i2 = Index(L"i_2");
    auto t = Tensor(L"t", {i1}, {i2});
    auto result = std::format("{}", t);
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.find("t") != std::string::npos);
  }
}

TEST_CASE("std::format support for ExprPtr", "[format][exprptr]") {
  SECTION("Non-null ExprPtr") {
    auto c = ex<Constant>(42);
    auto result = std::format("{}", c);
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.find("42") != std::string::npos);
  }

  SECTION("Null ExprPtr") {
    ExprPtr null_ptr;
    auto result = std::format("{}", null_ptr);
    REQUIRE(result.find("null") != std::string::npos);
  }
}

TEST_CASE("std::format support for Index", "[format][index]") {
  auto i = Index(L"i_1");
  auto result = std::format("{}", i);
  REQUIRE_FALSE(result.empty());
  REQUIRE(result.find("i") != std::string::npos);
}

TEST_CASE("std::format support for Complex", "[format][complex]") {
  auto c = Complex<rational>(rational(3), rational(4));
  auto result = std::format("{}", c);
  REQUIRE_FALSE(result.empty());
  REQUIRE(result.find("3") != std::string::npos);
  REQUIRE(result.find("4") != std::string::npos);
}

TEST_CASE("std::format support for Op", "[format][op]") {
  auto idx = Index(L"i");
  auto op = Op<Statistics::FermiDirac>(idx, Action::Create);
  auto result = std::format("{}", op);
  REQUIRE_FALSE(result.empty());
}

TEST_CASE("std::format support for EvalExpr", "[format][evalexpr]") {
  auto t = Tensor(L"f", {Index(L"i")}, {Index(L"a")});
  auto eval = EvalExpr(t);
  auto result = std::format("{}", eval);
  REQUIRE_FALSE(result.empty());
  REQUIRE(result.find("f") != std::string::npos);
}

TEST_CASE("std::format support for AsyCost", "[format][asycost]") {
  auto cost = AsyCost(2, 3);  // O^2 V^3
  auto result = std::format("{}", cost);
  REQUIRE_FALSE(result.empty());
}

TEST_CASE("std::ostream support for Expr types", "[ostream][expr]") {
  SECTION("Constant") {
    auto c = Constant(5);
    std::ostringstream oss;
    oss << c;
    auto result = oss.str();
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.find("5") != std::string::npos);
  }

  SECTION("Variable") {
    auto v = Variable(L"y");
    std::ostringstream oss;
    oss << v;
    auto result = oss.str();
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.find("y") != std::string::npos);
  }

  SECTION("Tensor") {
    auto t = Tensor(L"h", {Index(L"p")}, {Index(L"q")});
    std::ostringstream oss;
    oss << t;
    auto result = oss.str();
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.find("h") != std::string::npos);
  }
}

TEST_CASE("std::ostream support for ExprPtr", "[ostream][exprptr]") {
  SECTION("Non-null ExprPtr") {
    auto c = ex<Constant>(7);
    std::ostringstream oss;
    oss << c;
    auto result = oss.str();
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.find("7") != std::string::npos);
  }

  SECTION("Null ExprPtr") {
    ExprPtr null_ptr;
    std::ostringstream oss;
    oss << null_ptr;
    auto result = oss.str();
    REQUIRE(result.find("null") != std::string::npos);
  }
}

TEST_CASE("std::ostream support for Index", "[ostream][index]") {
  auto i = Index(L"j_2");
  std::ostringstream oss;
  oss << i;
  auto result = oss.str();
  REQUIRE_FALSE(result.empty());
  REQUIRE(result.find("j") != std::string::npos);
}

TEST_CASE("std::ostream support for Complex", "[ostream][complex]") {
  auto c = Complex<rational>(rational(1), rational(2));
  std::ostringstream oss;
  oss << c;
  auto result = oss.str();
  REQUIRE_FALSE(result.empty());
  REQUIRE(result.find("1") != std::string::npos);
  REQUIRE(result.find("2") != std::string::npos);
}

TEST_CASE("std::ostream support for Op", "[ostream][op]") {
  auto idx = Index(L"k");
  auto op = Op<Statistics::FermiDirac>(idx, Action::Annihilate);
  std::ostringstream oss;
  oss << op;
  auto result = oss.str();
  REQUIRE_FALSE(result.empty());
}

TEST_CASE("std::ostream support for EvalExpr", "[ostream][evalexpr]") {
  auto c = Constant(rational(1, 2));
  auto eval = EvalExpr(c);
  std::ostringstream oss;
  oss << eval;
  auto result = oss.str();
  REQUIRE_FALSE(result.empty());
}

TEST_CASE("std::ostream support for AsyCost", "[ostream][asycost]") {
  auto cost = AsyCost(rational(1, 2), 3, 4);
  std::ostringstream oss;
  oss << cost;
  auto result = oss.str();
  REQUIRE_FALSE(result.empty());
}

TEST_CASE("Mixed usage - combining format and ostream", "[format][ostream][mixed]") {
  auto i = Index(L"i");
  auto a = Index(L"a");
  auto t = Tensor(L"t", {i}, {a});

  // Use std::format
  auto formatted = std::format("Tensor via format: {}", t);
  REQUIRE_FALSE(formatted.empty());

  // Use std::ostream
  std::ostringstream oss;
  oss << "Tensor via ostream: " << t;
  auto streamed = oss.str();
  REQUIRE_FALSE(streamed.empty());

  // Both should contain the tensor label
  REQUIRE(formatted.find("t") != std::string::npos);
  REQUIRE(streamed.find("t") != std::string::npos);
}

TEST_CASE("Custom format specs - LaTeX vs Code format", "[format][specs]") {
  auto i = Index(L"i_1");
  auto a = Index(L"a_2");
  auto t = Tensor(L"H", {i}, {a});

  SECTION("Index format specs") {
    auto latex_fmt = std::format("{:l}", i);
    auto code_fmt = std::format("{:c}", i);
    auto default_fmt = std::format("{}", i);

    // Default should equal LaTeX
    REQUIRE(default_fmt == latex_fmt);

    // Code format should just be the label
    REQUIRE(code_fmt.find("i_1") != std::string::npos);
  }

  SECTION("Tensor format specs") {
    auto latex_fmt = std::format("{:l}", t);
    auto code_fmt = std::format("{:c}", t);
    auto default_fmt = std::format("{}", t);

    // Default should equal LaTeX
    REQUIRE(default_fmt == latex_fmt);

    // Code format should be bracket notation
    REQUIRE(code_fmt.find("[") != std::string::npos);
    REQUIRE(code_fmt.find(";") != std::string::npos);
    REQUIRE(code_fmt.find("]") != std::string::npos);
  }

  SECTION("Product format specs") {
    auto c1 = ex<Constant>(2);
    auto c2 = ex<Constant>(3);
    auto p = c1 * c2;

    auto latex_fmt = std::format("{:l}", *p);
    auto code_fmt = std::format("{:c}", *p);
    auto default_fmt = std::format("{}", *p);

    // Default should equal LaTeX
    REQUIRE(default_fmt == latex_fmt);

    // Both should contain something representing the product
    REQUIRE_FALSE(latex_fmt.empty());
    REQUIRE_FALSE(code_fmt.empty());
  }

  SECTION("Variable format specs") {
    auto v = Variable(L"x");

    auto latex_fmt = std::format("{:l}", v);
    auto code_fmt = std::format("{:c}", v);
    auto default_fmt = std::format("{}", v);

    // Default should equal LaTeX
    REQUIRE(default_fmt == latex_fmt);

    // Both should contain 'x'
    REQUIRE(latex_fmt.find("x") != std::string::npos);
    REQUIRE(code_fmt.find("x") != std::string::npos);
  }
}

TEST_CASE("External-interface compatibility", "[format][external]") {
  // Test that code format matches what external-interface expects
  auto i = Index(L"i");
  auto a = Index(L"a");
  auto t = Tensor(L"f", {i}, {a});

  auto code_fmt = std::format("{:c}", t);

  // Code format should be: "f[i;a;]"
  REQUIRE(code_fmt.find("f") != std::string::npos);
  REQUIRE(code_fmt.find("[") != std::string::npos);
  REQUIRE(code_fmt.find(";") != std::string::npos);
  REQUIRE(code_fmt.find("]") != std::string::npos);
}
