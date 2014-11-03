cmake_minimum_required(VERSION 2.8)
project (storage C)

ENABLE_TESTING()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

FILE (GLOB LIB_HDR *.h)
ADD_LIBRARY (storage binarystore.c textstore.c filestream.c memstream.c ${LIB_HDR})

ADD_EXECUTABLE (test_storage tests.c storage.test.c stream.test.c CuTest.c)
TARGET_LINK_LIBRARIES (test_storage storage)
ADD_TEST (storage test_storage)

IF (MSVC)
    FIND_PACKAGE (MSVC MODULE)
    MSVC_CRT_SECURE_NO_WARNINGS (${PROJECT_NAME} test_storage)
ENDIF (MSVC)

SET (STORAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "storage headers")
SET (STORAGE_LIBRARIES storage CACHE INTERNAL "storage libraries")
