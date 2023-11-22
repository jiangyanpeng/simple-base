CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(simple.base)

######## COMPILE #######
SET(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
SET(CMAKE_BUILD_TYPE "DEBUG")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
SET(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread")

# IF(BUILD_USE_ENPU MATCHES ON)
#     ADD_DEFINITIONS(-DUSE_ENPU)
#     SET(USE_ENPU_MACRO "#define USE_ENPU\n")
#     INCLUDE(cmake/enpu_config.cmake)
#     MESSAGE(STATUS "Using ENPU_VERSION: ${ENPU_VERSION}")
#     MESSAGE(STATUS "Using FORCE_32BIT_BUILD: ${FORCE_32BIT_BUILD}")
# ELSE()
#     INCLUDE(cmake/x86_config.cmake)
#     MESSAGE(STATUS "Using C: ${CMAKE_C_COMPILER_VERSION}")
# ENDIF()

# IF(BUILD_USE_STB MATCHES ON)
#     ADD_DEFINITIONS(-DSTB_IMAGE_IMPLEMENTATION)
# 	ADD_DEFINITIONS(-DSTB_IMAGE_WRITE_IMPLEMENTATION)
# ENDIF()

######## CONFIG OPTION ######
OPTION(BUILD_USE_AVX           "Use AVX"                    OFF)
OPTION(BUILD_TEST              "Build GooleTest"            OFF)

SET(SIMPLE_BASE_VERSION "1.0.5")
CONFIGURE_FILE(
    "${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h")
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

####### INCLUDE #######
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/include
)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)

####### SOURCES #######
FILE(GLOB_RECURSE base_src ${PROJECT_SOURCE_DIR}/src/*.cc)

####### THIDPARTY LIBS #######
SET(THIRD_PARTY_LIBS
    #${OpenCV_LIBS}
    #${EIGEN3_LIBS}
)

####### BUILD STATIC LIB #######
SET(SIMPLE_BASE_LIBS base)
ADD_LIBRARY(${SIMPLE_BASE_LIBS} STATIC ${base_src})

####### BUILD SHARED LIB #######
# SET(SIMPLE_BASE_LIBS base)
# ADD_LIBRARY(${SIMPLE_BASE_LIBS} SHARED ${base_src})

####### BUILD SAMPLE #######
ADD_SUBDIRECTORY(samples)

####### INSTALL LIBS #######
IF(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION "${CMAKE_INSTALL_PREFIX}/")
ENDIF()
IF(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    INSTALL(TARGETS ${SIMPLE_BASE_LIBS} ${SIMPLE_BASE_LIBS}
        ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
        LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" )
ENDIF()

####### UINT TEST   ##########
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
#[[
    @brief  Download dependency library from git repository.
    @param  url - git repository address
    @param  branch - the branch or tag name
    @param  lib_name - the expected local name
]]
function(base_download_dependency url branch lib_name work_directory)
    execute_process(COMMAND bash "-c" "if [ ! -d ${work_directory} ];\
        then mkdir -p ${work_directory}; fi")

    execute_process(COMMAND bash "-c" "if [ ! -d ${lib_name} ]; then \
        git clone --progress -q --depth=1 -b ${branch} ${url} ${lib_name}; fi"
        WORKING_DIRECTORY ${work_directory})
endfunction()

######### Note! Please Use GCC 5.0+ #######
IF(BUILD_TEST)
    base_download_dependency(
        "https://github.com/google/googletest.git"
        release-1.12.1
        gtest
        ${PROJECT_SOURCE_DIR}/third_party)

    CMAKE_POLICY(PUSH)
    OPTION(BUILD_GMOCK "Builds the googlemock subproject" OFF)
    OPTION(
        gtest_force_shared_crt
        "Use shared (DLL) run-time lib even when Google Test is built as static lib."
        ON)

    IF(NOT DEFINED GTEST_DIR)
        SET(GTEST_DIR third_party/gtest)
    ENDIF()

    ADD_SUBDIRECTORY(${GTEST_DIR})
    SET(GTEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/gtest/googletest/include)
    LIST(APPEND GTEST_LIBRARIES gtest gtest_main)
    CMAKE_POLICY(POP)
ENDIF(BUILD_TEST)

IF(BUILD_TEST)
    ADD_SUBDIRECTORY(tests)
ENDIF()