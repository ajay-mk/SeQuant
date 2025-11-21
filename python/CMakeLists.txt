project(python-sequant)

if (NOT TARGET Python::Module)
  find_package(Python 3 COMPONENTS Interpreter Development REQUIRED)
endif()

include(FindOrFetchPybind11)

if (CMAKE_CXX_STANDARD VERSION_LESS 17)
  message(FATAL_ERROR "C++ 2017 ISO Standard or higher is required to compile PySeQuant")
endif()

pybind11_add_module(python-sequant MODULE src/sequant/_sequant.cc)

target_link_libraries(python-sequant PRIVATE SeQuant)
target_include_directories(python-sequant PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(
  python-sequant
  PROPERTIES
  #PREFIX ""
  OUTPUT_NAME _sequant
  # Place the compiled module in the build directory
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

# Copy Python package files to build directory
add_custom_command(TARGET python-sequant POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/sequant
    ${CMAKE_CURRENT_BINARY_DIR}/sequant
  COMMENT "Copying Python package files to build directory"
)

# Copy the compiled extension into the package
add_custom_command(TARGET python-sequant POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:python-sequant>
    ${CMAKE_CURRENT_BINARY_DIR}/sequant/$<TARGET_FILE_NAME:python-sequant>
  COMMENT "Copying compiled extension into Python package"
)

if (BUILD_TESTING)
  add_test(sequant/unit/python/build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target python-sequant)
  set_tests_properties(sequant/unit/python/build PROPERTIES FIXTURES_SETUP PYSEQUANT_MODULE)

  set(SEQUANT_TEST_PYTHON_ENVVARS "PYTHONPATH=${PROJECT_BINARY_DIR}:$ENV{PYTHONPATH}")
  if (DEFINED SEQUANT_TEST_EXTRA_ENVVARS)
    list(APPEND SEQUANT_TEST_PYTHON_ENVVARS ${SEQUANT_TEST_EXTRA_ENVVARS})
  endif()

  # basic test
  set(test_name sequant/unit/python/basic/run)
  add_test(NAME ${test_name}
           COMMAND ${Python_EXECUTABLE} test_sequant.py
  )
  set_tests_properties(${test_name} PROPERTIES
           WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
           FIXTURES_REQUIRED PYSEQUANT_MODULE
           ENVIRONMENT "${SEQUANT_TEST_PYTHON_ENVVARS}")

  unset(SEQUANT_TEST_PYTHON_ENVVARS)

  add_dependencies(check-sequant python-sequant)

endif (BUILD_TESTING)
