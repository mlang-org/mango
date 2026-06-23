# Native installer packaging for mango (.deb, .rpm, .msi) via CPack.
# The release workflow selects the generator per platform with `cpack -G`.

set(CPACK_PACKAGE_NAME "mango")
set(CPACK_PACKAGE_VENDOR "The MLang Project")
set(CPACK_PACKAGE_CONTACT "MLang Maintainers <maintainers@mlang.org>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The package manager for the MLang language")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/mlang-org/mango")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "mango")
# WiX requires a .txt/.rtf license; the repo LICENSE has no extension, so copy it.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/LICENSE.txt" COPYONLY)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.txt")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_STRIP_FILES ON)

# Stable, descriptive artifact file names (mango_0.1.0_amd64.deb, etc.).
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

# --- Debian ----------------------------------------------------------------
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# --- RPM -------------------------------------------------------------------
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
set(CPACK_RPM_PACKAGE_URL "${CPACK_PACKAGE_HOMEPAGE_URL}")

# --- Windows (WiX -> .msi) -------------------------------------------------
set(CPACK_WIX_UPGRADE_GUID "8F3B6A2E-5C71-4B0A-9E2D-1A2B3C4D5E61")
set(CPACK_WIX_PROPERTY_ARPHELPLINK "${CPACK_PACKAGE_HOMEPAGE_URL}")

include(CPack)
