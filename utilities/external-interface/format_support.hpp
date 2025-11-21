#ifndef SEQUANT_EXTERNAL_FORMAT_SUPPORT_HPP
#define SEQUANT_EXTERNAL_FORMAT_SUPPORT_HPP

// External-interface now uses the core format support
// which provides custom format specs:
//   {:l} or {} - LaTeX format (default)
//   {:c}       - Code format (use this for code generation)
//
// All std::formatter specializations are defined in SeQuant/core/format.hpp
// and support both formats via custom format specs.

#include <SeQuant/core/format.hpp>

#endif  // SEQUANT_EXTERNAL_FORMAT_SUPPORT_HPP
