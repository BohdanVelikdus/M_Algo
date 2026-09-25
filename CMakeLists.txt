cmake_minimum_required(VERSION 4.3)

set(M_VERSION_MAJOR 1)
set(M_VERSION_MINOR 0)
set(M_VERSION_PATCH 0)

project(algos
        LANGUAGES C CXX
        VERSION "${M_VERSION_MAJOR}.${M_VERSION_MINOR}.${M_VERSION_PATCH}"
)

if(BUILD_TESTING)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare( googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG main
            GIT_SHALLOW TRUE
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

# ARR ADT
if(BUILD_ADD_ADT)
    add_library(arr_adt
        ${CMAKE_CURRENT_SOURCE_DIR}/arr_adt/dyn_arr.c
    )
    target_include_directories(arr_adt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/arr_adt/include)

    add_executable(add_adt_test ${CMAKE_CURRENT_SOURCE_DIR}/arr_adt/test.cpp)

    target_link_libraries(add_adt_test
        PRIVATE
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
            GTest::gmock_main
            arr_adt
    )
endif ()

# CONSTEXPR ALGO
if(BUILD_CONSTEXPR_REC_ALGO)
    add_library(constexpr_recursion_alog INTERFACE)
    target_include_directories(constexpr_recursion_alog
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/constexpr_recursion_algo/include
    )

    add_executable(constexpr_recursion_alog_test ${CMAKE_CURRENT_SOURCE_DIR}/arr_adt/test.cpp)

    target_link_libraries(constexpr_recursion_alog_test
            PRIVATE
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
            GTest::gmock_main
            constexpr_recursion_alog
    )
endif ()

if(BUILD_U8_STR)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dyn_str_utf8)
endif()
