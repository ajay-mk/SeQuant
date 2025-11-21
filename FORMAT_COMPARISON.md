# Format Output Comparison

## External-Interface Formatters (utilities/external-interface/format_support.hpp)

**Purpose**: Code generation for external tools (ITF, Julia, etc.)
**Output Style**: Readable, code-like notation

### Examples:

```cpp
Index i(L"i_1");
// Output: "i_1"  (just the label)

Tensor t(L"H", {Index(L"i")}, {Index(L"a")});
// Output: "H[i;a;]"  (bracket notation)

Variable v(L"x");
// Output: "x"  (just the label)

Constant c(rational(3,4));
// Output: "3/4"  (the value)

Complex z(3, 4);
// Output: "(3 + 4i)"  (math notation)

Product p = 2 * t * f;
// Output: "2 H[i;a;] F[j;b;]"  (space-separated)

Sum s = expr1 + expr2 + expr3;
// Output: "expr1\n+ expr2\n+ expr3"  (newline separated)
```

## Our Implementation (SeQuant/core/format.hpp)

**Purpose**: Mathematical display via LaTeX
**Output Style**: LaTeX-formatted strings (via `to_latex()`)

### Examples:

```cpp
Index i(L"i_1");
// Output: "{i_1}"  (LaTeX subscript formatting)

Tensor t(L"H", {Index(L"i")}, {Index(L"a")});
// Output: "{H^{i}_{a}}"  (LaTeX tensor notation)

Variable v(L"x");
// Output: "{x}"  (LaTeX variable)

Constant c(rational(3,4));
// Output: "{\frac{1}{...}}"  (LaTeX fraction)

Complex z(3, 4);
// Output: "{\bigl(3 + i 4\bigr)}"  (LaTeX with \bigl)

Product p = 2 * t * f;
// Output: LaTeX product notation

Sum s = expr1 + expr2 + expr3;
// Output: LaTeX sum notation
```

## Key Differences

| Aspect | External-Interface | Core (Our Impl) |
|--------|-------------------|-----------------|
| **Purpose** | Code generation | Math display |
| **Index format** | `label()` | `to_latex()` |
| **Tensor format** | `"T[i;a;]"` | LaTeX superscript/subscript |
| **Output readability** | Code-like | LaTeX markup |
| **Target audience** | External tools | Human display/documents |

## The Conflict

Both implementations define the same `std::formatter<T>` specializations:
- `std::formatter<sequant::Tensor>`
- `std::formatter<sequant::Index>`
- `std::formatter<sequant::Expr>`
- `std::formatter<sequant::Variable>`
- etc.

**This is an ODR (One Definition Rule) violation if both headers are included!**

The program behavior is undefined if:
1. User includes `SeQuant/core/expr.hpp` (which includes `format.hpp`)
2. User also includes `utilities/external-interface/format_support.hpp`

Both define the same specializations with different implementations.
