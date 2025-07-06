cmake_minimum_required(VERSION 3.22.1)
project(voidstar LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

################################################################################
# Dependencies
#

set(_voidstar_module_path_save "${CMAKE_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(libffi REQUIRED 3.0)

set(CMAKE_MODULE_PATH "${_voidstar_module_path_save}")
unset(_voidstar_module_path_save)

################################################################################
# Target
#

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(${PROJECT_NAME} INTERFACE libffi::libffi)

set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})
set_property(TARGET ${PROJECT_NAME} PROPERTY
    INTERFACE_${PROJECT_NAME}_MAJOR_VERSION 3)
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING ${PROJECT_NAME}_MAJOR_VERSION
)

################################################################################
# Install
#

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Headers
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN *.h)

# Docs
install(
    DIRECTORY docs/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    FILES_MATCHING PATTERN *.md
)

# CMake target
install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

################################################################################
# Package
#

# Findlibffi.cmake
install(
    FILES cmake/Findlibffi.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/find_modules
)

include(CMakePackageConfigHelpers)

# Configuration file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

################################################################################
# Misc
#

add_subdirectory(example)
add_subdirectory(test)
