#include <SeQuant/core/export/context.hpp>
#include <SeQuant/core/utility/exception.hpp>

#include <algorithm>
#include <type_traits>

namespace sequant {

ZeroStrategy operator|(ZeroStrategy lhs, ZeroStrategy rhs) {
  using underlying_type = std::underlying_type_t<ZeroStrategy>;
  return static_cast<ZeroStrategy>(static_cast<underlying_type>(lhs) |
                                   static_cast<underlying_type>(rhs));
}

ZeroStrategy operator&(ZeroStrategy lhs, ZeroStrategy rhs) {
  using underlying_type = std::underlying_type_t<ZeroStrategy>;
  return static_cast<ZeroStrategy>(static_cast<underlying_type>(lhs) &
                                   static_cast<underlying_type>(rhs));
}

bool operator!(ZeroStrategy strategy) {
  return !static_cast<std::underlying_type_t<ZeroStrategy>>(strategy);
}

StrategyPair::StrategyPair(LoadStrategy load, ZeroStrategy zero)
    : load(load), zero(zero) {}

StrategyPair::StrategyPair(LoadStrategy load) : StrategyPair() {
  this->load = load;
}

StrategyPair::StrategyPair(ZeroStrategy zero) : StrategyPair() {
  this->zero = zero;
}

UsageSet &UsageSet::operator=(Usage usage) {
  val_ = static_cast<Value>(usage);
  return *this;
}

UsageSet &UsageSet::operator|=(UsageSet usage) {
  val_ |= usage.val_;
  return *this;
}

UsageSet &UsageSet::operator&=(UsageSet usage) {
  val_ &= usage.val_;
  return *this;
}

UsageSet &UsageSet::operator|=(Usage usage) {
  val_ |= static_cast<Value>(usage);
  return *this;
}

UsageSet &UsageSet::operator&=(Usage usage) {
  val_ &= static_cast<Value>(usage);
  return *this;
}

bool operator&(UsageSet set, Usage usage) {
  return set.val_ & static_cast<UsageSet::Value>(usage);
}

bool operator&(Usage usage, UsageSet set) { return set & usage; }

bool operator==(UsageSet set, Usage usage) {
  return set.val_ == static_cast<UsageSet::Value>(usage);
}

bool operator==(Usage usage, UsageSet set) { return set == usage; }

bool operator!=(UsageSet set, Usage usage) { return !(set == usage); }

bool operator!=(Usage usage, UsageSet set) { return set != usage; }

ExportContext::ExportContext(TensorStrategyMap tensor_map,
                             VariableStrategyMap variable_map)
    : tensor_strategies_({{ID_GLOBAL, std::move(tensor_map)}}),
      variable_strategies_({{ID_GLOBAL, std::move(variable_map)}}) {}

ExportContext::ExportContext(VariableStrategyMap map)
    : variable_strategies_({{ID_GLOBAL, std::move(map)}}) {}

ExportContext::~ExportContext() = default;

LoadStrategy ExportContext::load_strategy(const Tensor &tensor) const {
  if (auto map_iter = tensor_strategies_.find(ID_GLOBAL);
      map_iter != tensor_strategies_.end()) {
    auto iter = map_iter->second.find(tensor);
    if (iter != map_iter->second.end()) {
      return iter->second.load;
    }
  }

  if (has_current_expression_id()) {
    if (auto map_iter = tensor_strategies_.find(current_expression_id());
        map_iter != tensor_strategies_.end()) {
      auto iter = map_iter->second.find(tensor);
      if (iter != map_iter->second.end()) {
        return iter->second.load;
      }
    }
  }

  return LoadStrategy::Create;
}

LoadStrategy ExportContext::load_strategy(const Variable &variable) const {
  if (auto map_iter = variable_strategies_.find(ID_GLOBAL);
      map_iter != variable_strategies_.end()) {
    auto iter = map_iter->second.find(variable);
    if (iter != map_iter->second.end()) {
      return iter->second.load;
    }
  }

  if (has_current_expression_id()) {
    if (auto map_iter = variable_strategies_.find(current_expression_id());
        map_iter != variable_strategies_.end()) {
      auto iter = map_iter->second.find(variable);
      if (iter != map_iter->second.end()) {
        return iter->second.load;
      }
    }
  }

  return LoadStrategy::Create;
}

void ExportContext::set_load_strategy(
    const Tensor &tensor, LoadStrategy strategy,
    const std::optional<std::size_t> &expression_id) {
  std::size_t id = expression_id.value_or(
      has_current_expression_id() ? current_expression_id() : ID_GLOBAL);

  auto iter = tensor_strategies_[id].find(tensor);

  if (iter == tensor_strategies_[id].end()) {
    tensor_strategies_[id][tensor] = StrategyPair(strategy);
  } else {
    iter->second.load = strategy;
  }
}

void ExportContext::set_load_strategy(
    const Variable &variable, LoadStrategy strategy,
    const std::optional<std::size_t> &expression_id) {
  std::size_t id = expression_id.value_or(
      has_current_expression_id() ? current_expression_id() : ID_GLOBAL);

  auto iter = variable_strategies_[id].find(variable);

  if (iter == variable_strategies_[id].end()) {
    variable_strategies_[id][variable] = StrategyPair(strategy);
  } else {
    iter->second.load = strategy;
  }
}

ZeroStrategy ExportContext::zero_strategy(const Tensor &tensor) const {
  if (auto map_iter = tensor_strategies_.find(ID_GLOBAL);
      map_iter != tensor_strategies_.end()) {
    auto iter = map_iter->second.find(tensor);
    if (iter != map_iter->second.end()) {
      return iter->second.zero;
    }
  }

  if (has_current_expression_id()) {
    if (auto map_iter = tensor_strategies_.find(current_expression_id());
        map_iter != tensor_strategies_.end()) {
      auto iter = map_iter->second.find(tensor);
      if (iter != map_iter->second.end()) {
        return iter->second.zero;
      }
    }
  }

  return ZeroStrategy::ZeroOnCreate;
}

ZeroStrategy ExportContext::zero_strategy(const Variable &variable) const {
  if (auto map_iter = variable_strategies_.find(ID_GLOBAL);
      map_iter != variable_strategies_.end()) {
    auto iter = map_iter->second.find(variable);
    if (iter != map_iter->second.end()) {
      return iter->second.zero;
    }
  }

  if (has_current_expression_id()) {
    if (auto map_iter = variable_strategies_.find(current_expression_id());
        map_iter != variable_strategies_.end()) {
      auto iter = map_iter->second.find(variable);
      if (iter != map_iter->second.end()) {
        return iter->second.zero;
      }
    }
  }

  return ZeroStrategy::ZeroOnCreate;
}

void ExportContext::set_zero_strategy(
    const Tensor &tensor, ZeroStrategy strategy,
    const std::optional<std::size_t> &expression_id) {
  std::size_t id = expression_id.value_or(
      has_current_expression_id() ? current_expression_id() : ID_GLOBAL);

  auto iter = tensor_strategies_[id].find(tensor);

  if (iter == tensor_strategies_[id].end()) {
    tensor_strategies_[id][tensor] = StrategyPair(strategy);
  } else {
    iter->second.zero = strategy;
  }
}

void ExportContext::set_zero_strategy(
    const Variable &variable, ZeroStrategy strategy,
    const std::optional<std::size_t> &expression_id) {
  std::size_t id = expression_id.value_or(
      has_current_expression_id() ? current_expression_id() : ID_GLOBAL);

  auto iter = variable_strategies_[id].find(variable);

  if (iter == variable_strategies_[id].end()) {
    variable_strategies_[id][variable] = StrategyPair(strategy);
  } else {
    iter->second.zero = strategy;
  }
}

bool ExportContext::rewrite(Tensor &) const { return false; }

bool ExportContext::rewrite(Variable &) const { return false; }

bool ExportContext::inside_named_section() const {
  return current_section_.has_value();
}

const std::string &ExportContext::current_section_name() const {
  return current_section_.value();
}

void ExportContext::set_current_section_name(std::string name) {
  current_section_ = std::move(name);
}

void ExportContext::clear_current_section_name() { current_section_.reset(); }

bool ExportContext::has_current_expression_id() const {
  return current_expression_id_.has_value();
}

std::size_t ExportContext::current_expression_id() const {
  return current_expression_id_.value();
}

void ExportContext::set_current_expression_id(std::size_t id) {
  current_expression_id_ = id;
}

void ExportContext::clear_current_expression_id() {
  current_expression_id_.reset();
}

std::span<const Index> ExportContext::batch_indices(
    std::optional<std::size_t>) const {
  return {};
}

void ExportContext::set_batch_indices(std::span<const Index>,
                                      std::optional<std::size_t>) {
  throw Exception(
      "The generator this context belongs to does not support index batching");
}

bool ExportContext::is_batched(std::optional<std::size_t> id) const {
  return !batch_indices(std::move(id)).empty();
}

bool ExportContext::is_index_batched(const Index &idx,
                                     std::optional<std::size_t> id) const {
  const auto &indices = batch_indices(std::move(id));

  return std::ranges::find(indices, idx) != indices.end();
}

}  // namespace sequant
