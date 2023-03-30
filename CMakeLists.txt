cmake_minimum_required(VERSION 3.22)
project(signals VERSION 1.0.0)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.template ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h)

add_library(${PROJECT_NAME} INTERFACE)

file(GLOB SrcFiles src/*.h)

target_include_directories(${PROJECT_NAME} 
        INTERFACE 
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ygv/${PROJECT_NAME}/>
)

install(TARGETS ${PROJECT_NAME} 
    EXPORT ${PROJECT_NAME}Targets
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION 
        ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(Config.cmake.in
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION 
                ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/
)

install(EXPORT 
                ${PROJECT_NAME}Targets
        FILE
                ${PROJECT_NAME}Targets.cmake
        NAMESPACE 
                "ygv::"
        DESTINATION
                ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        COMPONENT
                Devel
)

install(
        FILES
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION
                ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        COMPONENT
                Devel
)


install(
        FILES 
                ${SrcFiles}
        DESTINATION 
                ${CMAKE_INSTALL_INCLUDEDIR}/ygv/${PROJECT_NAME}
)