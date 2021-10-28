cmake_minimum_required(VERSION 3.10)
project(OtmMemoryService VERSION 0.1.2)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(RESTBED_PATH        "${ROOT}/Packages/restbed/distribution")
set(RESTBED_INCLUDE_DIR "${RESTBED_PATH}/include")
set(RESTBED_LIBRARIES   "${RESTBED_PATH}/library")


include(./cmake/CheckGit.cmake)
CheckGitSetup()

set(OtmMemoryService_Source_Files
    OtmMemoryService.cpp
    OtmMemoryServiceWorker.cpp
    OTMMSJSONFactory.cpp
    #OtmMemoryServiceTester.cpp
    otmd.cpp
    config.cpp
)

link_directories(${RESTBED_LIBRARIES})

add_executable(OtmMemoryService ${OtmMemoryService_Source_Files})

set(CMAKE_CXX_STANDARD 14)
target_compile_features(OtmMemoryService PRIVATE cxx_std_14)

target_include_directories(OtmMemoryService PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OPENTM2_INCLUDE_DIR}
    ${RESTBED_INCLUDE_DIR}
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(OtmMemoryService PRIVATE
    OTMFUNC
    librestbed.a
    git_version
    Threads::Threads
)

add_subdirectory(opentm2)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

configure_file(OtmMemoryServiceConfig.h.in OtmMemoryServiceConfig.h  @ONLY)
#configure_file(OpenTM2_EqfMemoryPlugin.pc @ONLY)

install(TARGETS OtmMemoryService DESTINATION bin)

#install(TARGETS OtmMemoryService
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#)

#install(FILES
#    ${CMAKE_BINARY_DIR}/OpenTM2_EqfMemoryPlugin.pc
#    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
#)

# Configure CPack
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME                    "OtmMemoryService")
#set(CPACK_PACKAGE_VENDOR                  "Translate 5")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY     "OtmMemoryService")
set(CPACK_PACKAGE_INSTALL_DIRECTORY       ${CPACK_PACKAGE_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR           "${OtmMemoryService_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR           "${OtmMemoryService_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH           "${OtmMemoryService_VERSION_PATCH}")
set(CPACK_VERBATIM_VARIABLES              TRUE)
#set(CPACK_PACKAGE_DESCRIPTION_FILE        "${CMAKE_CURRENT_SOURCE_DIR}/Description.txt")
#set(CPACK_RESOURCE_FILE_WELCOME           "${CMAKE_CURRENT_SOURCE_DIR}/Welcome.txt")
#set(CPACK_RESOURCE_FILE_LICENSE           "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
#set(CPACK_RESOURCE_FILE_README            "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
include(CPack)
