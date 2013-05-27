project (storage C)

FILE (GLOB LIB_HDR *.h)
add_library (storage binarystore.c textstore.c filestream.c memstream.c ${LIB_HDR})

INCLUDE_DIRECTORIES (${CUTEST_INCLUDE_DIR})

add_executable (test_storage tests.c storage.test.c stream.test.c )
TARGET_LINK_LIBRARIES (test_storage ${CUTEST_LIBRARIES} )
target_link_libraries (test_storage storage )

enable_testing()
add_test (storage test_storage)

IF (MSVC)
    find_package (MSVC MODULE)
    MSVC_CRT_SECURE_NO_WARNINGS (${PROJECT_NAME} test_storage)
ENDIF (MSVC)

set (STORAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "storage headers")
set (STORAGE_LIBRARIES storage CACHE INTERNAL "storage libraries")
