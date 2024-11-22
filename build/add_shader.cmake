include(build/add_embedded_file.cmake)

find_program(GLSLANG glslang)

function(add_shader TARGET FILE ARRAYNAME)
    add_custom_command(
        OUTPUT ${FILE}.spv
        COMMAND ${GLSLANG} -G -o ${FILE}.spv ${FILE}
        DEPENDS ${FILE}
        COMMENT "Compiling shader ${FILE}"
        VERBATIM
    )
    add_embedded_file(${TARGET} ${FILE}.spv ${ARRAYNAME})
endfunction()