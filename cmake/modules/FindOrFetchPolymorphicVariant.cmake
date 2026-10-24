if (NOT TARGET polymorphic_variant::polymorphic_variant)
    include(FetchContent)

    FetchContent_Declare(
        polymorphic_variant
        GIT_REPOSITORY "https://github.com/Krzmbrzl/polymorphic_variant.git"
        GIT_TAG "${SEQUANT_TRACKED_POLYMORPHICVARIANT_TAG}"
        GIT_SHALLOW
        SYSTEM
        FIND_PACKAGE_ARGS ${SEQUANT_OLDEST_POLYMORPHICVARIANT_VERSION} NAMES polymorphic_variant
    )

	FetchContent_MakeAvailable(polymorphic_variant)
endif()

# postcond check
if (NOT TARGET polymorphic_variant::polymorphic_variant)
    message(FATAL_ERROR "FindOrFetchPolymorphicVariant could not make TARGET polymorphic_variant::polymorphic_variant available")
endif()

# libPerm links the unqualified target when built from source.
if (NOT TARGET polymorphic_variant)
    add_library(polymorphic_variant ALIAS polymorphic_variant::polymorphic_variant)
endif()
