cmake_minimum_required(VERSION 3.10)

SET(APP_VERSION  "0.3.1")
project(T5MEMORY VERSION ${APP_VERSION})

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")


#set(CMAKE_C_FLAGS_TSAN
#    "-fsanitize=thread -g -O1"
#    CACHE STRING "Flags used by the C compiler during ThreadSanitizer builds."
#    FORCE)
#set(CMAKE_CXX_FLAGS_TSAN
#    "-fsanitize=thread -g -O1"
#    CACHE STRING "Flags used by the C++ compiler during ThreadSanitizer builds."
#    FORCE)
#address sanitizer
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address log_path=asan_log_file.$pid -fsanitize-recover=address")


set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread")

#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")
#set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")

#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=thread")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=thread")
#set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=thread")
#

#thread sanitizer
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -g -O1")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -O1")
#set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread -g -O1")
#

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(RESTBED_PATH        "${ROOT}/Packages/restbed/distribution")
set(RESTBED_INCLUDE_DIR "${RESTBED_PATH}/include")
set(RESTBED_LIBRARIES   "${RESTBED_PATH}/library")

include(./cmake/CheckGit.cmake)
CheckGitSetup()

set(T5MEMORY_Source_Files
    OtmMemoryService.cpp
    OtmMemoryServiceWorker.cpp
    OTMMSJSONFactory.cpp
    otmd.cpp
    config.cpp
    ProxygenServer.cpp
    ProxygenHandler.cpp 
)

link_directories(${RESTBED_LIBRARIES})

add_executable(T5MEMORY ${T5MEMORY_Source_Files})

set(CMAKE_CXX_STANDARD 17)
target_compile_features(T5MEMORY PRIVATE cxx_std_17)

target_include_directories(T5MEMORY PRIVATE
    "${CMAKE_SOURCE_DIR}/proxygen"
    "${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/include"
    #${CMAKE_CURRENT_BINARY_DIR}
    ${OPENTM2_INCLUDE_DIR}
    ${RESTBED_INCLUDE_DIR}
)

message(STATUS " TRANSLATE5:  PROXYGEN_INCLUDE_DIR  is ${CMAKE_SOURCE_DIR}/proxygen")

message(STATUS " TRANSLATE5:  PROXYGEN_INCLUDE_DIR  is ${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/include")
#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
find_package(Threads)
if(TARGET Threads::Threads)
  target_link_libraries(T5MEMORY PRIVATE Threads::Threads)
endif()

#find_package(Boost REQUIRED)

set(CMAKE_PREFIX_PATH 
"${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build"
"${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/lib/cmake/mapped_sodium"
)

find_package(gflags REQUIRED)
find_package(Boost 1.58 REQUIRED
  COMPONENTS
    iostreams
    context
    filesystem
    program_options
    regex
    system
    thread
)

# target_link_libraries(T5MEMORY PRIVATE lzma)
set(FMT_INSTALL ON)
add_subdirectory(proxygen/proxygen/_build/deps/fmt)
target_link_libraries(T5MEMORY PRIVATE fmt  )
add_subdirectory(proxygen/proxygen/_build/deps/folly)
add_subdirectory(proxygen/proxygen/_build/deps/fizz/fizz)
add_subdirectory(proxygen/proxygen/_build/deps/googletest)
add_subdirectory(proxygen/proxygen/_build/deps/wangle/wangle)

add_subdirectory(proxygen)
add_subdirectory(opentm2)

target_link_libraries(T5MEMORY PRIVATE
  OTMFUNC
  librestbed.a
  git_version
  proxygen
  proxygenhttpserver
)
 


set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

configure_file(OtmMemoryServiceConfig.h.in OtmMemoryServiceConfig.h  @ONLY)

install(TARGETS T5MEMORY DESTINATION bin)


# Configure CPack
include(InstallRequiredSystemLibraries)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME t5memory_${APP_VERSION})
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
