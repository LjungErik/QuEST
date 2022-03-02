if (GPUACCELERATED)
    add_subdirectory(GPU)
else()
    add_subdirectory(CPU)
endif()

add_subdirectory(compression)

set(QuEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/QuEST.c
    ${CMAKE_CURRENT_SOURCE_DIR}/QuEST_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/QuEST_qasm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/QuEST_validation.c
    ${CMAKE_CURRENT_SOURCE_DIR}/QuEST_extended.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mt19937ar.c
    ${QuEST_SRC_ARCHITECTURE_DEPENDENT}
    ${QuEST_SRC_COMPRESSION}
    PARENT_SCOPE
)



