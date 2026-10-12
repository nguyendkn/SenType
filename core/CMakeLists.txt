add_library(sentype-core STATIC
  src/engine.cpp
  src/letter.cpp
  src/session.cpp
  src/settings.cpp
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

# Real-data syllable test: needs the corpus from tools/fetch-test-data.sh (skipped when absent).
set(SENTYPE_TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/build/test-data" CACHE PATH "fetched test corpora")
add_executable(syllable-corpus-test tests/syllable-corpus-test.cpp
  tests/support/vi-units.cpp tests/support/typing-orders.cpp)
target_link_libraries(syllable-corpus-test PRIVATE sentype-core)
target_include_directories(syllable-corpus-test PRIVATE src tests)
add_test(NAME core-syllable-corpus
  COMMAND syllable-corpus-test ${SENTYPE_TEST_DATA_DIR}/vi-syllables.txt
          ${SENTYPE_TEST_DATA_DIR}/english-words.txt
          ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/non-native-syllables.txt
          ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/english-must-restore.txt)
set_tests_properties(core-syllable-corpus PROPERTIES SKIP_RETURN_CODE 77)

# Property / fuzz tests. Same invariants in a deterministic ctest and a libFuzzer target.
add_library(sentype-test-support STATIC tests/support/vi-units.cpp tests/support/typing-orders.cpp
  tests/fuzz/fuzz-invariants.cpp)
target_link_libraries(sentype-test-support PUBLIC sentype-core)
target_include_directories(sentype-test-support PUBLIC tests tests/fuzz)

add_executable(fuzz-random-test tests/fuzz/fuzz-random-test.cpp)
target_link_libraries(fuzz-random-test PRIVATE sentype-test-support)
add_test(NAME core-fuzz-random COMMAND fuzz-random-test 200000 1)  # always on: quick smoke
add_test(NAME core-fuzz-random-corpus
  COMMAND fuzz-random-test 200000 2 ${SENTYPE_TEST_DATA_DIR}/vi-syllables.txt)
set_tests_properties(core-fuzz-random-corpus PROPERTIES SKIP_RETURN_CODE 77)

if(SENTYPE_FUZZ)
  add_executable(sentype-fuzz tests/fuzz/fuzz-target.cpp)
  target_link_libraries(sentype-fuzz PRIVATE sentype-test-support)
  target_link_options(sentype-fuzz PRIVATE -fsanitize=fuzzer)
  add_test(NAME core-fuzz-smoke  # 30k libFuzzer executions from the seed corpus
    COMMAND sentype-fuzz -runs=30000 -seed=1 -max_len=128 ${CMAKE_CURRENT_SOURCE_DIR}/tests/fuzz/corpus)
endif()
