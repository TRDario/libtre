include(cmake/add_embedded_file.cmake)

find_program(GLSLANG glslang)

function(add_shader TARGET FILE ARRAYNAME)
    set(FILEPATH "${FILE}")
    cmake_path(ABSOLUTE_PATH FILEPATH NORMALIZE)
    add_custom_command(
        OUTPUT ${FILEPATH}.spv
        COMMAND ${GLSLANG} --quiet -G -o ${FILEPATH}.spv ${FILEPATH}
        DEPENDS ${FILEPATH}
        COMMENT "Compiling GLSL shader ${FILE}"
        VERBATIM
    )
    add_embedded_file(${TARGET} ${FILE}.spv ${ARRAYNAME})
endfunction()