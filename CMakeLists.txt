cmake_minimum_required(VERSION 3.24)

project(
    mgraph
    VERSION 0.1.0
    DESCRIPTION "ncnn model graph topology and param parsing"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

option(mgraph_build_tests "Build mgraph tests" ${PROJECT_IS_TOP_LEVEL})
option(MGRAPH_SANITIZE "Enable ASan+UBSan" OFF)

if(NOT TARGET mformat::mformat)
    set(MGRAPH_MFORMAT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mformat" CACHE PATH "mformat source directory")
    if(NOT EXISTS "${MGRAPH_MFORMAT_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "mgraph requires the mformat source tree; set MGRAPH_MFORMAT_DIR")
    endif()
    set(MFORMAT_MMAT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mmat" CACHE PATH "" FORCE)
    set(mformat_build_tests OFF CACHE BOOL "" FORCE)
    add_subdirectory("${MGRAPH_MFORMAT_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/mformat")
endif()

set(MGRAPH_WARNINGS_COMMON
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wold-style-cast
    -Wcast-qual
    -Wdouble-promotion
    -Wformat=2
    -Wundef
    -Wpointer-arith
    -Wwrite-strings
    -Wnull-dereference
    -Wzero-as-null-pointer-constant
)
set(MGRAPH_WARNINGS_GCC
    -Wmissing-declarations
    -Wredundant-decls
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
)
set(MGRAPH_WARNINGS
    ${MGRAPH_WARNINGS_COMMON}
    $<$<CXX_COMPILER_ID:GNU>:${MGRAPH_WARNINGS_GCC}>
)

set(MGRAPH_WARNINGS_PORTED
    -Wall
    -Wextra
    -Wpedantic
    -Wundef
    -Wpointer-arith
    -Wwrite-strings
    -Wnull-dereference
)

add_library(mgraph_sanitizer INTERFACE)
if(MGRAPH_SANITIZE)
    target_compile_options(mgraph_sanitizer INTERFACE
        -fsanitize=address,undefined -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS)
    target_link_options(mgraph_sanitizer INTERFACE
        -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/mgraph/config.hpp
    @ONLY)

add_library(mgraph_objs OBJECT
    src/graph.cpp
    src/log.cpp)
target_include_directories(mgraph_objs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    PRIVATE
        $<TARGET_PROPERTY:mformat::mformat,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_options(mgraph_objs PRIVATE ${MGRAPH_WARNINGS_PORTED})
target_link_libraries(mgraph_objs PRIVATE mformat::mformat mgraph_sanitizer)

add_library(mgraph STATIC)
target_link_libraries(mgraph PUBLIC
    $<BUILD_INTERFACE:mgraph_objs>
    $<BUILD_INTERFACE:mformat_objs>
    $<BUILD_INTERFACE:mmat_objs>
    $<BUILD_INTERFACE:mmemory_objs>
    $<BUILD_INTERFACE:mplatform_objs>)
add_library(mgraph::mgraph ALIAS mgraph)

target_include_directories(mgraph
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(mgraph INTERFACE $<BUILD_INTERFACE:mgraph_sanitizer>)

if(PROJECT_IS_TOP_LEVEL)
    include(CMakePackageConfigHelpers)

    install(TARGETS mgraph EXPORT mgraphTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/mgraph/config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mgraph)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../mformat/include/mformat
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mformat/include/mformat/config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mformat)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../mmat/include/mmat
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mformat/mmat/include/mmat/config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mmat)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../mmemory/include/mmemory
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mformat/mmat/mmemory/include/mmemory/config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mmemory)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../mplatform/include/mplatform
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mformat/mmat/mmemory/mplatform/include/mplatform/config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mplatform)
    install(EXPORT mgraphTargets
        NAMESPACE mgraph::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mgraph)

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mgraphConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/mgraphConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mgraph)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/mgraphConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/mgraphConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/mgraphConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mgraph)
endif()

if(mgraph_build_tests AND BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
