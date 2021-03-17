cmake_minimum_required(VERSION 3.13)
project (storage C)

# see https://cmake.org/pipermail/cmake/2010-January/034669.html
#cmake_policy(SET CMP0092 NEW)
#string(REGEX REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (MSVC)
  add_compile_options(/WX /MP /FC)
endif (MSVC)
enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file (GLOB LIB_HDR *.h)
add_library (storage
	binarystore.c 
	textstore.c
	filestream.c
	memstream.c
	${LIB_HDR})

add_executable (test_storage tests.c storage.test.c stream.test.c CuTest.c)
target_link_libraries (test_storage storage)
add_test (storage test_storage)

IF (MSVC)
  #target_compile_definitions(storage PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(storage PRIVATE /WX /MP)
ENDIF (MSVC)

set (STORAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "storage headers")
set (STORAGE_LIBRARIES storage CACHE INTERNAL "storage libraries")
