project (storage C)

FILE (GLOB LIB_HDR *.h)
add_library (${PROJECT_NAME} binarystore.c textstore.c ${LIB_HDR})

INCLUDE_DIRECTORIES (${CUTEST_INCLUDE_DIR})

add_executable (storage_tests tests.c storage.test.c )
TARGET_LINK_LIBRARIES (storage_tests ${CUTEST_LIBRARIES} )
target_link_libraries (storage_tests storage )

enable_testing()
add_test (storage storage_tests)

IF (MSVC)
    find_package (MSVC MODULE)
    MSVC_CRT_SECURE_NO_WARNINGS (${PROJECT_NAME} storage_test)
ENDIF (MSVC)

set (STORAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "storage headers")
set (STORAGE_LIBRARIES storage CACHE INTERNAL "storage libraries")
