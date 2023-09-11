cmake_minimum_required(VERSION 3.24)
project(test VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (MSVC)
	add_compile_options(/openmp)
else()
	add_compile_options(-fopenmp)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

find_package(Poco REQUIRED COMPONENTS Data DataSQLite Foundation Net NetSSL Util PATHS ${POCO_DIR})

add_executable(main main.cpp)

target_include_directories(main PRIVATE ${POCO_DIR}/include)
target_link_directories(main PRIVATE ${OPENSSL_ROOT_DIR}/lib)
target_link_libraries(main PRIVATE Poco::Foundation Poco::Net Poco::NetSSL)