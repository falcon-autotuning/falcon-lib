vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/cereal-xtensor
    REF v1.0.0
    SHA512 f3781cbb4e9e190df38c3fe7fa80ba69bf6f9dbafb158e0426dd4604f2f1ba794450679005a38d0f9f1dad0696e2f22b8b086b2d7d08a0f99bb4fd3b0f7ed5d8
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME cereal-xtensor CONFIG_PATH lib/cmake/cereal-xtensor)

# Align with falcon_core's expectation of <cereal/types/xtensor.hpp>
file(INSTALL "${SOURCE_PATH}/include/cereal-xtensor/types/xtensor.hpp" DESTINATION "${CURRENT_PACKAGES_DIR}/include/cereal/types")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
