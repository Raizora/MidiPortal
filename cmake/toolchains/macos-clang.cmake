# Toolchain file for macOS with Homebrew Clang
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Try to find Homebrew Clang in common locations
set(HOMEBREW_CLANG_PATHS
    "/opt/homebrew/Cellar/llvm/19.1.7/bin/clang"
    "/opt/homebrew/Cellar/llvm/19.1.6/bin/clang"
    "/opt/homebrew/Cellar/llvm/19.1.5/bin/clang"
    "/opt/homebrew/Cellar/llvm/19.1.4/bin/clang"
    "/opt/homebrew/Cellar/llvm/19.1.3/bin/clang"
    "/opt/homebrew/Cellar/llvm/19.1.2/bin/clang"
    "/opt/homebrew/Cellar/llvm/19.1.1/bin/clang"
    "/opt/homebrew/Cellar/llvm/19.1.0/bin/clang"
    "/opt/homebrew/Cellar/llvm/19.0.0/bin/clang"
    "/opt/homebrew/opt/llvm/bin/clang"
)

set(HOMEBREW_CLANGPP_PATHS
    "/opt/homebrew/Cellar/llvm/19.1.7/bin/clang++"
    "/opt/homebrew/Cellar/llvm/19.1.6/bin/clang++"
    "/opt/homebrew/Cellar/llvm/19.1.5/bin/clang++"
    "/opt/homebrew/Cellar/llvm/19.1.4/bin/clang++"
    "/opt/homebrew/Cellar/llvm/19.1.3/bin/clang++"
    "/opt/homebrew/Cellar/llvm/19.1.2/bin/clang++"
    "/opt/homebrew/Cellar/llvm/19.1.1/bin/clang++"
    "/opt/homebrew/Cellar/llvm/19.1.0/bin/clang++"
    "/opt/homebrew/Cellar/llvm/19.0.0/bin/clang++"
    "/opt/homebrew/opt/llvm/bin/clang++"
)

# Find the first existing Homebrew Clang
foreach(CLANG_PATH ${HOMEBREW_CLANG_PATHS})
    if(EXISTS ${CLANG_PATH})
        set(FOUND_CLANG ${CLANG_PATH})
        break()
    endif()
endforeach()

# Find the first existing Homebrew Clang++
foreach(CLANGPP_PATH ${HOMEBREW_CLANGPP_PATHS})
    if(EXISTS ${CLANGPP_PATH})
        set(FOUND_CLANGPP ${CLANGPP_PATH})
        break()
    endif()
endforeach()

# Set the compiler paths
if(DEFINED FOUND_CLANG AND DEFINED FOUND_CLANGPP)
    set(CMAKE_C_COMPILER ${FOUND_CLANG} CACHE PATH "C compiler")
    set(CMAKE_CXX_COMPILER ${FOUND_CLANGPP} CACHE PATH "C++ compiler")
    message(STATUS "Using Homebrew Clang: ${FOUND_CLANG}")
    message(STATUS "Using Homebrew Clang++: ${FOUND_CLANGPP}")
else()
    # Fall back to system Clang if Homebrew Clang is not found
    set(CMAKE_C_COMPILER /usr/bin/clang CACHE PATH "C compiler")
    set(CMAKE_CXX_COMPILER /usr/bin/clang++ CACHE PATH "C++ compiler")
    message(WARNING "Homebrew Clang not found, falling back to system Clang")
endif()

# Enforce minimum version check
set(CMAKE_C_COMPILER_ID "Clang")
set(CMAKE_CXX_COMPILER_ID "Clang")

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Enforce Ninja generator
if(NOT CMAKE_GENERATOR)
  set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "Generator" FORCE)
endif()

# Add compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

message(STATUS "Using toolchain file: macos-clang.cmake")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Generator: ${CMAKE_GENERATOR}") 