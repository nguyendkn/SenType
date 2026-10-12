add_library(sentype-addon MODULE sentype-engine.cpp)
target_link_libraries(sentype-addon PRIVATE sentype-core Fcitx5::Core Fcitx5::Config Fcitx5::Utils)
set_target_properties(sentype-addon PROPERTIES OUTPUT_NAME sentype)  # -> libsentype.so

configure_file(sentype-addon.conf.in ${CMAKE_CURRENT_BINARY_DIR}/sentype-addon.conf @ONLY)
configure_file(sentype.conf.in ${CMAKE_CURRENT_BINARY_DIR}/sentype.conf @ONLY)

install(TARGETS sentype-addon DESTINATION ${FCITX_INSTALL_ADDONDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sentype-addon.conf
        DESTINATION ${FCITX_INSTALL_PKGDATADIR}/addon RENAME sentype.conf)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sentype.conf
        DESTINATION ${FCITX_INSTALL_PKGDATADIR}/inputmethod)
