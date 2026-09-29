#ifndef SEQUANT_CORE_EXPORT_JULIA_TENSOR_OPERATIONS_HPP
#define SEQUANT_CORE_EXPORT_JULIA_TENSOR_OPERATIONS_HPP

#include <SeQuant/core/export/context.hpp>
#include <SeQuant/core/export/generator.hpp>
#include <SeQuant/core/export/reordering_context.hpp>
#include <SeQuant/core/export/utils.hpp>
#include <SeQuant/core/expr.hpp>
#include <SeQuant/core/index.hpp>
#include <SeQuant/core/space.hpp>
#include <SeQuant/core/utility/macros.hpp>
#include <SeQuant/core/utility/string.hpp>

#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

#include <cstdlib>
#include <string>
#include <string_view>

namespace sequant {

/// Context class for the JuliaTensorOperationsGenerator
class JuliaTensorOperationsGeneratorContext : public ReorderingContext {
 public:
  using TagMap = std::map<IndexSpace, std::string>;
  using DimMap = std::map<IndexSpace, std::string>;

  JuliaTensorOperationsGeneratorContext()
      : ReorderingContext(MemoryLayout::ColumnMajor){};
  ~JuliaTensorOperationsGeneratorContext() = default;
  JuliaTensorOperationsGeneratorContext(TagMap index_tags, DimMap index_dims);

  std::string get_tag(const IndexSpace &space) const;

  std::string get_dim(const IndexSpace &space) const;

  std::string get_tags(const Tensor &tensor) const;

  void set_tag(const IndexSpace &space, std::string tag);
  void set_dim(const IndexSpace &space, std::string dim);

 protected:
  TagMap index_tags_;
  DimMap index_dims_;
};

/// Generator for producing Julia code using the ITensor framework
/// - https://julialang.org/
/// - https://jutho.github.io/TensorOperations.jl
template <typename Context = JuliaTensorOperationsGeneratorContext>
class JuliaTensorOperationsGenerator : public Generator<Context> {
 public:
  JuliaTensorOperationsGenerator() = default;
  ~JuliaTensorOperationsGenerator() = default;

  std::string get_format_name() const override {
    return "Julia (TensorOperations)";
  }

  bool supports_named_sections() const override { return false; }

  bool requires_named_sections() const override { return false; }

  bool supports_index_batching() const override { return false; }

  DeclarationScope index_declaration_scope() const override {
    return DeclarationScope::Global;
  }

  DeclarationScope variable_declaration_scope() const override {
    return DeclarationScope::Global;
  }

  DeclarationScope tensor_declaration_scope() const override {
    return DeclarationScope::Global;
  }

  PrunableScalars prunable_scalars() const override {
    return PrunableScalars::All;
  }

  std::string represent(const Index &idx, const Context &) const override {
    if (idx.has_proto_indices()) {
      throw Exception("Proto Indices are not (yet) supported!");
    }

    return to_utf8(idx.full_label());
  }

  std::string represent(const Tensor &tensor,
                        const Context &ctx) const override {
    std::string representation = tensor_name(tensor, ctx);

    representation += "[ ";

    using namespace std::literals;
    representation += tensor.const_indices() |
                      ranges::views::transform([&](const Index &idx) {
                        return represent(idx, ctx);
                      }) |
                      ranges::views::join(", "sv) | ranges::to<std::string>();

    representation += " ]";

    return representation;
  }

  std::string represent(const Variable &variable,
                        const Context &) const override {
    return to_utf8(variable.label());
  }

  std::string represent(const Constant &constant,
                        const Context &) const override {
    std::stringstream sstream;

    if (constant.value().imag() != 0) {
      sstream << "(" << constant.value().real();
      if (constant.value().imag() < 0) {
        sstream << "-" << (-1 * constant.value().imag()) << "im)";
      } else {
        sstream << "+" << constant.value().imag() << "im)";
      }
    } else {
      sstream << constant.value().real();
    }

    return sstream.str();
  }

  std::string represent(const Power &power, const Context &ctx) const override {
    return detail::format_power(power, to_julia_expr(*power.base(), ctx), "^",
                                /*double_slash*/ true, &wrap_conj);
  }

  void create(const Tensor &tensor, bool zero_init,
              const Context &ctx) override {
    if (!zero_init) {
      throw Exception(
          "In Julia tensors can't be created without being initialized");
    }

    generated_ += zero_initialization(tensor, ctx) + "\n";
  }

  void load(const Tensor &tensor, bool set_to_zero,
            const Context &ctx) override {
    if (!set_to_zero) {
      generated_ += tensor_name(tensor, ctx);
      generated_ += " = ";
      generated_ += "deserialize(\"" + tensor_name(tensor, ctx) + ".jlbin\")";
    } else {
      // zero_initialization already emits "<name> = zeros(...)"
      generated_ += zero_initialization(tensor, ctx);
    }

    generated_ += "\n";
  }

  void set_to_zero(const Tensor &tensor, const Context &ctx) override {
    // In Julia setting to zero essentially has same effect as creating a zero
    // tensor
    create(tensor, true, ctx);
  }

  void unload(const Tensor &tensor, const Context &ctx) override {
    generated_ += tensor_name(tensor, ctx) + " = nothing\n";
  }

  void destroy(const Tensor &tensor, const Context &ctx) override {
    unload(tensor, ctx);
    // Remove potential serialized version of this tensor from disk
    generated_ += "rm( \"" + tensor_name(tensor, ctx) + +".jlbin\"" + ")\n";
  }

  void persist(const Tensor &tensor, const Context &ctx) override {
    generated_ += "return " + tensor_name(tensor, ctx) + "\n";
  }

  void create(const Variable &variable, bool zero_init,
              const Context &ctx) override {
    if (!zero_init) {
      throw Exception(
          "Julia doesn't support declaring a variable without initializing it");
    }

    generated_ += represent(variable, ctx) + " = 0.0\n";
  }
  void load(const Variable &variable, bool set_to_zero,
            const Context &ctx) override {
    generated_ += represent(variable, ctx) + " = ";

    if (set_to_zero)
      generated_ += "0.0";
    else {
      generated_ += "deserialize(\"" + represent(variable, ctx) + ".jlbin\")";
    }
    generated_ += "\n";
  }

  void set_to_zero(const Variable &variable, const Context &ctx) override {
    generated_ += represent(variable, ctx) + " = 0.0\n";
  }

  void unload(const Variable &variable, const Context &ctx) override {
    generated_ += represent(variable, ctx) + " = nothing\n";
  }

  void destroy(const Variable &variable, const Context &ctx) override {
    unload(variable, ctx);
    // Remove any potentially serialized version from disk
    generated_ += "rm(\"" + represent(variable, ctx) + ".jlbin\")\n";
  }

  void persist(const Variable &variable, const Context &ctx) override {
    generated_ += "return " + represent(variable, ctx) + "\n";
  }

  void compute(const Expr &expression, const Tensor &result,
               const Context &ctx) override {
    generated_ += "@tensor " + represent(result, ctx) +
                  " += " + to_julia_expr(expression, ctx) + "\n";
  }

  void compute(const Expr &expression, const Variable &result,
               const Context &ctx) override {
    generated_ += "@tensor " + represent(result, ctx) +
                  " += " + to_julia_expr(expression, ctx) + "\n";
  }

  void declare(const Index &, const Context &) override {}

  void declare(const Variable &, UsageSet, const Context &) override {}

  void declare(const Tensor &, UsageSet, const Context &) override {}

  void all_indices_declared(std::size_t, const Context &) override {}

  void all_variables_declared(std::size_t, const Context &) override {}

  void all_tensors_declared(std::size_t, const Context &) override {}

  void begin_declarations(DeclarationScope, const Context &) override {}

  void end_declarations(DeclarationScope, const Context &) override {}

  void insert_comment(const std::string &comment, const Context &) override {
    generated_ += "# " + comment + "\n";
  }

  void begin_named_section(std::string_view, const Context &) override {
    SEQUANT_ABORT("This function should not have been called");
  }

  void end_named_section(std::string_view, const Context &) override {
    SEQUANT_ABORT("This function should not have been called");
  }

  void begin_expression(const Context &) override {
    if (!generated_.empty() && !generated_.ends_with("\n\n")) {
      generated_ += "\n";
    }
  }

  void end_expression(const Context &) override {}

  void begin_export(const Context &) override { generated_.clear(); }

  void end_export(const Context &) override {}

  std::string get_generated_code() const override { return generated_; }

 protected:
  std::string generated_;

  std::string to_julia_expr(const Expr &expr, const Context &ctx) const {
    return detail::stringify_expr(
        expr, " * ",
        [this, &ctx](const auto &e) { return this->represent(e, ctx); },
        "to_julia_expr");
  }

  static std::string wrap_conj(std::string s) {
    return "conj(" + std::move(s) + ")";
  }

  /// emits `<name> = <wrapper>(zeros(Float64, <dims>...), <shape>)`
  void create_wrapped(const Tensor &tensor, bool zero_init, const Context &ctx,
                      std::string_view wrapper, const std::string &shape) {
    if (!zero_init) {
      throw Exception(
          "In Julia tensors can't be created without being initialized");
    }

    generated_ += tensor_name(tensor, ctx);
    generated_ += " = ";
    generated_ += wrapper;
    generated_ += "(zeros(Float64";

    for (const Index &idx : tensor.const_indices()) {
      generated_ += ", ";
      generated_ += ctx.get_dim(idx.space());
    }

    generated_ += "), ";
    generated_ += shape;
    generated_ += ")\n";
  }

  /// emits `<name> = <wrapper>(deserialize("<name>.jlbin"), <shape>)`, or
  /// the zero-initialization of create_wrapped if @p set_to_zero
  void load_wrapped(const Tensor &tensor, bool set_to_zero, const Context &ctx,
                    std::string_view wrapper, const std::string &shape) {
    if (set_to_zero) {
      // In Julia setting a tensor to zero has to be done by overwriting it with
      // a zero tensor
      create_wrapped(tensor, true, ctx, wrapper, shape);
      return;
    }

    generated_ += tensor_name(tensor, ctx);
    generated_ += " = ";
    generated_ += wrapper;
    generated_ += "(deserialize(\"";
    generated_ += tensor_name(tensor, ctx);
    generated_ += ".jlbin\"), ";
    generated_ += shape;
    generated_ += ")\n";
  }

  std::string tensor_name(const Tensor &tensor, const Context &ctx) const {
    std::string representation = to_utf8(tensor.label());

    representation += "_";
    representation += ctx.get_tags(tensor);

    return representation;
  }

  std::string zero_initialization(const Tensor &tensor,
                                  const Context &ctx) const {
    std::string code = tensor_name(tensor, ctx);
    code += " = zeros(Float64";

    for (const Index &idx : tensor.const_indices()) {
      code += ", ";
      code += ctx.get_dim(idx.space());
    }

    code += ")";

    return code;
  }
};

}  // namespace sequant

#endif  // SEQUANT_CORE_EXPORT_JULIA_TENSOR_OPERATIONS_HPP
