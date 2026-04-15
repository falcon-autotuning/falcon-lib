vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-qarray-device
    REF v${VERSION}
    SHA512 5ec4b361d2a8ca53e3bbf0b6ee82c0fb1d00c47c7667a71efca607ad6a6c2d58f041fcd8b4daa4b5e2d190d0ba715034324bbb0971ad03354b2eedf9e6f392a6
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
