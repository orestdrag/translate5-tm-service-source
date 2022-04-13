cmake_minimum_required(VERSION 3.10)


SET(APP_VERSION  "0.2.7")
project(OtmMemoryService VERSION ${APP_VERSION})

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")


set(CMAKE_C_FLAGS_TSAN
    "-fsanitize=thread -g -O1"
    CACHE STRING "Flags used by the C compiler during ThreadSanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_TSAN
    "-fsanitize=thread -g -O1"
    CACHE STRING "Flags used by the C++ compiler during ThreadSanitizer builds."
    FORCE)
#address sanitizer
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address log_path=asan_log_file.$pid -fsanitize-recover=address")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")
set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")
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
#find_package(Threads REQUIRED)
find_package(Threads)
if(TARGET Threads::Threads)
  target_link_libraries(OtmMemoryService PRIVATE Threads::Threads)
endif()

target_link_libraries(OtmMemoryService PRIVATE
    OTMFUNC
    librestbed.a
    git_version
    #Threads::Threads
)

add_subdirectory(opentm2)
#add_subdirectory(zip)

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
set(CPACK_PACKAGE_NAME                    "t5memory")
#set(CPACK_PACKAGE_VENDOR                  "Translate 5")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY     "t5memory")
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
