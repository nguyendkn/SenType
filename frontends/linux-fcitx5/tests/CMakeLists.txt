# Real Fcitx5 instance + test frontend; data dir mimics /usr/share/fcitx5.
find_package(Fcitx5ModuleTestFrontend REQUIRED)
set(DATA ${CMAKE_CURRENT_BINARY_DIR}/fcitx-data)
file(MAKE_DIRECTORY ${DATA}/addon ${DATA}/inputmethod)
file(WRITE ${DATA}/addon/testfrontend.conf
     "[Addon]\nName=testfrontend\nType=SharedLibrary\nLibrary=libtestfrontend\nCategory=Frontend\n")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../addon/sentype-addon.conf.in ${DATA}/addon/sentype.conf @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../addon/sentype.conf.in ${DATA}/inputmethod/sentype.conf @ONLY)

add_executable(addon-integration-test addon-integration-test.cpp)
target_link_libraries(addon-integration-test PRIVATE Fcitx5::Core Fcitx5::Utils Fcitx5::Module::TestFrontend)
target_compile_definitions(addon-integration-test PRIVATE
  SENTYPE_TEST_BINARY_DIR="${CMAKE_BINARY_DIR}/frontends/linux-fcitx5"
  SENTYPE_FCITX_ADDON_DIR="${FCITX_INSTALL_ADDONDIR}")
add_dependencies(addon-integration-test sentype-addon)
add_test(NAME addon-integration COMMAND addon-integration-test)
