vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-typing
    REF v${VERSION}
    SHA512 9be44e054f334acefe140d13d893694c9fd981e4b906c91b52370e5a35b3dcd045bdee851608918d60b2859438caaf397c12032b606d19f4e603dc7ea7e8481d
)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()
file(INSTALL "${SOURCE_PATH}/LICENSE"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)
vcpkg_copy_pdbs()
