PROJECT(simple.base_test)

SET(TARGET_NAME simple_base_test)

file(GLOB_RECURSE util_sources 
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cc 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
list(APPEND test_sources ${util_sources})

add_executable(${TARGET_NAME}
    ${test_sources}
)

target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/
    ${GTEST_INCLUDE_DIR}
    ${CMAKE_INSTALL_PREFIX}/simple.base/include
)

target_link_libraries(${TARGET_NAME}
    ${SIMPLE_BASE_LIBS}
    ${GTEST_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
