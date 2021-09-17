cmake_minimum_required (VERSION 3.10)
project(OtmMemoryService)

add_subdirectory(opentm2)

set(ROOT "..")
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(RESTBED_PATH        "${ROOT}/Packages/restbed")
set(RESTBED_INCLUDE_DIR "${RESTBED_PATH}/distribution/include")
set(RESTBED_LIBRARIES   "${RESTBED_PATH}/distribution/library")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(OtmMemoryService_Source_Files
        OtmMemoryService.cpp
        OtmMemoryServiceWorker.cpp
        OTMMSJSONFactory.cpp
        #OtmMemoryServiceTester.cpp
        otmd.cpp
        config.cpp
)

include_directories(${OPENTM2_INCLUDE_DIR} ${RESTBED_INCLUDE_DIR})
link_directories(${RESTBED_LIBRARIES})

add_executable(OtmMemoryService ${OtmMemoryService_Source_Files})
target_link_libraries(OtmMemoryService OTMFUNC librestbed.so)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
include(CPack)
