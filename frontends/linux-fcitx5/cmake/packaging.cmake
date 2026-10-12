# CPack DEB: fcitx5-sentype_<version>_amd64.deb
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME fcitx5-sentype)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "SenType <nguyendk9@fptsmartcloud.vn>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SenType Vietnamese input method for Fcitx5 (preedit based)")
set(CPACK_PACKAGE_DESCRIPTION
    "Telex/VNI Vietnamese engine that composes in the client preedit and never uses backspace tricks.")
set(CPACK_DEBIAN_PACKAGE_SECTION utils)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "fcitx5")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)  # adds libfcitx5*, libstdc++ from ldd
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)  # fcitx5-sentype_<ver>_<arch>.deb
include(CPack)
