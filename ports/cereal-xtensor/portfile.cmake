vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/cereal-xtensor
    REF v1.3.2
    SHA512 28856ce19074b9dd5156c51bd34d56574997f6b780fd64aa7a53b073bdb90b4077d571bc8c8436d13f77af4898d249a8c735cbad31080aca35a18b6da2ef888a
)

set(VCPKG_BUILD_TYPE release)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME cereal-xtensor CONFIG_PATH lib/cmake/cereal-xtensor)

# Align with falcon_core's expectation of <cereal/types/xtensor.hpp>
file(INSTALL "${SOURCE_PATH}/include/cereal-xtensor/types/xtensor.hpp" DESTINATION "${CURRENT_PACKAGES_DIR}/include/cereal/types")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
