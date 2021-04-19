cmake_minimum_required (VERSION 3.10)
project(OtmMemoryService)

add_subdirectory(opentm2)

set(ROOT "..")
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(RESTBED_PATH        "${ROOT}/Packages/restbed")
set(RESTBED_INCLUDE_DIR "${RESTBED_PATH}/distribution/include")
set(RESTBED_LIBRARIES   "${RESTBED_PATH}/distribution/library")

set(OtmMemoryService_Source_Files
        #OtmMemoryService.cpp
        OtmMemoryServiceWorker.cpp
        OTMMSJSONFactory.cpp
        #OtmMemoryServiceTester.cpp
        dummy_main.cpp
)

include_directories(${OPENTM2_INCLUDE_DIR} ${RESTBED_INCLUDE_DIR})
link_directories(${RESTBED_LIBRARIES})

add_executable(OtmMemoryService ${OtmMemoryService_Source_Files})
target_link_libraries(OtmMemoryService OTMFUNC librestbed.so) 
