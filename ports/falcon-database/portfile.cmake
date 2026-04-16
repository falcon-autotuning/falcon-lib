vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-database
    REF v${VERSION}
    SHA512 8011515560cd839399845f3336573cc06b49b6f5963e57b2e6603273358a313ff46f0be36316c62c49ebfb7995589874f629d7880f09cb3455f65aa3a39c3caa
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
vcpkg_copy_tools(TOOL_NAMES falcon-db-cli AUTO_CLEAN)
vcpkg_copy_pdbs()
