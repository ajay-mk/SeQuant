#include "utils.hpp"

#include <SeQuant/core/expr.hpp>
#include <SeQuant/core/index.hpp>
#include <SeQuant/core/space.hpp>
#include <SeQuant/core/utility/exception.hpp>
#include <SeQuant/core/utility/expr.hpp>
#include <SeQuant/core/utility/indices.hpp>
#include <SeQuant/core/utility/macros.hpp>
#include <SeQuant/core/utility/string.hpp>

#include <algorithm>
#include <optional>

#include <range/v3/view/zip.hpp>

using namespace sequant;

std::size_t IndexSpaceMeta::get_size(const IndexSpace &space) const {
  return space.approximate_size();
}

std::size_t IndexSpaceMeta::get_size(const Index &index) const {
  return get_size(index.space());
}

std::string IndexSpaceMeta::get_name(const IndexSpace &space) const {
  auto iter = entries_.find(space);
  if (iter == entries_.end()) {
    throw Exception("No known name for index space " +
                    to_utf8(space.base_key()));
  }

  return iter->second.name;
}

std::string IndexSpaceMeta::get_tag(const IndexSpace &space) const {
  auto iter = entries_.find(space);
  if (iter == entries_.end()) {
    throw Exception("No known tag for index space " +
                    to_utf8(space.base_key()));
  }

  return iter->second.tag;
}

void IndexSpaceMeta::register_space(IndexSpace space, Entry entry) {
  entries_.insert({std::move(space), std::move(entry)});
}

container::svector<container::svector<Index> > get_external_index_pairs(
    const Tensor &tensor) {
  container::svector<container::svector<Index> > external_pairs;

  auto zipped = ranges::views::zip(tensor.bra(), tensor.ket());

  for (const auto &current : zipped) {
    external_pairs.push_back({current.first, current.second});
  }

  for (std::size_t i = zipped.size(); i < tensor.bra_rank(); ++i) {
    external_pairs.push_back({tensor.bra()[i]});
  }

  for (std::size_t i = zipped.size(); i < tensor.ket_rank(); ++i) {
    external_pairs.push_back({tensor.ket()[i]});
  }

  for (const Index &current : tensor.aux()) {
    external_pairs.push_back({current});
  }

  return external_pairs;
}

container::svector<container::svector<Index> > get_external_index_pairs(
    const ExprPtr &expression) {
  if (expression->is<Tensor>()) {
    return get_external_index_pairs(expression->as<Tensor>());
  }

  if (expression->is<Sum>()) {
    // External indices are expected to be the same across summands
    return get_external_index_pairs(expression->as<Sum>().summand(0));
  }

  const Product &product = expression->as<Product>();

  if (product.at(0)->is<Tensor>() &&
      (product.at(0).as<Tensor>().label() == reserved::antisymm_label() ||
       product.at(0).as<Tensor>().label() == reserved::symm_label())) {
    // Special case in the presence of an (anti)symmetrizer operator
    return get_external_index_pairs(product.at(0).as<Tensor>());
  }

  IndexGroups groups = get_unique_indices(expression);
  container::svector<container::svector<Index> > external_pairs;

  auto visitor = [&groups, &external_pairs](const ExprPtr &expr) {
    SEQUANT_ASSERT(expr.is<Tensor>());

    const Tensor &tensor = expr.as<Tensor>();

    for (std::size_t i = 0; i < std::min(tensor.bra_rank(), tensor.ket_rank());
         ++i) {
      const Index &bra_idx = tensor.bra()[i];
      const Index &ket_idx = tensor.ket()[i];

      const bool bra_is_external =
          std::find(groups.bra.begin(), groups.bra.end(), bra_idx) !=
          groups.bra.end();
      const bool ket_is_external =
          std::find(groups.ket.begin(), groups.ket.end(), ket_idx) !=
          groups.ket.end();

      if (bra_is_external && ket_is_external) {
        // Bra and ket index are paired -> keep them paired in external index
        // list
        external_pairs.push_back({bra_idx, ket_idx});
      } else if (bra_is_external) {
        external_pairs.push_back({bra_idx});
      } else if (ket_is_external) {
        external_pairs.push_back({ket_idx});
      }
    }
  };

  expression->visit(visitor, true);

  for (Index &current : groups.aux) {
    external_pairs.push_back({std::move(current)});
  }

  return external_pairs;
}

bool needs_symmetrization(const sequant::ExprPtr &expression) {
  bool contains_symmetrizer = false;
  // Note: the assumption is that we never encounter cases where only part of
  // the expression requires symmetrization
  expression->visit(
      [&contains_symmetrizer](const ExprPtr &expr) {
        // Right now symmetrizer operators are represented as tensor objects
        // with name "S"
        if (expr.is<Tensor>() &&
            expr.as<Tensor>().label() == reserved::symm_label()) {
          contains_symmetrizer = true;
        }
      },
      true);

  return contains_symmetrizer;
}

sequant::ExprPtr generate_result_symmetrization(
    const ResultExpr &result, std::wstring_view precursor_name) {
  IndexGroups<std::vector<Index> > externals;

  for (const auto &[bra, ket] :
       result.index_particle_grouping<std::pair<Index, Index> >()) {
    externals.bra.push_back(bra);
    externals.ket.push_back(ket);
  }

  return generate_result_symmetrization(precursor_name, externals,
                                        result.result_as_tensor());
}

sequant::ExprPtr generate_result_symmetrization(
    const Tensor &result, std::wstring_view precursor_name) {
  SEQUANT_ASSERT(result.bra_rank() == result.ket_rank());

  IndexGroups<std::vector<Index> > externals;

  for (const auto [bra, ket] : ranges::views::zip(result.bra(), result.ket())) {
    externals.bra.push_back(bra);
    externals.ket.push_back(ket);
  }

  return generate_result_symmetrization(precursor_name, externals, result);
}

sequant::ExprPtr generate_result_symmetrization(
    std::wstring_view precursor_name,
    const sequant::IndexGroups<std::vector<sequant::Index> > &externals,
    const Tensor &ref) {
  SEQUANT_ASSERT(externals.bra.size() == externals.ket.size());

  // Note: we're only symmetrizing over bra-ket, not over auxiliary indices
  ExprPtr symmetrization = ex<Sum>(ExprPtrList{});
  for (std::size_t i = 0; i < externals.bra.size(); ++i) {
    std::vector<Index> sym_bra;
    std::vector<Index> sym_ket;

    for (std::size_t j = 0; j < externals.ket.size(); ++j) {
      sym_bra.push_back(externals.bra[(i + j) % externals.bra.size()]);
      sym_ket.push_back(externals.ket[(i + j) % externals.ket.size()]);
    }

    symmetrization +=
        rational(1, externals.bra.size()) *
        ex<Tensor>(precursor_name, bra(std::move(sym_bra)),
                   ket(std::move(sym_ket)), aux(externals.aux), ref.symmetry(),
                   ref.braket_symmetry(), ref.column_symmetry());
  }

  return symmetrization;
}
