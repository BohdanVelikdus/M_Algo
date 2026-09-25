include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(DynStrUtf8Libname dyn_str_utf8)

add_library(${DynStrUtf8Libname}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dyn_str_utf8_memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dyn_str_utf8_lifetime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dyn_str_utf8_validation.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dyn_str_utf8_inspection.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dyn_str_utf8_modification.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dyn_str_utf8_io.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dyn_str_utf8_encoding.c
)

target_include_directories(${DynStrUtf8Libname}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${DynStrUtf8Libname} PROPERTIES PUBLIC_HEADER
        "${CMAKE_CURRENT_SOURCE_DIR}/include/dyn_str_utf8.h"
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_options(${DynStrUtf8Libname} PRIVATE
            $<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>>:-Wall -Wextra>
    )
endif ()

# == Installing

set(EXPORT_NAME "${DynStrUtf8Libname}Targets")
set(NAMESPACE "${DynStrUtf8Libname}::")

set(CONFIG_TEMPLATE_IN "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in")

if (NOT EXISTS "${CONFIG_TEMPLATE_IN}")
    file(WRITE "${CONFIG_TEMPLATE_IN}" [[
    @PACKAGE_INIT@
    include("${CMAKE_CURRENT_LIST_DIR}/@EXPORT_NAME@.cmake")
  ]])
endif()

install(TARGETS ${DynStrUtf8Libname}
        EXPORT ${EXPORT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${EXPORT_NAME}
        NAMESPACE ${NAMESPACE}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${DynStrUtf8Libname}
)

set(VERSION_CONFIG_FILE "${CMAKE_BINARY_DIR}/${DynStrUtf8Libname}ConfigVersion.cmake")
set(CONFIG_FILE "${CMAKE_BINARY_DIR}/${DynStrUtf8Libname}Config.cmake")

include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CONFIG_TEMPLATE_IN}"
        "${CONFIG_FILE}"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${DynStrUtf8Libname}
)

write_basic_package_version_file(
        "${VERSION_CONFIG_FILE}"
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        "${CONFIG_FILE}"
        "${VERSION_CONFIG_FILE}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${DynStrUtf8Libname}
)

# == Testing

if(BUILD_TESTING)
    add_executable(dyn_str_utf8_test ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cpp)

    target_link_libraries(dyn_str_utf8_test
        PRIVATE
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
            GTest::gmock_main
            ${DynStrUtf8Libname}
    )
    add_test(NAME DynStrUtf8Test COMMAND $<TARGET_FILE:dyn_str_utf8_test>)
endif ()