cmake_minimum_required(VERSION 3.10)
SET(APP_VERSION_GLOBAL 0)
SET(APP_VERSION_MAJOR 6)
SET(APP_VERSION_MINOR 68)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(APP_VERSION  "${APP_VERSION_GLOBAL}.${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}")

project(T5MEMORY VERSION ${APP_VERSION})


# Set the compilers to gcc-10 and g++-10
#set(CMAKE_C_COMPILER "/usr/bin/gcc-10")
#set(CMAKE_CXX_COMPILER "/usr/bin/g++-10")

# Define an option for selecting the sanitizer
set(SANITIZER "asan" CACHE STRING "Specify sanitizer type: asan, tsan, or ubsan")

# Apply the chosen sanitizer flags
#if(SANITIZER STREQUAL "asan")
#    message("AddressSanitizer is enabled")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address")# -fsanitize-recover=address")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address")# -fsanitize-recover=address")
#    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address")# -fsanitize-recover=address")
#elseif(SANITIZER STREQUAL "tsan")
#    message("ThreadSanitizer is enabled")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=thread")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=thread")
#    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=thread")
#elseif(SANITIZER STREQUAL "ubsan")
#    message("Undefined Behavior Sanitizer is enabled")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=undefined")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=undefined")
#    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fno-omit-frame-pointer  -fsanitize=undefined -lstdc++")
#else()
#    message("No sanitizer is enabled")
#endif()

# Enable ASan with verbose error reporting for detailed info - slow asan 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fsanitize=address -fno-omit-frame-pointer")# -O1 -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -fsanitize=address -fno-omit-frame-pointer")# -O1 -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fno-omit-frame-pointer -fsanitize=address")# -O1 -g  -fsanitize-recover=address")

# Optionally set runtime options to make ASan output more informative
set(ASAN_OPTIONS "log_path=/root/asan.log:detect_stack_use_after_return=1:detect_leaks=1:fast_unwind_on_malloc=0:halt_on_error=1:verbosity=1")
add_definitions(-DASAN_OPTIONS="${ASAN_OPTIONS}")
add_custom_target(set_asan_options ALL
    COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS="${ASAN_OPTIONS}" ${CMAKE_COMMAND} .
)

set(CMAKE_BUILD_TYPE Debug)
#include(GNUInstallDirs)

message(STATUS "T5MEMORY CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
message(STATUS "T5MEMORY ROOT is ${ROOT}")
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(CMAKE_SOURCE_DIR    "${ROOT}/source")
set(TSAN_OPTIONS "suppressions=${ROOT}/tsan.supp")

set (lib_cmake_DIR "/home/libs/proxygen/_build/deps/lib/cmake")
#set(lib_cmake_DIR "/opt/local/lib/cmake/" )

#set (proxygen_DIR "${lib_cmake_DIR}/proxygen")
set (proxygen_DIR "/home/libs/proxygen/_build/lib/cmake/proxygen")

set (fmt_DIR "${lib_cmake_DIR}/fmt")
set (Fizz_DIR "${lib_cmake_DIR}/fizz")
set (folly_DIR "${lib_cmake_DIR}/folly")
set (wangle_DIR "${lib_cmake_DIR}/wangle")
set (zstd_DIR "${lib_cmake_DIR}/zstd")
set (glog_DIR "${lib_cmake_DIR}/glog")
set (gflags_DIR "${lib_cmake_DIR}/gflags")
set (mvfst_DIR "${lib_cmake_DIR}/mvfst")
set (Sodium_DIR "${lib_cmake_DIR}/sodium")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSODIUM libsodium)

set (Boost_INCLUDE_DIR "/opt/local/include")

include("${ROOT}/source/cmake/CheckGit.cmake")
CheckGitSetup()
set(T5MEMORY_SOURCE_ROOT "${ROOT}/source")
set(CORE_DIR "opentm2/core")

set(OPENTM2_MRI_DIR     "${ROOT}/mri")

set(T5MEMORY_Source_Files
    "${T5MEMORY_SOURCE_ROOT}/opentm2/OTMFUNC.cpp"
    "${T5MEMORY_SOURCE_ROOT}/otmd.cpp"
    "${T5MEMORY_SOURCE_ROOT}/config.cpp"
    "${CORE_DIR}/tm.cpp"
    "${CORE_DIR}/requestdata.cpp"
    "${CORE_DIR}/ProposalFilter.cpp"

#RestAPI
    "${T5MEMORY_SOURCE_ROOT}/RestAPI/OtmMemoryServiceWorker.cpp"
    "${T5MEMORY_SOURCE_ROOT}/RestAPI/OTMMSJSONFactory.cpp"
    "${T5MEMORY_SOURCE_ROOT}/RestAPI/ProxygenServer.cpp"
    "${T5MEMORY_SOURCE_ROOT}/RestAPI/ProxygenHandler.cpp"
    "${T5MEMORY_SOURCE_ROOT}/RestAPI/ProxygenStats.cpp"
#pluginmanager
    "${CORE_DIR}/pluginmanager/PluginManager.cpp"
    "${CORE_DIR}/pluginmanager/PluginManagerImpl.cpp"
#tagtable
    "${CORE_DIR}/tagtable/EQFTOKEN.cpp"
    "${CORE_DIR}/tagtable/MarkupPluginMapper.cpp"
#morph
    "${CORE_DIR}/morph/MorphFactory.cpp"
    "${CORE_DIR}/morph/OtmMorphICU.cpp"
    "${CORE_DIR}/morph/OtmMorphICUPlugin.cpp" 
#utilities
    "${CORE_DIR}/utilities/EncodingHelper.cpp"
    "${CORE_DIR}/utilities/EQFDICT.cpp"
    "${CORE_DIR}/utilities/EQFHASH.cpp"
    "${CORE_DIR}/utilities/EQFMORPH.cpp"
    "${CORE_DIR}/utilities/EQFMORPW.cpp"
    "${CORE_DIR}/utilities/EQFOBJ00.cpp"
    "${CORE_DIR}/utilities/EqfPluginWrapper.cpp"
    "${CORE_DIR}/utilities/EQFPRO00.cpp"
    "${CORE_DIR}/utilities/EQFSETUP.cpp"
    "${CORE_DIR}/utilities/EQFTADIT.cpp"
    "${CORE_DIR}/utilities/EQFUTDOS.cpp"
    "${CORE_DIR}/utilities/EQFUTERR.cpp"
    "${CORE_DIR}/utilities/EQFUTFIL.cpp"
    "${CORE_DIR}/utilities/EQFUTLNG.cpp"
    "${CORE_DIR}/utilities/FilesystemHelper.cpp"
    "${CORE_DIR}/utilities/FilesystemWrapper.cpp"
    "${CORE_DIR}/utilities/LanguageFactory.cpp"
    "${CORE_DIR}/utilities/LogWrapper.cpp"
    "${CORE_DIR}/utilities/OSWrapper.cpp"
    "${CORE_DIR}/utilities/OTMGLOBM.cpp"
    "${CORE_DIR}/utilities/OtmProposal.cpp"
    "${CORE_DIR}/utilities/Property.cpp"
    "${CORE_DIR}/utilities/ThreadingWrapper.cpp"
    "${CORE_DIR}/utilities/UtlAlloc.cpp"
    "${CORE_DIR}/utilities/UtlLangCP.cpp"
    "${CORE_DIR}/utilities/UtlMATVal.cpp"
    "${CORE_DIR}/utilities/UtlMisc.cpp"
    "${CORE_DIR}/utilities/UtlRegistry.cpp"
    "${CORE_DIR}/utilities/UtlString.cpp"
    "${CORE_DIR}/utilities/ZipHelper.cpp" 

#document
    "${CORE_DIR}/document/DocumentPluginMapper.cpp"
    "${CORE_DIR}/document/CXMLWRITER.cpp"
    "${CORE_DIR}/document/InitPlugins.cpp"
    "${CORE_DIR}/document/EQFTATAG.cpp"
    "${CORE_DIR}/document/EQFMAIN.cpp"

#EqfMemoryPlugin
    "${CORE_DIR}/EqfMemoryPlugin/EQFFUZZ.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFMEM00.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFMEMEP.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFMEMLP.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EqfMemory.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EqfMemoryPlugin.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFNTDEL.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFNTEXT.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFNTGET.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFNTMDB.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFNTOP.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFNTPUT.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/EQFNTUTL.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/GenericTagReplace.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/OTMTMXIE.cpp"
    "${CORE_DIR}/EqfMemoryPlugin/TMPluginWrapper.cpp"
       
)

link_directories(/usr/lib/gcc/x86_64-linux-gnu/9/)
#link_directories(/usr/lib/gcc/x86_64-linux-gnu/10/)




set_source_files_properties(
    /home/or/workspace/translate5/translate5-tm-service-source/source/RestAPI/ProxygenServer.cpp
    PROPERTIES COMPILE_FLAGS "-fno-sanitize=thread"
)

set_source_files_properties(${T5MEMORY_Source_Files} PROPERTIES LANGUAGE CXX )

add_definitions( -DT5GLOBVERSION=${APP_VERSION_GLOBAL} )
add_definitions( -DT5MAJVERSION=${APP_VERSION_MAJOR} )
add_definitions( -DT5MINVERSION=${APP_VERSION_MINOR} )

add_executable(T5MEMORY ${T5MEMORY_Source_Files})

#set(CMAKE_CXX_STANDARD 14)
target_compile_features(T5MEMORY PRIVATE  cxx_std_17)

target_compile_options(T5MEMORY PRIVATE -Wunused-function)#-Wall -Wextra 

target_include_directories(T5MEMORY PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OPENTM2_INCLUDE_DIR}
    "/opt/local/include"
    "${ROOT}/include"
    ${OPENTM2_INCLUDE_DIR}
    ${OPENTM2_MRI_DIR}
    
    "${CORE_DIR}/utilities"
    "${CORE_DIR}/pluginmanager"
    "${CORE_DIR}/tagtable"
    "${CORE_DIR}/morph"
    "${CORE_DIR}/EqfMemoryPlugin"
)

find_package(Threads)
find_package(OpenSSL REQUIRED)
find_package(proxygen REQUIRED)
#find_package(glog REQUIRED)

target_compile_definitions(T5MEMORY PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)

add_definitions(-w)
#add_definitions(-Wall -Wextra)

if(TARGET Threads::Threads)
  target_link_libraries(T5MEMORY PRIVATE Threads::Threads)
endif()

find_package(ICU 66.0 COMPONENTS uc i18n REQUIRED)

set(OPENTM2_FOLDER "${ROOT}/source/opentm2")

set(LIBS_DIRECTORY "/opt/local/lib")
set(SHARED_LIBS_DIR "/lib")

link_directories(${LIBS_DIRECTORY} 
				${SHARED_LIBS_DIR})

find_package(folly REQUIRED)
find_package(gflags REQUIRED)
find_package(proxygen REQUIRED)

add_subdirectory(opentm2/core/utilities/zip)

target_link_libraries(T5MEMORY PRIVATE
  git_version
  Folly::folly
  fizz::fizz
  proxygen::proxygen
  proxygen::proxygenhttpserver 
  wangle::wangle
  Boost::boost
  Boost::iostreams
  gflags
  ssl
  crypto
  libxerces-c-3.2.so  zip   ICU::uc ICU::i18n dl
#  -lubsan
#  -lstdc++
)


set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")

configure_file("${ROOT}/source/OtmMemoryServiceConfig.h.in" "${ROOT}/source/OtmMemoryServiceConfig.h"  @ONLY)

install(TARGETS T5MEMORY DESTINATION bin)

find_package(ICU 66.0 COMPONENTS uc i18n REQUIRED)

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