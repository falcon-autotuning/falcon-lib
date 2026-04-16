vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-dsl
    REF v${VERSION}
    SHA512 6a7b407ed2c21a5ae74adbd1cb6e2414baf7008b6b42553cb6e3b4079b511f60cf65354886a0b17f121d8c8a0496edd57f75be4e8821762004b69ec2f90a280d
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
vcpkg_copy_tools(TOOL_NAMES falcon-run falcon-test AUTO_CLEAN)
vcpkg_copy_pdbs()
