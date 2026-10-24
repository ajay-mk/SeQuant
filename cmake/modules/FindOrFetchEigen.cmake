include(FetchContent)

if (NOT TARGET Eigen3::Eigen)
    set(EIGEN_BUILD_CMAKE_PACKAGE ON)
    FetchContent_Declare(
        Eigen3
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG ${SEQUANT_TRACKED_EIGEN_TAG}
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
        SYSTEM
        # eigen3 registers its build tree; do not pick up a wiped build directory.
        # Eigen 3.x and 5.x accept different version ranges; check the minimum below.
        FIND_PACKAGE_ARGS NO_CMAKE_PACKAGE_REGISTRY
    )

    FetchContent_MakeAvailable(Eigen3)

    if (Eigen3_FOUND AND Eigen3_VERSION VERSION_LESS SEQUANT_OLDEST_EIGEN_VERSION)
        message(FATAL_ERROR "Found Eigen3 ${Eigen3_VERSION} at ${Eigen3_DIR}, but SeQuant requires Eigen3 ${SEQUANT_OLDEST_EIGEN_VERSION} or later")
    endif()
endif()

# postcond check
if (NOT TARGET Eigen3::Eigen)
    message(FATAL_ERROR "FindOrFetchEigen could not make TARGET Eigen3::Eigen available")
endif()
