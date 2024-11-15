function(compile_shader_spv FILE ARRAYNAME)
    if (UNIX)
        add_custom_command(
            OUTPUT ${FILE}.spv
            COMMAND glslang -G -o ${FILE}.spv ${FILE}
            DEPENDS ${FILE}
            VERBATIM
        )
        add_custom_command(
            OUTPUT ${FILE}.spv.hpp
            COMMAND xxd -i -n ${ARRAYNAME} ${FILE}.spv ${FILE}.spv.hpp
            DEPENDS ${FILE}.spv
            VERBATIM
        )
    else()
        message(FATAL_ERROR "compile_shader_spv not implemented on this platform!")
    endif (UNIX)
endfunction()