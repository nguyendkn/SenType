# Windows TSF text service -> sentype-tsf.dll (build with MSVC or mingw-w64).
add_library(sentype-win-keys STATIC src/key-translation.cpp)
target_include_directories(sentype-win-keys PUBLIC src)
target_link_libraries(sentype-win-keys PUBLIC sentype-core)

add_library(sentype-tsf SHARED
  src/dll-main.cpp src/text-service.cpp src/composition.cpp src/registration.cpp src/settings.cpp
  sentype-tsf.def)
set_target_properties(sentype-tsf PROPERTIES PREFIX "")  # sentype-tsf.dll, not libsentype-tsf.dll
target_compile_definitions(sentype-tsf PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries(sentype-tsf PRIVATE sentype-win-keys ole32 oleaut32 uuid advapi32 user32)
if(MINGW)  # self-contained DLL: no libstdc++/libgcc/winpthread runtime DLLs to ship
  target_link_options(sentype-tsf PRIVATE -static -static-libgcc -static-libstdc++ -Wl,--kill-at)
endif()

add_executable(key-translation-test tests/key-translation-test.cpp)
target_link_libraries(key-translation-test PRIVATE sentype-win-keys)
if(MINGW)
  target_link_options(key-translation-test PRIVATE -static)
endif()
add_test(NAME windows-key-translation COMMAND key-translation-test)
