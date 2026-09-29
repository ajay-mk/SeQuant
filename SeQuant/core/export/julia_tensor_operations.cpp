#include <SeQuant/core/export/julia_tensor_operations.hpp>
#include <SeQuant/core/expr.hpp>
#include <SeQuant/core/index.hpp>
#include <SeQuant/core/space.hpp>
#include <SeQuant/core/utility/string.hpp>

namespace sequant {

JuliaTensorOperationsGeneratorContext::JuliaTensorOperationsGeneratorContext(
    TagMap index_tags, DimMap index_dims)
    : ReorderingContext(MemoryLayout::ColumnMajor),
      index_tags_(std::move(index_tags)),
      index_dims_(std::move(index_dims)) {}

std::string JuliaTensorOperationsGeneratorContext::get_tag(
    const IndexSpace &space) const {
  auto it = index_tags_.find(space);

  if (it == index_tags_.end()) {
    throw Exception("No known tags for indices of space \"" +
                    to_utf8(space.base_key()) + "\"");
  }

  return it->second;
}

std::string JuliaTensorOperationsGeneratorContext::get_dim(
    const IndexSpace &space) const {
  auto it = index_dims_.find(space);

  if (it == index_dims_.end()) {
    // Auto-generate a variable name for the space's dimension
    return "dim_" + get_tag(space);
  }

  return it->second;
}

std::string JuliaTensorOperationsGeneratorContext::get_tags(
    const Tensor &tensor) const {
  std::string tags;

  for (const Index &idx : tensor.const_indices()) {
    tags += get_tag(idx.space());
  }

  return tags;
}

void JuliaTensorOperationsGeneratorContext::set_tag(const IndexSpace &space,
                                                    std::string tag) {
  index_tags_[space] = std::move(tag);
}

void JuliaTensorOperationsGeneratorContext::set_dim(const IndexSpace &space,
                                                    std::string dim) {
  index_dims_[space] = std::move(dim);
}

}  // namespace sequant
