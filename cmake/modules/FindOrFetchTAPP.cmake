# try find_package
if (NOT TARGET tapp::reference)
  include(FindPackageRegimport)
  find_package_regimport(tapp QUIET CONFIG COMPONENTS reference)
  if (TARGET tapp::reference)
    message(STATUS "Found tapp CONFIG at ${tapp_CONFIG}")
  endif()
endif()

# if not found, build via FetchContent
if (NOT TARGET tapp::reference)
  include(FetchContent)
  FetchContent_Declare(
      tapp
      GIT_REPOSITORY      https://github.com/TAPPorg/reference-implementation.git
      GIT_TAG             ${SEQUANT_TRACKED_TAPP_TAG}
      EXCLUDE_FROM_ALL
      SYSTEM
  )
  FetchContent_MakeAvailable(tapp)

endif()

# postcond check
if (NOT TARGET tapp::reference)
  message(FATAL_ERROR "FindOrFetchTAPP could not make tapp::reference target available")
endif()
