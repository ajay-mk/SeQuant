#include <SeQuant/core/context.hpp>
#include <SeQuant/core/expr.hpp>
#ifdef SEQUANT_HAS_TILEDARRAY
#include <SeQuant/core/eval/backends/tiledarray/eval_expr.hpp>
#endif
#ifdef SEQUANT_HAS_BTAS
#include <SeQuant/core/eval/backends/btas/eval_expr.hpp>
#endif
#ifdef SEQUANT_HAS_TAPP
#include <SeQuant/core/eval/backends/tapp/eval_expr.hpp>
#endif
#include <SeQuant/domain/mbpt/context.hpp>
#include <SeQuant/domain/mbpt/convention.hpp>
#include <SeQuant/domain/mbpt/op.hpp>
#include <SeQuant/domain/mbpt/vac_av.hpp>

#include <iostream>

int main() {
  using namespace sequant;
  using namespace sequant::mbpt;
  set_default_context({.index_basis_registry_shared_ptr = make_min_sr_spaces(),
                       .vacuum = Vacuum::SingleProduct});
  set_default_mbpt_context({.op_registry_ptr = make_legacy_registry()});

  const auto result = ref_av(H() * T(2));
  std::wcout << result.to_latex() << '\n';
  return 0;
}
