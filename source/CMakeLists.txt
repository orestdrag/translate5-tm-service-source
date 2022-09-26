cmake_minimum_required(VERSION 3.10)

SET(APP_VERSION  "0.3.5")
project(T5MEMORY VERSION ${APP_VERSION})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread")
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")
#set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")


set(CMAKE_BUILD_TYPE Debug)

message(STATUS "T5MEMORY CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
message(STATUS "T5MEMORY ROOT is ${ROOT}")
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(CMAKE_SOURCE_DIR    "${ROOT}/source")

#set (proxygen_DIR "/opt/local/lib/cmake/proxygen")
set (proxygen_DIR "/opt/local/lib/cmake/proxygen")
set (fmt_DIR "/opt/local/lib/cmake/fmt")
set (Fizz_DIR "/opt/local/lib/cmake/fizz")
set (folly_DIR "/opt/local/lib/cmake/folly")
set (wangle_DIR "/opt/local/lib/cmake/wangle")
set (zstd_DIR "/opt/local/lib/cmake/zstd")
set (glog_DIR "/opt/local/lib/cmake/glog")
set (gflags_DIR "/opt/local/lib/cmake/gflags")
set (mvfst_DIR "/opt/local/lib/cmake/mvfst")
set(Sodium_DIR "/opt/local/lib/cmake/sodium")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSODIUM libsodium)

set (Boost_INCLUDE_DIR "/opt/local/include")

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

#set(CMAKE_CXX_STANDARD 17)
#target_compile_features(T5MEMORY PRIVATE cxx_std_17)
set(CMAKE_CXX_STANDARD 14)
target_compile_features(T5MEMORY PRIVATE cxx_std_14)

target_include_directories(T5MEMORY PRIVATE
    "${CMAKE_SOURCE_DIR}/proxygen"
    "${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/include"
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OPENTM2_INCLUDE_DIR}
    "/opt/local/include"
    "${ROOT}/include"
)

find_package(Threads)
find_package(OpenSSL REQUIRED)
find_package(proxygen REQUIRED)
if(TARGET Threads::Threads)
  target_link_libraries(T5MEMORY PRIVATE Threads::Threads)
endif()

find_package(ICU 66.0 COMPONENTS uc i18n REQUIRED)

set(OPENTM2_FOLDER "${ROOT}/source/opentm2")
add_subdirectory("${OPENTM2_FOLDER}")



set(LIBS_DIRECTORY "/opt/local/lib")
set(SHARED_LIBS_DIR "/lib")

link_directories(${LIBS_DIRECTORY} 
				${SHARED_LIBS_DIR})

find_package(folly REQUIRED)
find_package(gflags REQUIRED)
find_package(proxygen REQUIRED)


target_link_libraries(T5MEMORY PRIVATE
  OTMFUNC
  git_version
  Folly::folly
  fizz::fizz
  proxygen::proxygen
  proxygen::proxygenhttpserver 
  wangle::wangle
  Boost::boost
  Boost::iostreams
  ssl
  crypto
  "${LIBS_DIRECTORY}/libzstd.a"
)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")

configure_file("${ROOT}/source/OtmMemoryServiceConfig.h.in" "${ROOT}/source/OtmMemoryServiceConfig.h"  @ONLY)

install(TARGETS T5MEMORY DESTINATION bin)

# Configure CPack
include(InstallRequiredSystemLibraries)

#next lines sets name of binaries as t5memory_DEBUG or t5memory_version(liket5memory_0_3_5)
#set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME t5memory_${APP_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME t5memory_DEBUG)

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

