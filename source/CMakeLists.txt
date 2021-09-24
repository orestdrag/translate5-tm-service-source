cmake_minimum_required (VERSION 3.10)

project(OtmMemoryService VERSION 0.1)

set(ROOT "..")
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(RESTBED_PATH        "${ROOT}/Packages/restbed")
set(RESTBED_INCLUDE_DIR "${RESTBED_PATH}/distribution/include")
set(RESTBED_LIBRARIES   "${RESTBED_PATH}/distribution/library")

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${OPENTM2_INCLUDE_DIR}
                    ${RESTBED_INCLUDE_DIR}
)

link_directories(${RESTBED_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(OtmMemoryService_Source_Files
    OtmMemoryService.cpp
    OtmMemoryServiceWorker.cpp
    OTMMSJSONFactory.cpp
    #OtmMemoryServiceTester.cpp
    otmd.cpp
    config.cpp
)

add_executable(OtmMemoryService ${OtmMemoryService_Source_Files})

target_compile_features(OtmMemoryService PRIVATE cxx_std_14)

target_link_libraries(OtmMemoryService OTMFUNC librestbed.so)

configure_file(OtmMemoryServiceConfig.h.in OtmMemoryServiceConfig.h  @ONLY)

add_subdirectory(opentm2)

#configure_file(OpenTM2_EqfMemoryPlugin.pc @ONLY)

install(TARGETS OtmMemoryService DESTINATION bin)

#install(TARGETS OtmMemoryService
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#)

install(FILES
    ${CMAKE_BINARY_DIR}/OpenTM2_EqfMemoryPlugin.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${OtmMemoryService_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${OtmMemoryService_VERSION_MINOR}")
include(CPack)
