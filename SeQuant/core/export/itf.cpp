#include <SeQuant/core/export/itf.hpp>
#include <SeQuant/core/expr.hpp>
#include <SeQuant/core/space.hpp>
#include <SeQuant/core/utility/macros.hpp>
#include <SeQuant/core/utility/string.hpp>

#include <algorithm>
#include <cctype>
#include <optional>
#include <span>
#include <string>

namespace sequant {

std::string ItfContext::index_name(const IndexSpace &space,
                                   std::size_t ordinal) const {
  SEQUANT_ASSERT(ordinal >= idx_offset_);
  ordinal -= idx_offset_;

  std::string base_key = to_utf8(space.base_key());

  char limit = [&]() -> char {
    auto it = index_label_limits_.find(space);
    if (it == index_label_limits_.end()) {
      // Note that in between capital and lowercase letters there are symbols
      // [,\,],^,_ and `. Hence, this default is not quite safe but should work
      // in most cases anyway
      return 'z';
    }

    return it->second;
  }();

  if (base_key.empty() || base_key.size() > 1 ||
      base_key[0] + ordinal > limit) {
    return base_key + std::to_string(ordinal);
  }

  // Merge base key and ordinal into a single letter. That is
  // i1 -> i
  // i2 -> j
  // i3 -> k
  // etc.
  char target = base_key[0] + ordinal;

  return std::string(1, target);
}

std::string ItfContext::get_name(const IndexSpace &space) const {
  auto it = space_names_.find(space);

  if (it == space_names_.end()) {
    throw Exception("No name known for index space '" +
                    to_utf8(space.base_key()) + "'");
  }

  return it->second;
}

std::string ItfContext::get_tag(const IndexSpace &space) const {
  auto it = tags_.find(space);

  if (it == tags_.end()) {
    throw Exception("No tag known for index space '" +
                    to_utf8(space.base_key()) + "'");
  }

  return it->second;
}

std::optional<std::string> ItfContext::import_name(const Tensor &tensor) const {
  auto it = tensor_imports_.find(tensor);

  if (it == tensor_imports_.end()) {
    return {};
  }

  return it->second;
}

std::optional<std::string> ItfContext::import_name(
    const Variable &variable) const {
  auto it = variable_imports_.find(variable);

  if (it == variable_imports_.end()) {
    return {};
  }

  return it->second;
}

void ItfContext::set_name(const IndexSpace &space, std::string name) {
  space_names_[space] = std::move(name);
}

void ItfContext::set_tag(const IndexSpace &space, std::string tag) {
  tags_[space] = std::move(tag);
}

void ItfContext::set_import_name(const Tensor &tensor, std::string name) {
  tensor_imports_[tensor] = std::move(name);
}

void ItfContext::set_import_name(const Variable &variable, std::string name) {
  variable_imports_[variable] = std::move(name);
}

bool ItfContext::rewrite(Tensor &tensor) const {
  bool modified = false;

  auto num_indices = std::distance(tensor.const_indices().begin(),
                                   tensor.const_indices().end());

  if (tensor.label() == integral_label_ && num_indices == 4 &&
      tensor.bra_rank() == 2 && tensor.ket_rank() == 2) {
    // These are the 2-electron integrals. They posses intra-particle braket
    // symmetry, that is in g^{pq}_{rs} we can exchange p with r, without also
    // swapping q with s. This symmetry can currently not be represented within
    // SeQuant, so we have to use this hacky approach in order to make use of
    // it.

    container::svector<Index> bra = tensor.bra();
    container::svector<Index> ket = tensor.ket();

    for (std::size_t i = 0; i < 2; ++i) {
      if (needs_swap(bra.at(i).space(), ket.at(i).space())) {
        std::swap(bra[i], ket[i]);
      }
    }

    bool swap_cols = false;
    if (bra[0].space() != bra[1].space()) {
      swap_cols = needs_swap(bra[0].space(), bra[1].space());
    } else if (ket[0].space() != ket[1].space()) {
      swap_cols = needs_swap(ket[0].space(), ket[1].space());
    }

    if (swap_cols) {
      std::swap(bra[0], bra[1]);
      std::swap(ket[0], ket[1]);
    }

    std::wstring label;

    // Look at the index space patterns to figure out whether
    // this is a K or a J integral. If the previously attempted sorting
    // of index spaces can be improved by switching the second and third
    // index, do that and thereby produce a J tensor. Otherwise, we retain
    // the index sequence as-is and thereby produce a K tensor.
    // There are some explicit exceptions for J-tensors though.
    Index *p1_1 = nullptr;
    Index *p1_2 = nullptr;
    Index *p2_1 = nullptr;
    Index *p2_2 = nullptr;
    if (is_exceptional_J(bra, ket) ||
        needs_swap(bra[1].space(), ket[0].space())) {
      label = L"J";
      std::swap(bra[1], ket[0]);

      p1_1 = &bra[0];
      p2_1 = &ket[0];
      p1_2 = &bra[1];
      p2_2 = &ket[1];
    } else {
      label = L"K";

      p1_1 = &bra[0];
      p2_1 = &bra[1];
      p1_2 = &ket[0];
      p2_2 = &ket[1];
    }

    // Go through the symmetries again to try and produce the most canonical
    // index ordering possible without breaking the index-space ordering
    // established up to this point.
    // This is a purely cosmetic change, but it is very useful for testing
    // purposes to have  unique representation of the integrals.
    if (p1_1->space() == p1_2->space() && *p1_2 < *p1_1) {
      std::swap(*p1_1, *p1_2);
    }
    if (p2_1->space() == p2_2->space() && *p2_2 < *p2_1) {
      std::swap(*p2_1, *p2_2);
    }
    if (p1_1->space() == p2_1->space() && p1_2->space() == p2_2->space()) {
      if (*p2_1 < *p1_1 || (*p1_1 == *p2_1 && *p2_2 < *p1_2)) {
        std::swap(*p1_1, *p2_1);
        std::swap(*p1_2, *p2_2);
      }
    }

    tensor =
        Tensor(std::move(label), sequant::bra(std::move(bra)),
               sequant::ket(std::move(ket)), tensor.aux(), tensor.symmetry(),
               tensor.braket_symmetry(), tensor.column_symmetry());

    modified = true;
  }

  modified |= ReorderingContext::rewrite(tensor);

  return modified;
}

bool ItfContext::is_exceptional_J(std::span<Index> bra,
                                  std::span<Index> ket) const {
  SEQUANT_ASSERT(bra.size() == 2);
  SEQUANT_ASSERT(ket.size() == 2);

  // integrals with 3 external (virtual) indices ought to be converted to
  // J-integrals
  // Here, we generalize this to all integrals for which the bra indices and
  // the first ket index are of the same space, the ket indices are of different
  // spaces and the additional ket space compares less than the space of the
  // bras.
  bool bras_are_same = bra[0].space() == bra[1].space();
  bool kets_are_different = ket[0].space() != ket[1].space();
  bool kets_are_ordered = is_ordered(ket[0].space(), ket[1].space());
  bool first_ket_same_as_bra = ket[0].space() == bra[0].space();

  return bras_are_same && kets_are_different && kets_are_ordered &&
         first_ket_same_as_bra;
}

std::size_t ItfContext::index_id_offset() const { return idx_offset_; }

void ItfContext::set_index_id_offset(std::size_t offset) {
  idx_offset_ = offset;
}

void ItfContext::set_batch_indices(std::span<const Index> indices,
                                   std::optional<std::size_t> id) {
  auto it = batch_indices_.find(id.value_or(ID_GLOBAL));

  if (it == batch_indices_.end()) {
    std::vector<Index> copy(indices.begin(), indices.end());
    it = batch_indices_.emplace(id.value_or(ID_GLOBAL), std::move(copy)).first;
  } else {
    it->second.assign(indices.begin(), indices.end());
  }

  // ITF can parallelize over the first index so make sure this is as large
  // as possible
  std::ranges::stable_sort(it->second, std::greater<>{}, [](const Index &idx) {
    return idx.space().approximate_size();
  });
}

std::span<const Index> ItfContext::batch_indices(
    std::optional<std::size_t> id) const {
  auto it = batch_indices_.find(id.value_or(ID_GLOBAL));

  if (it == batch_indices_.end() && id.has_value()) {
    it = batch_indices_.find(ID_GLOBAL);
  }

  if (it == batch_indices_.end()) {
    return {};
  }

  return it->second;
}

}  // namespace sequant
