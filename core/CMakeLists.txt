add_library(sentype-core STATIC
  src/engine.cpp
  src/letter.cpp
  src/session.cpp
  src/syllable-layout.cpp
  src/syllable-validator.cpp
  src/word-state.cpp
  src/word-state-marks.cpp)
target_include_directories(sentype-core PUBLIC include PRIVATE src)
target_compile_features(sentype-core PUBLIC cxx_std_20)
set_target_properties(sentype-core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(engine-test tests/engine-test.cpp)
target_link_libraries(engine-test PRIVATE sentype-core)
add_test(NAME core-engine COMMAND engine-test)

add_executable(session-test tests/session-test.cpp)
target_link_libraries(session-test PRIVATE sentype-core)
add_test(NAME core-session COMMAND session-test)
