cmake_minimum_required(VERSION 3.10)

SET(APP_VERSION  "0.3.2")
project(T5MEMORY VERSION ${APP_VERSION})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread")

message(STATUS "T5MEMORY CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
message(STATUS "T5MEMORY ROOT is ${ROOT}")
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(CMAKE_SOURCE_DIR    "${ROOT}/source")

set(folly_DIR "${ROOT}/source/proxygen/proxygen/_build/deps/lib/cmake/folly")
set(wangle_DIR "${ROOT}/source/proxygen/proxygen/_build/deps/lib/cmake/wangle")
set(Fizz_DIR "${ROOT}/source/proxygen/proxygen/_build/deps/lib/cmake/fizz")

include("${ROOT}/source/cmake/CheckGit.cmake")
CheckGitSetup()
set(T5MEMORY_SOURCE_ROOT "${ROOT}/source")
set(T5MEMORY_Source_Files
    "${T5MEMORY_SOURCE_ROOT}/RestAPI/OtmMemoryServiceWorker.cpp"
    "${T5MEMORY_SOURCE_ROOT}/RestAPI/OTMMSJSONFactory.cpp"
    "${T5MEMORY_SOURCE_ROOT}/otmd.cpp"
    "${T5MEMORY_SOURCE_ROOT}/config.cpp"
    "${T5MEMORY_SOURCE_ROOT}/RestAPI/ProxygenServer.cpp"
    "${T5MEMORY_SOURCE_ROOT}/RestAPI/ProxygenHandler.cpp"
)

link_directories(${RESTBED_LIBRARIES})

add_executable(T5MEMORY ${T5MEMORY_Source_Files})

set(CMAKE_CXX_STANDARD 17)
target_compile_features(T5MEMORY PRIVATE cxx_std_17)

target_include_directories(T5MEMORY PRIVATE
    "${CMAKE_SOURCE_DIR}/proxygen"
    "${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/include"
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OPENTM2_INCLUDE_DIR}
    "${ROOT}/include"
)

find_package(Threads)
if(TARGET Threads::Threads)
  target_link_libraries(T5MEMORY PRIVATE Threads::Threads)
endif()

#find_package(Boost REQUIRED)
find_package(ICU 67.1 COMPONENTS uc i18n REQUIRED)

set(CMAKE_PREFIX_PATH 
  "${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build"
  "${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/lib/cmake/mapped_sodium"
)


set(PROXYGEN_PREFIX_PATH "${ROOT}/source/proxygen")
set(PROXYGEN_DEPENDENCIES_PREFIX_PATH "${ROOT}/source/proxygen/proxygen/_build/deps")
target_link_libraries(T5MEMORY PRIVATE lzma)
set(FMT_INSTALL ON)
add_subdirectory("${PROXYGEN_DEPENDENCIES_PREFIX_PATH}/fmt")
target_link_libraries(T5MEMORY PRIVATE fmt  )
add_subdirectory("${PROXYGEN_DEPENDENCIES_PREFIX_PATH}/folly")
add_subdirectory("${PROXYGEN_DEPENDENCIES_PREFIX_PATH}/fizz/fizz")
add_subdirectory("${PROXYGEN_DEPENDENCIES_PREFIX_PATH}/googletest")
add_subdirectory("${PROXYGEN_DEPENDENCIES_PREFIX_PATH}/wangle/wangle")

add_subdirectory("${PROXYGEN_PREFIX_PATH}")

set(OPENTM2_FOLDER "${ROOT}/source/opentm2")
add_subdirectory("${OPENTM2_FOLDER}")

target_link_libraries(T5MEMORY PRIVATE
  OTMFUNC
  git_version
  proxygen
  proxygenhttpserver
)
 
#add missing libs here (probably?) 
#target_link_libraries(T5MEMORY PUBLIC
#  proxygen
#  proxygenhttpserver
#)


set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")

configure_file("${ROOT}/source/OtmMemoryServiceConfig.h.in" "${ROOT}/source/OtmMemoryServiceConfig.h"  @ONLY)

install(TARGETS T5MEMORY DESTINATION bin)


# Configure CPack
include(InstallRequiredSystemLibraries)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME t5memory_${APP_VERSION})
#set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME t5memory_DEBUG)
set(PNAME t5memory_${APP_VERSION})
set(CPACK_PACKAGE_NAME                    ${PNAME})
#set(CPACK_PACKAGE_VENDOR                  "Translate 5")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY     ${PNAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY       ${CPACK_PACKAGE_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR           "${OtmMemoryService_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR           "${OtmMemoryService_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH           "${OtmMemoryService_VERSION_PATCH}")
set(CPACK_VERBATIM_VARIABLES              TRUE)
set(CPACK_GENERATOR                        "TXZ")
#set(CPACK_PACKAGE_DESCRIPTION_FILE        "${CMAKE_CURRENT_SOURCE_DIR}/Description.txt")
#set(CPACK_RESOURCE_FILE_WELCOME           "${CMAKE_CURRENT_SOURCE_DIR}/Welcome.txt")
#set(CPACK_RESOURCE_FILE_LICENSE           "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
#set(CPACK_RESOURCE_FILE_README            "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
include(CPack)

