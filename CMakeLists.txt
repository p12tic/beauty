cmake_minimum_required(VERSION 3.15)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

project(Beauty)
set(VERSION 1.0.0-rc2)

# C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

# Some packages...
if (NOT DEFINED CONAN OR CONAN)
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
        message(FATAL_ERROR "In Conan mode, you must run a 'conan install' command")
    endif()

    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

    find_package(Boost CONFIG REQUIRED)
    if (BEAUTY_ENABLE_OPENSSL)
        find_package(OpenSSL CONFIG REQUIRED)
    endif()
else()
    if (CMAKE_COMPILER_IS_GNUCXX)
        if (EXISTS "/etc/debian_version")
            find_package(Boost REQUIRED)
        else()
            set (DEFAULT_BOOST_VERSION 1.66.0)

            if (NOT DEFINED BOOST_VERSION)
                set (BOOST_VERSION ${DEFAULT_BOOST_VERSION})
            endif()

            set (BOOST_DIR ${DEVPKGS_PATH}/boost/${BOOST_VERSION})
            set (BOOST_INCLUDE ${BOOST_DIR}/include)
            set (BOOST_LIB ${BOOST_DIR}/lib)

            include_directories(SYSTEM ${BOOST_INCLUDE})
            link_directories(${BOOST_LIB})
        endif()
    endif()

    if (BEAUTY_ENABLE_OPENSSL)
        find_package(OpenSSL REQUIRED)
        add_library(openssl::openssl ALIAS OpenSSL::SSL)
    endif()
endif()

if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Beauty
add_subdirectory(src)
# add_subdirectory(examples)
