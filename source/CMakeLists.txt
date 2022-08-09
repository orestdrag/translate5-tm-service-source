cmake_minimum_required(VERSION 3.10)

SET(APP_VERSION  "0.3.0")
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


set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}") # -pthread")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS}") # -pthread")

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

#set(PROXYGEN_LIBRARIES   "${ROOT}/Packages/proxygen")
#set(PROXYGEN_ROOT "${ROOT}/source/proxygen")
#set(PROXYGEN_DEPS_LIBRARIES   "${ROOT}/Packages/proxygen/deps/lib")
#set(PROXYGEN_DEPS_LIBRARIES   "${ROOT}/Packages/proxygen/deps/lib")

#set(PROXYGEN_DEPS_LIBRARIES   "${ROOT}/Packages/proxygen/proxygen/_build/deps/lib")
#set(PROXYGEN_DEPS_INCLUDE_DIR   "${ROOT}/Packages/proxygen/proxygen/_build/deps/include")
#set(PROXYGEN_LIBRARIES   "${ROOT}/Packages/proxygen/proxygen/_build/lib")
#set(PROXYGEN_INCLUDE_DIR   "${ROOT}/Packages/proxygen/proxygen/_build/include")

#set(PROXYGEN_INCLUDE_LIBRARIES   "${ROOT}/Packages/proxygen/include")

include(./cmake/CheckGit.cmake)
CheckGitSetup()

set(PROXYGEN_DEPS_LIBRARIES   "${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/lib")

set(CMAKE_PREFIX_PATH 
#    #"${PROXYGEN_DEPS_LIBRARIES}/cmake/unofficial-sodium"    
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/mapped_sodium"  
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/fmt" 
    "${PROXYGEN_DEPS_LIBRARIES}/cmake/folly"
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/wangle"
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/fizz"
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/zstd"
#    #"${PROXYGEN_ROOT}/proxygen/_build/lib/cmake/proxygen"
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/proxygen"
)

set(CMAKE_PREFIX_PATH 
#    #"${PROXYGEN_DEPS_LIBRARIES}/cmake/unofficial-sodium"    

#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/mapped_sodium"  
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/fmt" 
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/folly"
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/wangle"
#    "${PROXYGEN_DEPS_LIBRARIES}/cmake/fizz"

    #    "${PROXYGEN_DEPS_LIBRARIES}/cmake/zstd"
#    #"${PROXYGEN_ROOT}/proxygen/_build/lib/cmake/proxygen"
  #  "${ROOT}/Packages/proxygen/proxygen/_build/"
 # "${ROOT}/Packages/proxygen/proxygen/_build/lib/cmake/proxygen"

)

#set( ZSTD_INCLUDE_DIR "${PROXYGEN_INCLUDE_LIBRARIES}")
#set ( FOLLY_INCLUDE_DIR "${PROXYGEN_INCLUDE_LIBRARIES}")


set(T5MEMORY_Source_Files
    OtmMemoryService.cpp
    OtmMemoryServiceWorker.cpp
    OTMMSJSONFactory.cpp
    otmd.cpp
    config.cpp
    #ProxygenServer.cpp
    #ProxygenHandler.cpp 
)

link_directories(${RESTBED_LIBRARIES})
#link_directories(${PROXYGEN_LIBRARIES})
#link_directories(${PROXYGEN_DEPS_LIBRARIES})

add_executable(T5MEMORY ${T5MEMORY_Source_Files})

#set(CMAKE_CXX_STANDARD 14)
#target_compile_features(T5MEMORY PRIVATE cxx_std_14)
set(CMAKE_CXX_STANDARD 17)
target_compile_features(T5MEMORY PRIVATE cxx_std_17)

target_include_directories(T5MEMORY PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OPENTM2_INCLUDE_DIR}
    ${RESTBED_INCLUDE_DIR}
    #${PROXYGEN_ROOT}
    #${ZSTD_INCLUDE_DIR}
    #${PROXYGEN_DEPS_INCLUDE_DIR}
    ${PROXYGEN_INCLUDE_DIR}
)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
find_package(Threads)
if(TARGET Threads::Threads)
  target_link_libraries(T5MEMORY PRIVATE Threads::Threads)
endif()

#find_package(Boost REQUIRED)


#find_package(proxygen REQUIRED)
find_package(gflags REQUIRED)
#add_library(proxygenhttpserver)

#find_package(unofficial-sodium CONFIG REQUIRED)
#target_link_libraries(T5MEMORY PRIVATE unofficial-sodium::sodium unofficial-sodium::sodium_config_public)

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
#install( EXPORT FMT)
target_link_libraries(T5MEMORY PRIVATE fmt  )
add_subdirectory(proxygen/proxygen/_build/deps/folly)
#add_subdirectory(proxygen/proxygen/_build/deps/wangle/wangle)
#add_subdirectory(proxygen/proxygen/_build/deps/fizz/fizz)

target_link_libraries(T5MEMORY PRIVATE
    OTMFUNC
    librestbed.a
    
    git_version
    #libfolly.a
#)
   
#target_link_libraries(T5MEMORY PUBLIC
#PRIVATE
#proxygen
    #libproxygen.a
    #libproxygencurl.a
    #libproxygenhttpserver.a

   # fmt

    libdouble-conversion.a
    #sodium

    gflags
    crypto
    #Boost
    ${GFLAGS_LIBRARY}
    ssl
    zstd
    Boost::boost
    Boost::iostreams
    Boost::context
    Boost::filesystem
    Boost::program_options
    Boost::regex
    Boost::system
    Boost::thread
    libsodium.a
    libunwind.a
    liblzma.a
    libevent.a
    libaio.a
    libbz2.a 
    libc.a 
    libdwarf.a 
    libelf.a 
    libiberty.a 
    libjemalloc.a 
    liblz4.a 
    #libpthread-stubs0.a
    #libpthread.a
    libsnappy.a 
    #libunwind8.a 
    #libzstd-dev 
    #-lz
#
    #PROXYGEN_DEPS_LIBRARIES
    #proxygen
    #PROXYGEN_LIBRARIES
    #sodium
    #libfizz.a
    #libfizz_test_support.a
    #libfmt.a
    #libfolly.a
    #libfollybenchmark.a
    #libfolly_test_util.a
    #libgmock.a
    #libgmock_main.a
    #libgtest.a
    #libgtest_main.a
    #libproxygen.a
    #libproxygencurl.a
    #libproxygenhttpserver.a
    #libwangle.a
    #libzstd.a
    #glog
    #gflags
    #crypto
    #boost
    #${GFLAGS_LIBRARY}

    #libmvfst_buf_accessor.a
    #libmvfst_bufutil.a
    #libmvfst_cc_algo.a
    #libmvfst_client.a
    #libmvfst_codec.a
    #libmvfst_codec_decode.a
    #libmvfst_codec_packet_number_cipher.a
    #libmvfst_codec_pktbuilder.a
    #libmvfst_codec_pktrebuilder.a
    #libmvfst_codec_types.a
    #libmvfst_constants.a
    #libmvfst_d6d_state_functions.a
    #libmvfst_d6d_types.a
    #libmvfst_dsr_frontend.a
    #libmvfst_dsr_types.a
    #libmvfst_exception.a
    #libmvfst_fizz_client.a
    #libmvfst_fizz_handshake.a
    #libmvfst_flowcontrol.a
    #libmvfst_handshake.a
    #libmvfst_happyeyeballs.a
    #libmvfst_looper.a
    #libmvfst_loss.a
    #libmvfst_observer.a
    #libmvfst_qlogger.a
    #libmvfst_server.a
    #libmvfst_server_state.a
    #libmvfst_socketutil.a
    #libmvfst_state_ack_handler.a
    #libmvfst_state_datagram_handler.a
    #libmvfst_state_functions.a
    #libmvfst_state_machine.a
    #libmvfst_state_pacing_functions.a
    #libmvfst_state_simple_frame_functions.a
    #libmvfst_state_stream.a
    #libmvfst_state_stream_functions.a
    #libmvfst_transport.a
    #libmvfst_transport_knobs.a



    #Threads::Threads


    libglog.a
    libzstd.a
    #libfmt.a
    #libfolly.a
    #libfollybenchmark.a
    #libfolly_test_util.a
    libgmock.a
    libgmock_main.a
    libgtest.a
    libgtest_main.a
    #libfizz.a
    #libfizz_test_support.a
   #folly
    #libwangle.a
    #glog


    #libproxygen.a
    #libproxygencurl.a
    #libproxygenhttpserver.a
    #proxygen


)
 


add_subdirectory(opentm2)
#add_subdirectory(zip)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

configure_file(OtmMemoryServiceConfig.h.in OtmMemoryServiceConfig.h  @ONLY)
#configure_file(OpenTM2_EqfMemoryPlugin.pc @ONLY)

install(TARGETS T5MEMORY DESTINATION bin)

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
