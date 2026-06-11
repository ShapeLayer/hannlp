cmake_minimum_required(VERSION 3.16)

project(HanNLP C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(HANNANUM_BUILD_RUNTIME "Build the HanNanum command line runtime" ON)
option(HANNANUM_BUILD_TESTS "Add HanNanum parity test targets" ON)

set(HANNANUM_COMPONENT_SOURCES
  modules/hannanum/lib/hannanum/strbuf.c
  modules/hannanum/lib/hannanum/strbuf.h
  modules/hannanum/lib/hannanum/share.h
  modules/hannanum/lib/hannanum/share.c
  modules/hannanum/lib/hannanum/code.h
  modules/hannanum/lib/hannanum/code.c
  modules/hannanum/lib/hannanum/comm.h
  modules/hannanum/lib/hannanum/comm.c
  modules/hannanum/lib/hannanum/tag_set.h
  modules/hannanum/lib/hannanum/tag_set.c
  modules/hannanum/lib/hannanum/connection.h
  modules/hannanum/lib/hannanum/connection.c
  modules/hannanum/lib/hannanum/connection_not.h
  modules/hannanum/lib/hannanum/connection_not.c
  modules/hannanum/lib/hannanum/number_dic.h
  modules/hannanum/lib/hannanum/number_dic.c
  modules/hannanum/lib/hannanum/trie.h
  modules/hannanum/lib/hannanum/trie.c
  modules/hannanum/lib/hannanum/simti.h
  modules/hannanum/lib/hannanum/simti.c
  modules/hannanum/lib/hannanum/segment_position.h
  modules/hannanum/lib/hannanum/segment_position.c
  modules/hannanum/lib/hannanum/morpheme_chart.h
  modules/hannanum/lib/hannanum/morpheme_chart.c
  modules/hannanum/lib/hannanum/exp.h
  modules/hannanum/lib/hannanum/exp.c
  modules/hannanum/lib/hannanum/sentence_segmentor.h
  modules/hannanum/lib/hannanum/sentence_segmentor.c
  modules/hannanum/lib/hannanum/informal_sentence_filter.h
  modules/hannanum/lib/hannanum/informal_sentence_filter.c
  modules/hannanum/lib/hannanum/chart_morph_analyzer.h
  modules/hannanum/lib/hannanum/chart_morph_analyzer.c
  modules/hannanum/lib/hannanum/post_processor.h
  modules/hannanum/lib/hannanum/post_processor.c
  modules/hannanum/lib/hannanum/unknown_processor.h
  modules/hannanum/lib/hannanum/unknown_processor.c
  modules/hannanum/lib/hannanum/hmm_pos_tagger.h
  modules/hannanum/lib/hannanum/hmm_pos_tagger.c
  modules/hannanum/lib/hannanum/simple_pos_result.h
  modules/hannanum/lib/hannanum/simple_pos_result.c
  modules/hannanum/lib/hannanum/simple_ma_result.h
  modules/hannanum/lib/hannanum/simple_ma_result.c
  modules/hannanum/lib/hannanum/noun_extractor.h
  modules/hannanum/lib/hannanum/noun_extractor.c
  modules/hannanum/lib/hannanum/workflow.h
  modules/hannanum/lib/hannanum/workflow.c
)

add_library(hannanum STATIC
  modules/hannanum/lib/hannanum/hannanum.c
  ${HANNANUM_COMPONENT_SOURCES}
)

set_source_files_properties(${HANNANUM_COMPONENT_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

target_include_directories(hannanum
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/hannanum/lib/hannanum
)

if(HANNANUM_BUILD_RUNTIME)
  add_executable(hannanum-runtime
    modules/hannanum/main/hannanum.c
  )

  target_link_libraries(hannanum-runtime PRIVATE hannanum)
  set_target_properties(hannanum-runtime PROPERTIES OUTPUT_NAME hannanum)
endif()

if(HANNANUM_BUILD_TESTS)
  enable_testing()
  add_executable(hannanum-code-test
    tests/code_test.c
  )
  target_include_directories(hannanum-code-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules/hannanum/lib/hannanum)
  add_executable(hannanum-tagset-test
    tests/tagset_test.c
  )
  target_include_directories(hannanum-tagset-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules/hannanum/lib/hannanum)
  add_executable(hannanum-trie-test
    tests/trie_test.c
  )
  target_include_directories(hannanum-trie-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules/hannanum/lib/hannanum)
  add_executable(hannanum-simti-test
    tests/simti_test.c
  )
  target_include_directories(hannanum-simti-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules/hannanum/lib/hannanum)
  add_executable(hannanum-morpheme-chart-test
    tests/morpheme_chart_test.c
  )
  target_include_directories(hannanum-morpheme-chart-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules/hannanum/lib/hannanum)
  add_executable(hannanum-segment-position-test
    tests/segment_position_test.c
  )
  target_include_directories(hannanum-segment-position-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules/hannanum/lib/hannanum)
  add_executable(hannanum-exp-test
    tests/exp_test.c
  )
  target_include_directories(hannanum-exp-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules/hannanum/lib/hannanum)
  add_executable(hannanum-chart-adapter-test
    tests/chart_adapter_test.c
  )
  target_include_directories(hannanum-chart-adapter-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules/hannanum/lib/hannanum)
  add_test(
    NAME hannanum-code
    COMMAND hannanum-code-test
  )
  add_test(
    NAME hannanum-tagset
    COMMAND hannanum-tagset-test ${CMAKE_CURRENT_SOURCE_DIR}/../JHanNanum-0.8.4-en/JHanNanum/data
  )
  add_test(
    NAME hannanum-trie
    COMMAND hannanum-trie-test
  )
  add_test(
    NAME hannanum-simti
    COMMAND hannanum-simti-test
  )
  add_test(
    NAME hannanum-morpheme-chart
    COMMAND hannanum-morpheme-chart-test
  )
  add_test(
    NAME hannanum-segment-position
    COMMAND hannanum-segment-position-test
  )
  add_test(
    NAME hannanum-exp
    COMMAND hannanum-exp-test
  )
  add_test(
    NAME hannanum-chart-adapter
    COMMAND hannanum-chart-adapter-test ${CMAKE_CURRENT_SOURCE_DIR}/../JHanNanum-0.8.4-en/JHanNanum/data 학교 사실을 그리고
  )
  add_test(
    NAME hannanum-parity
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_hannanum_tests.sh ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()
