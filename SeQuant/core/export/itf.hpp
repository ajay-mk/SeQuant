#ifndef SEQUANT_CORE_EXPORT_ITF_HPP
#define SEQUANT_CORE_EXPORT_ITF_HPP

#include <SeQuant/core/container.hpp>
#include <SeQuant/core/export/context.hpp>
#include <SeQuant/core/export/reordering_context.hpp>
#include <SeQuant/core/export/text_generator.hpp>
#include <SeQuant/core/export/utils.hpp>
#include <SeQuant/core/expr.hpp>
#include <SeQuant/core/index.hpp>
#include <SeQuant/core/space.hpp>
#include <SeQuant/core/utility/macros.hpp>
#include <SeQuant/core/utility/string.hpp>
#include <SeQuant/core/utility/tensor.hpp>

#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace sequant {

/// Context for the ItfGenerator
class ItfContext : public ReorderingContext {
 public:
  using TagMap = container::map<IndexSpace, std::string>;
  using NameMap = container::map<IndexSpace, std::string>;
  using TensorImportMap =
      container::map<Tensor, std::string, TensorBlockLessThanComparator>;
  using VariableImportMap = container::map<Variable, std::string>;

  ItfContext() : ReorderingContext(MemoryLayout::ColumnMajor){};
  ~ItfContext() = default;

  /// @returns The ITF representation of an index in the given space and with
  /// the given ordinal
  virtual std::string index_name(const IndexSpace &space,
                                 std::size_t ordinal) const;

  /// @returns The ITF representation of the given index space
  virtual std::string get_name(const IndexSpace &space) const;

  /// @returns The tag corresponding to the given index space. Tags are appended
  /// to tensor names in order to make different blocks of the same tensor
  /// distinguishable
  virtual std::string get_tag(const IndexSpace &space) const;

  /// @returns The name under which the given tensor shall be imported. If none
  /// is obtained, the tensor will not be imported.
  virtual std::optional<std::string> import_name(const Tensor &tensor) const;
  /// @returns The name under which the given variable shall be imported. If
  /// none is obtained, the variable will not be imported.
  virtual std::optional<std::string> import_name(
      const Variable &variable) const;

  /// Sets the name of the given index space
  virtual void set_name(const IndexSpace &space, std::string name);

  /// Sets the name for the given index space
  virtual void set_tag(const IndexSpace &space, std::string tag);

  /// Sets the name under which the given tensor is to be exported
  virtual void set_import_name(const Tensor &tensor, std::string name);
  /// Sets the name under which the given variable is to be exported
  virtual void set_import_name(const Variable &variable, std::string name);

  /// @returns The label of the 2-electron integral tensor
  virtual const std::wstring two_electron_integral_label() const {
    return integral_label_;
  }
  /// Sets the label of the 2-electron integral tensor
  virtual void set_two_electron_integral_label(std::wstring label) {
    integral_label_ = std::move(label);
  }

  using ReorderingContext::rewrite;
  bool rewrite(Tensor &tensor) const override;

  /// @returns The offset index ordinals are using. All index ordinals are
  /// expected to be >= this value
  virtual std::size_t index_id_offset() const;
  /// Sets the offset index ordinals are using. All index ordinals are expected
  /// to be >= this value
  virtual void set_index_id_offset(std::size_t offset);

  void set_batch_indices(std::span<const Index> indices,
                         std::optional<std::size_t> id = {}) override;
  std::span<const Index> batch_indices(
      std::optional<std::size_t> id = {}) const override;

 private:
  NameMap space_names_;
  TagMap tags_;
  TensorImportMap tensor_imports_;
  VariableImportMap variable_imports_;
  container::map<IndexSpace, char> index_label_limits_;
  std::wstring integral_label_ = L"g";
  std::size_t idx_offset_ = 0;
  std::map<std::size_t, std::vector<Index>> batch_indices_;

 protected:
  bool is_exceptional_J(std::span<Index> bra, std::span<Index> ket) const;
};

/// Generator for ITF. ITF (Integrated Tensor Framework) is a domain-specific
/// language for tensor contractions used by parts of the Molpro quantum
/// chemistry program.
/// WIREs Comput Mol Sci 2012, 2: 242-253 doi: 10.1002/wcms.82
template <typename Context = ItfContext>
class ItfGenerator : public Generator<Context> {
 public:
  ItfGenerator() = default;
  ~ItfGenerator() = default;

  std::string get_format_name() const override { return "ITF"; }

  bool supports_named_sections() const override { return true; }

  bool requires_named_sections() const override { return true; }

  bool supports_index_batching() const override { return true; }

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
    return PrunableScalars::Constants;
  }

  std::string represent(const Index &idx, const Context &ctx) const override {
    if (idx.has_proto_indices()) {
      throw Exception("ITF doesn't support proto indices");
    }

    const std::size_t ordinal = idx.ordinal().value();

    std::string name = ctx.index_name(idx.space(), ordinal);
    SEQUANT_ASSERT(!name.empty());

    if (name.size() > 1) {
      name = "{" + name + "}";
    }

    return name;
  }

  std::string get_name(const Tensor &tensor, const Context &ctx) const {
    std::string name = to_utf8(tensor.label());

    if (tensor.num_indices() > 0) {
      name += ":";
      for (const Index &idx : tensor.const_indices()) {
        name += ctx.get_tag(idx.space());
      }

      if (name.back() == ':') {
        // all tags have been empty -> remove colon and leave tensor untagged
        name.erase(name.size() - 1);
      }
    }

    return name;
  }

  std::string represent(const Tensor &tensor,
                        const Context &ctx) const override {
    std::string representation = get_name(tensor, ctx);

    representation += "[";

    for (const Index &idx : tensor.indices()) {
      representation += represent(idx, ctx);
    }

    representation += "]";

    return representation;
  }

  std::string represent(const Variable &variable,
                        const Context &) const override {
    return to_utf8(variable.label()) + "[]";
  }

  std::string represent(const Constant &constant,
                        const Context &) const override {
    std::stringstream sstream;
    if (constant.value().imag() != 0) {
      sstream << "(" << constant.value().real();
      if (constant.value().imag() < 0) {
        sstream << " - j" << (-1 * constant.value().imag());
      } else {
        sstream << " + j" << constant.value().imag();
      }
      sstream << ")";
    } else {
      sstream << constant.value().real();
    }
    return sstream.str();
  }

  std::string represent(const Power &power, const Context &ctx) const override {
    const ExprPtr &base = power.base();
    // ITF can only express powers of Constants
    if (!base->is<Constant>()) {
      throw Exception(
          "ITF only supports Power with a Constant base; got base of type " +
          base->type_name());
    }
    auto s =
        detail::format_power_base(base, represent(base->as<Constant>(), ctx)) +
        "**" +
        detail::format_power_exponent(power.exponent(),
                                      /*double_slash*/ false);
    if (power.conjugated() && (base->as<Constant>().value().imag() != 0 ||
                               power.exponent().imag() != 0)) {
      s = "(" + s + ").conjugate()";
    }
    return s;
  }

  void create(const Tensor &tensor, bool zero_init,
              const Context &ctx) override {
    if (!zero_init) {
      throw Exception("Can't create ITF tensor without setting it to zero");
    }

    append_line("alloc " + represent(tensor, ctx));
  }

  void load(const Tensor &tensor, bool set_to_zero,
            const Context &ctx) override {
    if (set_to_zero) {
      create(tensor, true, ctx);
      return;
    }

    append_line("load " + represent(tensor, ctx));
  }

  void set_to_zero(const Tensor &tensor, const Context &ctx) override {
    append_line("." + represent(tensor, ctx) + " := 0 * One[]");
  }

  void unload(const Tensor &tensor, const Context &ctx) override {
    append_line("drop " + represent(tensor, ctx));
  }

  void destroy(const Tensor &tensor, const Context &ctx) override {
    // There exists no real notion of deleting/destroying a given tensor
    unload(tensor, ctx);
  }

  void persist(const Tensor &tensor, const Context &ctx) override {
    append_line("store " + represent(tensor, ctx));
  }

  void create(const Variable &variable, bool zero_init,
              const Context &ctx) override {
    if (!zero_init) {
      throw Exception("Can't create ITF variable without setting it to zero");
    }

    append_line("alloc " + represent(variable, ctx));
  }

  void load(const Variable &variable, bool set_to_zero,
            const Context &ctx) override {
    append_line("load " + represent(variable, ctx));

    if (set_to_zero) {
      this->set_to_zero(variable, ctx);
    }
  }

  void set_to_zero(const Variable &variable, const Context &ctx) override {
    append_line("." + represent(variable, ctx) + " := 0 * One[]");
  }

  void unload(const Variable &variable, const Context &ctx) override {
    append_line("drop " + represent(variable, ctx));
  }

  void destroy(const Variable &variable, const Context &ctx) override {
    // There exists no real notion of deleting/destroying a given variable
    unload(variable, ctx);
  }

  void persist(const Variable &variable, const Context &ctx) override {
    append_line("store " + represent(variable, ctx));
  }

  void compute(const Expr &expression, const Variable &result,
               const Context &ctx) override {
    append_line("." + represent(result, ctx) +
                " += " + stringify(expression, ctx));
  }

  void compute(const Expr &expression, const Tensor &result,
               const Context &ctx) override {
    append_line("." + represent(result, ctx) +
                " += " + stringify(expression, ctx));
  }

  void declare(const Index &idx, const Context &ctx) override {
    append_line("index-space: " + represent(idx, ctx) + ", " +
                ctx.get_name(idx.space()) + ", " + ctx.get_tag(idx.space()));
  }

  void declare(const Variable &variable, UsageSet usage,
               const Context &ctx) override {
    append("tensor: " + represent(variable, ctx) + ", ");

    std::optional<std::string> import_name = ctx.import_name(variable);
    bool needs_import = import_name.has_value() || usage == Usage::Terminal ||
                        usage == Usage::Result;

    if (needs_import) {
      if (import_name.has_value()) {
        append(import_name.value());
      } else {
        append(to_utf8(variable.label()));
      }
    } else {
      append("!Create{type:scalar}");
    }

    append_line();
  }

  void declare(const Tensor &tensor, UsageSet usage,
               const Context &ctx) override {
    append("tensor: " + represent(tensor, ctx) + ", ");

    std::optional<std::string> import_name = ctx.import_name(tensor);
    bool needs_import = import_name.has_value() || usage == Usage::Terminal ||
                        usage == Usage::Result;

    if (needs_import) {
      if (import_name.has_value()) {
        append(import_name.value());
      } else {
        append(get_name(tensor, ctx));
      }
    } else if (usage == Usage::Intermediate) {
      append("!Create{type:plain}");
    } else {
      append("!Create{type:disk}");
    }

    append_line();
  }

  void all_indices_declared(std::size_t amount, const Context &) override {
    if (amount > 0) {
      append_line();
    }
  }

  void all_variables_declared(std::size_t amount, const Context &) override {
    if (amount > 0) {
      append_line();
    }
  }

  void all_tensors_declared(std::size_t amount, const Context &) override {
    if (amount > 0) {
      append_line();
    }
  }

  void begin_declarations(DeclarationScope scope, const Context &) override {
    if (scope == DeclarationScope::Global) {
      append_line("---- decl");
    }
  }

  void end_declarations(DeclarationScope scope, const Context &) override {
    if (scope == DeclarationScope::Global) {
      append_line();
    }
  }

  void insert_comment(const std::string &comment, const Context &) override {
    append_line("// " + comment);
  }

  void begin_named_section(std::string_view name, const Context &) override {
    append_line("---- code(\"" + std::string(name) + "\")");
  }

  void end_named_section(std::string_view /*name*/, const Context &) override {
    end_batching();

    append_line();
    append_line();
  }

  void begin_expression(const Context &ctx) override {
    if (!generated_.empty() && !generated_.ends_with("\n\n") &&
        !generated_.ends_with(")\n")) {
      append_line();
    }

    if (ctx.is_batched(ctx.current_expression_id())) {
      std::span<const Index> batch_indices =
          ctx.batch_indices(ctx.current_expression_id());

      SEQUANT_ASSERT(std::ranges::is_sorted(
          batch_indices, std::greater<>{},
          [](const Index &idx) { return idx.space().approximate_size(); }));

      std::size_t num_shared = 0;
      for (std::size_t i = 0;
           i < std::min(current_batch_indices_.size(), batch_indices.size());
           ++i) {
        if (batch_indices[i] != current_batch_indices_[i]) {
          break;
        }
        ++num_shared;
      }

      // ITF is able to fuse index batching for indices of the same space
      std::size_t num_fuseable = std::ranges::distance(
          batch_indices.begin(),
          std::ranges::partition_point(
              batch_indices, [&batch_indices](const Index &idx) {
                return idx.space() == batch_indices.front().space();
              }));

      if (num_shared >= num_fuseable &&
          num_shared == current_batch_indices_.size()) {
        // "Reuse" previously started batch loop
        if (batch_indices.size() == num_shared) {
          return;
        }
      } else {
        // Drop previous loop and start a fresh one
        end_batching();
        num_shared = 0;
      }

      append("for [");
      for (std::size_t i = num_shared; i < batch_indices.size(); ++i) {
        append(represent(batch_indices[i], ctx));
        if (i + 1 < batch_indices.size()) {
          append(", ");
        }
      }
      append_line("]:");
      indent_level_ += 1;
      current_batch_indices_.assign(batch_indices.begin(), batch_indices.end());
    } else {
      end_batching();
    }
  }

  void end_expression(const Context &) override {}

  void begin_export(const Context &) override { generated_.clear(); }

  void end_export(const Context &) override {
    end_batching();

    append_line("---- end");
  }

  std::string get_generated_code() const override { return generated_; }

 private:
  std::string generated_;
  std::size_t indent_level_ = 0;
  std::vector<Index> current_batch_indices_;

  std::string stringify(const Expr &expr, const Context &ctx) const {
    if (expr.is<Tensor>()) {
      return represent(expr.as<Tensor>(), ctx);
    } else if (expr.is<Variable>()) {
      return represent(expr.as<Variable>(), ctx);
    } else if (expr.is<Constant>()) {
      return represent(expr.as<Constant>(), ctx);
    } else if (expr.is<Power>()) {
      return represent(expr.as<Power>(), ctx);
    } else if (expr.is<Product>()) {
      const Product &product = expr.as<Product>();

      if (product.factors().size() > 2) {
        throw Exception("ITF can only handle binary contractions");
      }

      std::string repr;

      if (!product.scalar().is_identity()) {
        repr += represent(Constant(product.scalar()), ctx) + " * ";
      }

      for (std::size_t i = 0; i < product.size(); ++i) {
        repr += stringify(*product.factor(i), ctx);

        if (i + 1 < product.size()) {
          repr += " ";
        }
      }

      return repr;
    } else if (expr.is<Sum>()) {
      // TODO: Handle on-the-fly antisymmetrization (K[abij] - K[baij])
      throw Exception("ITF doesn't support explicit summation");
    }

    throw Exception("Unsupported expression type in ItfGenerator::compute");
  }

  void append(const std::string &snippet) {
    if (snippet.empty()) {
      return;
    }

    std::string indent = std::string(indent_level_, '\t');

    if (generated_.empty() || generated_.back() == '\n') {
      generated_ += indent;
    }

    std::size_t offset = 0;
    while (offset != std::string::npos) {
      std::size_t idx = snippet.find('\n', offset);
      if (idx == std::string::npos) {
        generated_.append(snippet.data() + offset,
                          snippet.data() + snippet.size());
      } else {
        idx += 1;
        generated_.append(snippet.data() + offset, snippet.data() + idx);
      }

      offset = idx;
    }
  }

  void append_line(const std::string &snippet = {}) {
    append(snippet);
    generated_ += "\n";
  }

  void end_batching() {
    if (!current_batch_indices_.empty()) {
      current_batch_indices_.clear();
      SEQUANT_ASSERT(indent_level_ > 0);
      indent_level_ = 0;
    }
  }
};

}  // namespace sequant

#endif  // SEQUANT_CORE_EXPORT_ITF_HPP
