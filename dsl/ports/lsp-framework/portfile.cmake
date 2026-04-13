vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO leon-bckl/lsp-framework
    REF 1.3.0
    SHA512 3c4cdce6c65d38e23b7bc524d1abf3ffcbc1af02a642365d948a39f4573abcffa6635caabd47f6aa2155c1796e81137bcc5a81b2229f2147a865ecb94fbf53ab
)

# Handle specific compiler requirements
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_C_COMPILER=clang
        -DCMAKE_CXX_COMPILER=clang++
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    CONFIG_PATH lib/cmake/lsp
    PACKAGE_NAME lsp
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" 
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" 
     RENAME copyright)

vcpkg_copy_pdbs()
