vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-routine
    REF v${VERSION}
    SHA512 24a80d5432920d5d3e3807d3fa765d886002c88374e798d5e42c6ebcba1e040b2f7bb267b5b29c171a29f218c5387d10be1bde9add4dd059d979670c7ba217f2
)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS -DBUILD_TESTS=OFF
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()
file(INSTALL "${SOURCE_PATH}/LICENSE"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)
vcpkg_copy_pdbs()
