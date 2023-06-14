#include <SeQuant/core/op.hpp>
#include <SeQuant/core/runtime.hpp>
#include <SeQuant/core/timer.hpp>
#include <SeQuant/domain/mbpt/convention.hpp>
#include <SeQuant/domain/mbpt/formalism.hpp>
#include <SeQuant/domain/mbpt/models/cc.hpp>

#include <clocale>

using namespace sequant;
using namespace sequant::mbpt::sr;

namespace {

#define runtime_assert(tf)                                         \
  if (!(tf)) {                                                     \
    std::ostringstream oss;                                        \
    oss << "failed assert at line " << __LINE__ << " in function " \
        << __func__;                                               \
    throw std::runtime_error(oss.str().c_str());                   \
  }

TimerPool<32> tpool;

// profiles evaluation of all CC equations for a given ex rank N with projection
// ex rank PMIN .. P
class compute_cceqvec {
  size_t P, PMIN, N;

 public:
  compute_cceqvec(size_t p, size_t pmin, size_t n) : P(p), PMIN(pmin), N(n) {}

  void operator()(bool print, bool screen, bool use_topology,
                  bool use_connectivity, bool canonical_only) {
    tpool.start(N);
    auto eqvec = cceqs{N, P, 1}.t(screen, use_topology, use_connectivity,
                                  canonical_only);
    tpool.stop(N);
    std::wcout << std::boolalpha << "expS" << N << "[screen=" << screen
               << ",use_topology=" << use_topology
               << ",use_connectivity=" << use_connectivity
               << ",canonical_only=" << canonical_only << "] computed in "
               << tpool.read(N) << " seconds" << std::endl;
    for (size_t R = PMIN; R <= P; ++R) {
      std::wcout << "R" << R << "(expS" << N << ") has " << eqvec[R]->size()
                 << " terms:" << std::endl;
      if (print) std::wcout << to_latex_align(eqvec[R], 20, 3) << std::endl;

      // validate known sizes of some CC residuals
      // N.B. # of equations depends on whether we use symmetric or
      // antisymmetric amplitudes
      if (mbpt::get_default_formalism().two_body_interaction() ==
          mbpt::TwoBodyInteraction::Antisymm) {
        if (R == 1 && N == 1) runtime_assert(eqvec[R]->size() == 8);
        if (R == 1 && N == 2) runtime_assert(eqvec[R]->size() == 14);
        if (R == 2 && N == 2) runtime_assert(eqvec[R]->size() == 31);
        if (R == 1 && N == 3) runtime_assert(eqvec[R]->size() == 15);
        if (R == 2 && N == 3) runtime_assert(eqvec[R]->size() == 37);
        if (R == 3 && N == 3) runtime_assert(eqvec[R]->size() == 47);
        if (R == 4 && N == 4) runtime_assert(eqvec[R]->size() == 74);
        if (R == 5 && N == 5) runtime_assert(eqvec[R]->size() == 99);
      } else {
        if (R == 1 && N == 2) runtime_assert(eqvec[R]->size() == 26);
        if (R == 2 && N == 2) runtime_assert(eqvec[R]->size() == 55);
        if (R == 1 && N == 3) runtime_assert(eqvec[R]->size() == 30);
        if (R == 2 && N == 3) runtime_assert(eqvec[R]->size() == 73);
        if (R == 3 && N == 3) runtime_assert(eqvec[R]->size() == 93);
        if (R == 4 && N == 4) runtime_assert(eqvec[R]->size() == 149);
      }
    }
  }
};  // class compute_cceqvec

// profiles evaluation of all CC equations with ex rank 2 .. N
class compute_all {
  size_t NMAX;

 public:
  compute_all(size_t nmax) : NMAX(nmax) {}

  void operator()(bool print = true, bool screen = true,
                  bool use_topology = true, bool use_connectivity = true,
                  bool canonical_only = true) {
    for (size_t N = 1; N <= NMAX; ++N)
      compute_cceqvec{N, 1, N}(print, screen, use_topology, use_connectivity,
                               canonical_only);
  }
};  // class compute_all

}  // namespace

int main(int argc, char* argv[]) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  std::wcout.precision(std::numeric_limits<double>::max_digits10);
  std::wcerr.precision(std::numeric_limits<double>::max_digits10);
  std::wcout.sync_with_stdio(false);
  std::wcerr.sync_with_stdio(false);
  std::wcout.imbue(std::locale("en_US.UTF-8"));
  std::wcerr.imbue(std::locale("en_US.UTF-8"));
  std::wcout.sync_with_stdio(true);
  std::wcerr.sync_with_stdio(true);
  sequant::detail::OpIdRegistrar op_id_registrar;
  sequant::set_default_context(
      SeQuant(Vacuum::SingleProduct, IndexSpaceMetric::Unit,
              BraKetSymmetry::conjugate, SPBasis::spinorbital));
  mbpt::set_default_convention();
  mbpt::set_default_formalism();

  TensorCanonicalizer::register_instance(
      std::make_shared<DefaultTensorCanonicalizer>());
  // set_num_threads(1);

#ifndef NDEBUG
  const size_t DEFAULT_NMAX = 3;
#else
  const size_t DEFAULT_NMAX = 4;
#endif
  const size_t NMAX = argc > 1 ? std::atoi(argv[1]) : DEFAULT_NMAX;
  // change to true to print out the resulting equations
  constexpr bool print = false;
  // change to true to print stats
  Logger::get_instance().wick_stats = false;

  ranges::for_each(std::array<bool, 2>{false, true}, [=](const bool screen) {
    ranges::for_each(
        std::array<bool, 2>{false, true}, [=](const bool use_topology) {
          ranges::for_each(std::array<bool, 2>{false, true},
                           [=](const bool canonical_only) {
                             tpool.clear();
                             // comment out to run all possible combinations
                             if (screen && use_topology && canonical_only)
                               compute_all{NMAX}(print, screen, use_topology,
                                                 true, canonical_only);
                           });
        });
  });
}
