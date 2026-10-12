# CPack DEB: fcitx5-sentype_<ver>-1_ubuntu26.04_<arch>.deb  (package name stays fcitx5-sentype: Debian
# convention for Fcitx5 addons, and `apt remove fcitx5-sentype` matches the file the user downloaded).
set(SENTYPE_DEB_DISTRO "ubuntu26.04" CACHE STRING "distro tag put in the .deb file name")
execute_process(COMMAND dpkg --print-architecture OUTPUT_VARIABLE _deb_arch OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
if(NOT _deb_arch)
  set(_deb_arch ${CMAKE_SYSTEM_PROCESSOR})
endif()
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME fcitx5-sentype)
set(CPACK_PACKAGE_VERSION ${SENTYPE_VERSION})
set(CPACK_DEBIAN_PACKAGE_RELEASE 1)  # -> Version: 0.1.202610120-1
set(CPACK_PACKAGE_CONTACT "SenType <nguyendk9@fptsmartcloud.vn>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SenType Vietnamese input method for Fcitx5 (preedit based)")
set(CPACK_PACKAGE_DESCRIPTION
    "Telex/VNI Vietnamese engine that composes in the client preedit and never uses backspace tricks.")
set(CPACK_DEBIAN_PACKAGE_SECTION utils)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "fcitx5")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)  # adds libfcitx5*, libstdc++ from ldd
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_FILE_NAME
    "fcitx5-sentype_${SENTYPE_VERSION}-1_${SENTYPE_DEB_DISTRO}_${_deb_arch}.deb")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_CURRENT_LIST_DIR}/../packaging/postinst)
include(CPack)
