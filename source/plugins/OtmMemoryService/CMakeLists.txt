cmake_minimum_required (VERSION 3.10)

project(OtmMemoryService VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(RESTBED_PATH "../../../Packages/restbed")
set(RESTBED_INCLUDE_DIR "${RESTBED_PATH}/distribution/include")
set(RESTBED_LIBRARIES "${RESTBED_PATH}/distribution/library")

set(OPENTM2_INCLUDE_DIR "../../../include")
set(OPENTM2_API_DIR "../../api")
set(OPENTM2_MEM_DIR "../../core/memory")

include_directories(${RESTBED_INCLUDE_DIR}
                    ${OPENTM2_INCLUDE_DIR})

link_directories(${RESTBED_LIBRARIES})

add_library(OpenTM2 ${OPENTM2_API_DIR}/OTMFUNC.C
                    ${OPENTM2_MEM_DIR}/EQFMEMLP.CPP
                    ${OPENTM2_MEM_DIR}/EQFMEMEP.CPP)

add_executable(OtmMemoryService
               OtmMemoryService.cpp
               OtmMemoryServiceWorker.cpp
               OtmMemoryServiceTester.cpp
               OTMMSJSONFactory.cpp)

#add_executable(OtmMemoryServiceTester
                #OtmMemoryServiceTester.cpp
                #OtmMemoryServiceWorker.cpp
                #OTMMSJSONFactory.cpp)

target_link_libraries(OtmMemoryService librestbed.so OpenTM2)
