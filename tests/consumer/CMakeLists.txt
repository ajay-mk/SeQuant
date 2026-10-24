# Standalone project that consumes an installed SeQuant via find_package;
# not part of the SeQuant build, see the consumer step in .github/workflows/cmake.yml
cmake_minimum_required(VERSION 3.28)
project(sequant-consumer CXX)

find_package(SeQuant CONFIG REQUIRED)

if (DEFINED EXPECTED_BACKENDS)
  foreach (backend ta btas tapp)
    if (backend IN_LIST EXPECTED_BACKENDS)
      if (NOT TARGET SeQuant::eval::${backend})
        message(FATAL_ERROR "Expected SeQuant::eval::${backend} in the package")
      endif()
    elseif (TARGET SeQuant::eval::${backend})
      message(FATAL_ERROR "Unexpected SeQuant::eval::${backend} in the package")
    endif()
  endforeach()
endif()

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE SeQuant::SeQuant)
