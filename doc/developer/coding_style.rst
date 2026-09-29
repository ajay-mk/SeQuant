Coding style
==============

This page describes naming and formatting conventions for contributors. See :doc:`documentation` for how to document
interfaces and build the documentation.

Naming
--------

Use the following conventions for ordinary C++ identifiers.

.. list-table::
   :header-rows: 1
   :widths: 35 25 40

   * - Entity
     - Convention
     - Examples
   * - Classes, structs, and enum types
     - PascalCase
     - ``IndexSpace``, ``BatchPolicy``, ``BatchScheduler``
   * - Concepts
     - snake_case
     - ``index_container``, ``cost_model``
   * - Functions, methods, namespaces, parameters, and local variables
     - snake_case
     - ``to_utf8``, ``index_space``
   * - Public data members
     - snake_case
     - ``scalar_factors``
   * - Private and protected data members, including static members
     - snake_case with a trailing underscore
     - ``tensor_strategies_``, ``index_comparer_``
   * - Enum values
     - PascalCase
     - ``ForestDescent``, ``Ordered``
   * - Type aliases and typedefs
     - PascalCase or STL-style snake_case
     - ``IndexList``, ``value_type``, ``ordinal_type``
   * - Compile-time constants
     - snake_case or UPPER_CASE
     - ``default_hash_value``, ``ID_GLOBAL``, ``DEFAULT_NMAX``

Concept names follow the standard library convention, as in ``std::same_as`` and ``std::convertible_to``.
Use a descriptive name when a short spelling would collide with a keyword, a function, or another variable:
``input_expr`` is an example.

Private and protected data members use the trailing underscore consistently, including members of template classes.
Storage that backs an accessor can also use a trailing underscore to distinguish it from the accessor. Use the same
spelling in declarations, definitions, references, and parameter documentation.

Established exceptions
------------------------

Preserve the established set-operation names ``unIon``, ``xOr``, ``add_unIon``, and ``valid_unIon``, along with
the existing ``add_union`` and ``valid_union`` aliases.

Scientific notation is part of the interface. Preserve operator names and symbols such as ``H``, ``T``,
``N_part``, ``A_maps``, and corresponding scientific variables. Scientific enum spellings also remain unchanged,
including ``Spin::alpha``, ``Spin::beta``, orbital-space tags, and ``CC::Ansatz::oT`` and ``CC::Ansatz::oU``.

STL-style traits, predicates, scope guards, cursors, and tag types retain their conventional lowercase names. Examples
include ``is_shared_ptr``, ``proportional_to``, ``tuple_less``, ``scope_exit``, and ``cursor``. Implementation helpers
that follow these conventions, such as ``impl`` and ``impl_base``, can retain those names as well.

Preserve established identifiers that begin with an underscore, including local variables, helper functions, and aliases.

Python bindings retain their established naming. Calls to shared C++ helpers follow the naming of those helpers.

``AbstractTensor`` has an established interface with leading-underscore hooks such as ``_bra`` and ``_clone``. Preserve
that interface and its overrides. Names prescribed by an external interface or framework follow that interface's rules.

Formatting
------------

The repository's ``.clang-format`` is the formatting authority. Use ``bin/admin/clang-format.sh`` to apply the
supported formatter to changed C++ lines and retain surrounding style. Keep naming changes focused on the identifiers
and the references and documentation that depend on them.

Checking names
----------------

The repository's ``.clang-tidy`` enables only ``readability-identifier-naming`` and records the established exceptions
above. The ``Naming check`` CI workflow uses LLVM 18 to report naming diagnostics for library sources, utilities,
benchmarks, and documentation examples. Naming violations, configuration errors, and parsing errors fail the job.
When the check fails, the full log is available as the ``naming-check`` workflow artifact.
Clang-tidy groups all static data members together, so the check accepts snake_case with or without a trailing
underscore for them; private and protected static members still follow the convention above.

Tests, Python bindings, and vendored sources are excluded from this initial check. Optional backend headers are checked
only when the selected build enables those backends and includes them in a checked translation unit.

For a local check, configure a non-unity build with ``CMAKE_EXPORT_COMPILE_COMMANDS=ON`` and run LLVM 18's
``run-clang-tidy`` against that build directory. For example, on Linux:

.. code-block:: console

   clang-tidy-18 --verify-config
   run-clang-tidy-18 -clang-tidy-binary clang-tidy-18 -p build/release -j 2 \
     '(^|/)(SeQuant/(?!external/)|utilities/|benchmarks/|doc/examples/)'

Review a diagnostic against the exceptions before renaming an identifier. Add an exception only for an established
scientific or interface convention, rather than to silence an ordinary naming inconsistency.
